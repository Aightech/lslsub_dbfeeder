#ifndef PSQL_H
#define PSQL_H
#include <libpq-fe.h>

#include <iostream>
#include <mutex>
#include <strANSIseq.hpp>
#include <string>
#include <vector>

#define SIZE_MAX_ARR 200000
namespace psql
{
class Database : public ESC::CLI
{
    public:
    Database(int verbose = -1);
    ~Database(){};

    /**
     * @brief connect Enable to connect to a database
     * @param name Name of the database
     * @param user User name to connect the database
     * @param password User Password
     * @param host Host of the database
     * @param port Port of the host
     */
    void connect(std::string name,
                 std::string user,
                 std::string password,
                 std::string host,
                 std::string port);

    void disconnect();

    /**
     * @brief create_table Enable to create a new table.
     * @param name Name of the new table
     */
    void create_table(std::string name);

    /**
     * @brief store_data Enable to store data in a table
     * @param table Name of the table
     * @param chunk Data to store
     * @param timestamps Timestamps of the data
     * @param index Index of the data
     * @return
     */
    int store_data(std::string table,
                   std::vector<std::vector<double>> &chunk,
                   std::vector<double> &timestamps,
                   uint64_t index);

    private:
    /**
     * @brief prepare_buffer Enable to prepare the buffer to store data and save time during the recording
     * @param size_sample Size of the sample
     */
    void prepare_buffer(int size_sample);
    std::string m_name;
    std::string m_user;
    std::string m_password;
    std::string m_host;
    std::string m_port;
    PGconn *m_conn;
    char m_buffer[SIZE_MAX_ARR];
    int m_max_samples = 0;
    int m_sample_size = 0;
    std::mutex *m_mutex;
};
}; // namespace psql

#endif
