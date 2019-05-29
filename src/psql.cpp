/**
 * \file psql.cpp
 * \brief TODO.
 * \author Alexis Devillard
 * \version 1.0
 * \date 08 may 2019
 */
#include "psql.h"
#include "tools.h"


/**
 * @brief connect_db Enable to connect to a database and return the connected object of pqxx library
 * @param name Name of the database
 * @param user User name to connect the database
 * @param password User Password
 * @param host Host of the database
 * @param port Port of the host
 * @return The connected object.
 */
PGconn* connect_db(std::string name, std::string user, std::string password,std::string host, std::string port)
{
  std::string connParam=
    "dbname = " + name +    \
    " user = " + user +		  \
    " password = " + password +	    \
    " hostaddr = " + host +	    \
    " port = " + port;
  //std::cout << "Connecting to lsldb..." << "\xd" << std::flush;
  PGconn *conn = PQconnectdb(connParam.c_str());
  if (PQstatus(conn) == CONNECTION_BAD)
    {
      std::cout <<  "[" << name << "] Connection to database failed: " <<  PQerrorMessage(conn) << std::endl;
      PQfinish(conn);
      error("");
  }
    
  //char *userr = PQuser(conn);
  //char *db_name = PQdb(conn);
  //char *pswd = PQpass(conn);
  
  return conn;  
 
}


/**
 * @brief add_stream_metadata Enable to add a table with the stream name if it does not exist. return false if it existed
 * @param name Name of the database
 * @param user User name to connect the database
 * @return The connected object.
 */
bool add_stream_metadata(PGconn *C, lsl::stream_info& info)
{
  std::string sql = "SELECT name from lsl_streams_metadata WHERE name='" + info.name() + "'";
  PGresult *res = PQexec(C, sql.c_str());
  
  if (PQntuples(res)==0)
    {
      PQclear(res);
      sql="INSERT INTO lsl_streams_metadata (name, type, format, rate, nb_channels, host) " \
	"VALUES( '" + info.name()	+ "'" +				\
	" , '" + info.type()+ "'"		  +			\
	" , " + std::to_string(info.channel_format())+			\
	" , " + std::to_string(info.nominal_srate())  +			\
	" , " + std::to_string(info.channel_count())  +			\
    " , '" + info.hostname()+ "'"	  +				\
	")";
      res = PQexec(C,sql.c_str());
      if (PQresultStatus(res) != PGRES_COMMAND_OK)
	  error(PQerrorMessage(C));
      PQclear(res);
      std::cout << "[" <<  info.name() << "]" << "Added to lsl_streams_metadata." << std::endl;
      return true;
    }
  else
    {
      PQclear(res);
      std::cout << "[" <<  info.name() << "]" << "Already present in lsl_streams_metadata." << std::endl;
      return false;
    }
    
  return false;


}

void create_stream_table_db(PGconn *C, std::string name)
{
  std::string sql=
    "CREATE TABLE IF NOT EXISTS " + name + " ( "\
    "time DOUBLE PRECISION       NOT NULL,"	\
    "data DOUBLE PRECISION[]  NOT NULL,"	\
    "uid TEXT  NULL)";

  
  PGresult *res = PQexec(C, sql.c_str());
    
  if (PQresultStatus(res) != PGRES_COMMAND_OK) 
    error(PQerrorMessage(C));
  
  // Execute SQL query  
  PQclear(res);
  
}

void insert_data_db(PGconn *C, std::string name, std::vector<std::vector<float>>& chunk, std::vector<double>& timestamps )
{
  std::string sql="";
  for(int j = 0; j < chunk.size(); j++)
    {
      //std::cout << timestamps[j] << std::endl; // only showing the time stamps here
      sql += "INSERT INTO " + name + " (time, data) "+ \
	"VALUES ( " + std::to_string(timestamps[j]) +		\
	" , '{" + std::to_string(chunk[j][0]);
      for(int i =1; i < chunk[j].size(); i++)
	sql += "," + std::to_string(chunk[j][i]);
      sql += "}') " ;
    }
	    
  // Execute SQL query
  PGresult *res = PQexec(C, sql.c_str());
  if (PQresultStatus(res) != PGRES_COMMAND_OK) 
    error(PQerrorMessage(C));
  PQclear(res);
}
