//g++ main.cpp -lpqxx -lpq
#include <iostream>
#include <pqxx/pqxx>
#include <string>
#include <lsl_cpp.h>

void error(std::string str)
{
  std::cout << str << std::endl;
  exit(0);
}

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
      std::cout << "Connecting to lsldb...\xd" << std::flush;
      C = new pqxx::connection(connParam);
      
    
      if (!C->is_open())
        error("Connection to lsldb failed.");
      
      std::cout << "Connected to lsldb       " << std::endl;
      return C;
    }
  catch (const pqxx::pqxx_exception &e)
    {
      std::cerr  << e.base().what() << std::endl;
    }
}

void scanStream(std::vector<lsl::stream_info>& to, bool verbose = true)
{
  std::vector<lsl::stream_info> strm_info = lsl::resolve_streams();
  if(strm_info.size()>0)
    {
      std::string channel_format_str[9] { "none",
            "cf_float32",
            "cf_double64",
            "cf_string",
            "cf_int32",
            "cf_int16",
            "cf_int8",
            "cf_int64",
            "cf_undefined"     // Can not be transmitted.
           };
      
      std::cout << "Available Streams:" << std::endl;
      for (int i = 0; i<strm_info.size(); i++)
	{  
	  if(verbose)
	    {
	      std::cout <<"- " <<  strm_info[i].name()  << " [ "<< strm_info[i].uid() << " ]" << std::endl;
	      std::cout <<"   > Type:         " <<  strm_info[i].type() << std::endl;
	      std::cout <<"   > Nb. Channels: " <<  strm_info[i].channel_count() << std::endl;
	      std::cout <<"   > Format:       " <<  channel_format_str[strm_info[i].channel_format()] << std::endl;
	      std::cout <<"   > Host name:    " <<  strm_info[i].hostname() << std::endl;	  
	      std::cout <<"   > Rate:         " <<  strm_info[i].nominal_srate() << std::endl;
	      //channel_format_str[m_results[0].channel_format()];
	    }

	  //search for the stream in the already scanned ones
	  int new_strm = true;
	  for(int j =0; j < to.size(); j++)
	    {
	      if(strm_info[i].uid().compare( to[j].uid() ) == 0 )
		new_strm = false;
	    }
	  if(new_strm)
	    to.push_back(strm_info[i]);
	}
    }


}

bool add_stream_metadata(pqxx::connection *C, lsl::stream_info& info)
{
  pqxx::nontransaction N(*C);// Create a non-transactional object.
  std::string sql = "SELECT name from lsl_streams_metadata WHERE name='" + info.name() + "';";  
  pqxx::result R( N.exec( sql ));// Execute SQL query 
  N.commit();

  std::cout << "Name = " << R.size() << std::endl;
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
    }


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
      std::cout << timestamps[j] << std::endl; // only showing the time stamps here
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



int main(int argc, char* argv[])
{
  
  std::vector<lsl::stream_info> strm_info;
  scanStream(strm_info);

  
  pqxx::connection *C = connect_db("lsldb",
				   "lsldb_user",
				   "azerty",
				   "127.0.0.1",
				   "5432");
  
  add_stream_metadata(C,strm_info[0]);
  create_stream_table_db(C,strm_info[0].name());
  lsl::stream_inlet inlet(strm_info[0]);
  try {

    // and retrieve the chunks (note: this can of course also be done with pure std::vectors
    // instead of stereo_samples)
    while (true)
      {
	std::vector<std::vector<float>> chunk;
	std::vector<double> timestamps;
	if (inlet.pull_chunk(chunk, timestamps))
	  {
	    insert_data_db(C, strm_info[0].name(), chunk, timestamps); 
	  }
    }

  }
  catch (std::exception& e)
    {
      std::cerr << "Got an exception: " << e.what() << std::endl;
    }
  
	
  std::cout << "Disconnecting from lsldb...\xd" << std::flush;
  C->disconnect ();
  std::cout << "Disconnected from lsldb.       " << std::endl;
  
}
