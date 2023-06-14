/**
 * \file psql.cpp
 * \brief TODO.
 * \author Alexis Devillard
 * \version 1.0
 * \date 08 may 2019
 */
#include "psql.hpp"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

namespace psql
{

Database::Database(int verbose) : CLI(verbose, "psqlDB")
{
    /* Start of the buffer
  +--------------------+---------+-----------+
  |       HEADER       |  FLAG   | EXTENTION |
  |--------------------|---------|-----------|
  |      11 bytes      | 4 bytes |  4 bytes  |
  | PGCOPY\n\377\r\n\0 |   0000  |    0000   |
  +--------------------+---------+-----------+
  */
    memcpy(m_buffer, "PGCOPY\n\377\r\n\0\0\0\0\0\0\0\0\0", 19); //PGCOPY_HEADER
    m_mutex = new std::mutex();
};

void Database::connect(std::string name,
                       std::string user,
                       std::string password,
                       std::string host,
                       std::string port)
{
    std::string connParam = "dbname = " + name + " user = " + user +
                            " password = " + password + " hostaddr = " + host +
                            " port = " + port;
    m_name = name;
    m_user = user;
    m_password = password;
    m_host = host;
    m_port = port;

    //std::cout << "Connecting to lsldb..." << "\xd" << std::flush;
    logln("Connecting to " + name + "....", true);
    m_conn = PQconnectdb(connParam.c_str());
    if(PQstatus(m_conn) == CONNECTION_BAD)
    {
        disconnect();
        throw "Connection to database failed: " +
            std::string(PQerrorMessage(m_conn));
    }
    else
    {
        logln("Connection to " + name + " successful", true);
    }

    //char *userr = PQuser(conn);
    //char *db_name = PQdb(conn);
    //char *pswd = PQpass(conn);
}

void Database::create_table(std::string name)
{
    std::lock_guard<std::mutex> lck(*m_mutex); //ensure only one thread using it
    std::string sql = "CREATE TABLE IF NOT EXISTS " + name +
                      " ( "
                      "index BIGINT NOT NULL,"
                      "time DOUBLE PRECISION NOT NULL,"
                      "data DOUBLE PRECISION[]  NOT NULL)";

    // Execute SQL query
    PGresult *res = PQexec(m_conn, sql.c_str());
    if(PQresultStatus(res) != PGRES_COMMAND_OK) {} //error(PQerrorMessage(C));
    PQclear(res);

    sql = "SELECT create_hypertable('" + name +
          "', 'time', if_not_exists => true)";

    logln("Creating hypertable " + name + "...", true);
    //Execute SQL query
    res = PQexec(m_conn, sql.c_str());
    PQclear(res);
}

void Database::disconnect()
{
    logln("Disconnecting from " + m_name + "...", true);
    PQfinish(m_conn);
}

void Database::prepare_buffer(int size_sample)
{
    /*
     for each rows in the chunk:

        +------------+-------------------------------------------+------------------------------------------------------------------------------------------------------------------+
        |            |                                           |                                                         DATA CHUNK                                               |
        |  COLUMNS   |               INDEX & TIME                +------------------------------------------------------------------------------------------------------------------+
        |            |                                           |                            ARRAY HEADER                          |                      ARRAY                    |
        |------------+---------------------+---------------------+---------+-----------+---------+------------+-----------+---------+---------------------+---+---------------------+
        | NB COLUMNS |  SIZE   |    DATA   |  SIZE   |    DATA   |  SIZE   |   ndim    | hasnull |  elem_type | dimension | lbound  |   size  |   data 1  |   |   size  |   data n  |
        |------------+---------+-----------+---------+-----------+---------+-----------+---------+------------+-----------+---------+---------+-----------+---+---------+-----------+
        |   2 bytes  | 4 bytes |  8 bytes  | 4 bytes |  8 bytes  | 4 bytes |  4 bytes  | 4 bytes |  4 bytes   |  4 bytes  | 4 bytes | 4 bytes |  8 bytes  |...|   size  |  8 bytes  |
        |      3     |    8    |    index  |    8    | timestamp |    n    |     1     |    0    | 701-double |     408   |    1    |    8    | sample[0] |   |   size  | sample[n] |
        +------------+-------------------------------------------+------------------------------------------------------------------------------------------------------------------+

        End of the buffer:

        +---------+
        |   END   |
        |---------|
        | 2 bytes |
        |   -1    |
        +---------+


       */
      logln("Preparing buffer for " + std::to_string(size_sample) + " samples", true);
    m_sample_size = size_sample;
    char *b = m_buffer + 19;
    uint16_t nb_col = htobe16(3);
    uint32_t size;
    double timestamp;
    uint64_t ind;
    uint32_t ndim = htobe32(1);
    uint32_t hasnull = htobe32(0);
    uint32_t elem_type = htobe32(701);
    uint32_t dim = htobe32(size_sample);
    uint32_t lbound = htobe32(1);

    char array_header[24];
    size = htobe32(20 + 12 * size_sample);

    memcpy(array_header, (char *)(&size), 4);
    memcpy(array_header + 4, (char *)(&ndim), 4);
    memcpy(array_header + 8, (char *)(&hasnull), 4);
    memcpy(array_header + 12, (char *)(&elem_type), 4);
    memcpy(array_header + 16, (char *)(&dim), 4);
    memcpy(array_header + 20, (char *)(&lbound), 4);
    int j;
    for(j = 0; SIZE_MAX_ARR > 19 + (j + 1) * (50 + 12 * size_sample); j++)
    {

        //number of columns
        memcpy(b, (char *)(&nb_col), 2);
        b += 2;

        //size of index and time stamps
        size = htobe32(8);
        memcpy(b, (char *)(&size), 4);
        b += 12;
        memcpy(b, (char *)(&size), 4);
        b += 12;

        //data array
        memcpy(b, array_header, 24);
        b += 24;

        //size for each sample
        size = htobe32(8);
        for(int i = 0; i < size_sample; i++)
        {
            memcpy(b, (char *)(&size), 4);
            b += 12;
        }
    }
    m_max_samples = j;

    uint16_t negative = htobe16(-1);
    memcpy(b, (char *)(&negative), 2);
}

int Database::store_data(std::string table,
                         std::vector<std::vector<double>> &chunk,
                         std::vector<double> &timestamps,
                         uint64_t index)
{
    std::lock_guard<std::mutex> lck(*m_mutex); //ensure only one thread using it
    PGresult *res = NULL;
    if(chunk.size() == 0)
        return index;
    if(chunk[0].size() != m_sample_size)
        prepare_buffer(chunk[0].size());

    double data;
    double timestamp;
    uint64_t ind;
    for(int j = 0, k = 0; j < chunk.size(); k++)
    { //in case the chunk is too big to fit in the buffer, iterate over the chunk
        char *b = m_buffer + 19;
        /*
       for each rows in the chunk:
       +------------+-------------------------------------------+------------------------------------------------------------------------------------------------------------------+
       |            |                                           |                                                         DATA CHUNK                                               |
       |  COLUMNS   |               INDEX & TIME                +------------------------------------------------------------------------------------------------------------------+
       |            |                                           |                            ARRAY HEADER                          |                      ARRAY                    |
       |------------+---------------------+---------------------+---------+-----------+---------+------------+-----------+---------+---------------------+---+---------------------+
       | NB COLUMNS |  SIZE   |    DATA   |  SIZE   |    DATA   |  SIZE   |   ndim    | hasnull |  elem_type | dimension | lbound  |   size  |   data 1  |   |   size  |   data n  |
       |------------+---------+-----------+---------+-----------+---------+-----------+---------+------------+-----------+---------+---------+-----------+---+---------+-----------+
       |   2 bytes  | 4 bytes |  8 bytes  | 4 bytes |  8 bytes  | 4 bytes |  4 bytes  | 4 bytes |  4 bytes   |  4 bytes  | 4 bytes | 4 bytes |  8 bytes  |...|   size  |  8 bytes  |
       |      3     |    8    |    index  |    8    | timestamp |    n    |     1     |    0    | 701-double |     408   |    1    |    8    | sample[0] |   |   size  | sample[n] |
       +------------+-------------------------------------------+------------------------------------------------------------------------------------------------------------------+
      */

        for(; j < chunk.size() - k * m_max_samples; j++)
        {
            b += 6;//skip the number of columns and the size of index
            ind = htobe64(index++);
            memcpy(b, (char *)(&ind), 8);
            b += 12;//skip index and the size of timestamp
            *(uint64_t *)&timestamp = htobe64(*(uint64_t *)&(timestamps[j]));
            memcpy(b, (char *)(&timestamp), 8);
            b += 32;//skip the timestamp and the array header
            for(int i = 0; i < m_sample_size; i++)
            {
                b += 4;//skip the size of the sample's element
                *(uint64_t *)&data = htobe64(*(uint64_t *)&(chunk[j][i]));
                memcpy(b, (char *)(&data), 8);
                b += 8;
            }
        }
        uint16_t negative = htobe16(-1);
        memcpy(b, (char *)(&negative), 2);
        b += 2;

        res = PQexec(
            m_conn, ("COPY " + table + " FROM STDIN (FORMAT binary);").c_str());
        if(PQresultStatus(res) != PGRES_COPY_IN)
            throw "PQresultStatus failed: " +
                std::string(PQresultErrorMessage(res));
        else
        {
            PQclear(res);
            int copyRes =
                PQputCopyData(m_conn, m_buffer,
                              b - m_buffer); //Copy the buffer to the database
            if(copyRes == 1)                 //if the buffer is copied
            {
                if(PQputCopyEnd(m_conn, NULL) == 1) //if the copy is ended
                {
                    res = PQgetResult(m_conn);
                    if(PQresultStatus(res) !=
                       PGRES_COMMAND_OK) //if the copy is not ok
                        throw "PQresultStatus failed: " +
                            std::string(PQresultErrorMessage(res));
                    //else the copy is ok and we can continue
                }
                else
                    throw "PQputCopyEnd failed: " +
                        std::string(PQresultErrorMessage(res));
            }
            else //if the buffer is not copied
                throw "PQputCopyData failed: " +
                    std::string(PQresultErrorMessage(res));
        }
    }
    return index;
}
} // namespace psql