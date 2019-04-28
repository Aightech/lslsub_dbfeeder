//g++ main.cpp -lpqxx -lpq
#include <iostream>
#include <pqxx/pqxx>
#include <string>

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

int main(int argc, char* argv[])
{
  pqxx::connection *C = connect_db("lsldb",
				   "postgres",
				   "azerty",
				   "127.0.0.1",
				   "5432");
  

  std::string sql=
    "INSERT INTO sensors "\
    "VALUES "\
    "(NOW(), 'office', 30.2),"
    "(NOW(), 'office', 30.1);";

  pqxx::work W(*C);
      
  /* Execute SQL query */
  W.exec( sql );
  W.commit();
	
  std::cout << "Disconnecting from lsldb...\xd" << std::flush;
  C->disconnect ();
  std::cout << "Disconnected from lsldb.       " << std::endl;
    
}
