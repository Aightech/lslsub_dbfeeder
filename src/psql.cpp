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
pqxx::connection* connect_db(std::string name, std::string user, std::string password,std::string host, std::string port)
{
  pqxx::connection *C;
  try
    {
      std::string connParam=
	"dbname = " + name + \
	" user = " + user + \
	" password = " + password + \
	" hostaddr = " + host + \
	" port = " + port;
      //std::cout << "Connecting to lsldb...\xd" << std::flush;
      C = new pqxx::connection(connParam);
      
    
      if (!C->is_open())
        error("Connection to lsldb failed.");
      
      //std::cout << "Connected to lsldb       " << std::endl;
      return C;
    }
  catch (const pqxx::pqxx_exception &e)
    {
      std::cerr  << e.base().what() << std::endl;
    }
}


/**
 * @brief add_stream_metadata Enable to add a table with the stream name if it does not exist. return false if it existed
 * @param name Name of the database
 * @param user User name to connect the database
 * @return The connected object.
 */
bool add_stream_metadata(pqxx::connection *C, lsl::stream_info& info)
{
  pqxx::nontransaction N(*C);// Create a non-transactional object.
  std::string sql = "SELECT name from lsl_streams_metadata WHERE name='" + info.name() + "';";  
  pqxx::result R( N.exec( sql ));// Execute SQL query 
  N.commit();

  if(R.size()==0)
    {
      pqxx::work W(*C);
  
      sql=
	"INSERT INTO lsl_streams_metadata (name, type, format, rate, nb_channels, host) " \
	"VALUES ( '" + info.name()	+ "'" +					\
	" , '" + info.type()+ "'"		  +				\
	" , " + std::to_string(info.channel_format())+			\
	" , " + std::to_string(info.nominal_srate())  +			\
	" , " + std::to_string(info.channel_count())  +			\
	" , '" + info.hostname()+ "'"	  +				\
	");";
      
      // Execute SQL query 
      W.exec( sql );
      W.commit();
      return true;
    }
  return false;


}

void create_stream_table_db(pqxx::connection *C, std::string name)
{
  std::string sql=
    "CREATE TABLE IF NOT EXISTS " + name + " ( "\
    "time DOUBLE PRECISION       NOT NULL,"	\
    "data DOUBLE PRECISION[]  NOT NULL,"	\
    "uid TEXT  NULL);";

  pqxx::work W(*C);
      
  // Execute SQL query 
  W.exec( sql );
  W.commit();
}

void insert_data_db(pqxx::connection *C, std::string name, std::vector<std::vector<float>>& chunk, std::vector<double>& timestamps )
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
      sql += "}'); " ;
    }
	    
  pqxx::work W(*C);
      
  // Execute SQL query 
  W.exec( sql );
  W.commit();
}
