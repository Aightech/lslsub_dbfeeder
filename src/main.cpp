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

int main(int argc, char* argv[])
{
  
  std::vector<lsl::stream_info> strm_info;
  scanStream(strm_info);

  
  pqxx::connection *C = connect_db("lsldb",
				   "lsldb_user",
				   "azerty",
				   "127.0.0.1",
				   "5432");

  std::string sql=
    "INSERT INTO sensors "\
    "VALUES "\
    "(NOW(), 'office', 30.2),"
    "(NOW(), 'office', 30.1);";

  pqxx::work W(*C);
      
  // Execute SQL query 
  W.exec( sql );
  W.commit();
	
  std::cout << "Disconnecting from lsldb...\xd" << std::flush;
  C->disconnect ();
  std::cout << "Disconnected from lsldb.       " << std::endl;
  */
}
