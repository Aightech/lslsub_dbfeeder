//g++ main.cpp -lpqxx -lpq
#include <iostream>
#include <pqxx/pqxx> 

using namespace std;
using namespace pqxx;

void error(std::string str)
{
  std::cout << str << std::endl;
  exit(0);
}

int main(int argc, char* argv[])
{
  try
    {
      std::string conParam=
	"dbname = lsldb "\
	"user = postgres "\
	"password = abcdef "\
	"hostaddr = 127.0.0.1 "\
	"port = 5432";
      std::cout << "Connecting to lsldb...\xd" << std::flush;
      connection C(conParam);
    
      if (!C.is_open())
        error("Connection to lsldb failed.");
      
      std::cout << "Connected to lsldb       " << std::endl;


      std::string sql=
	"INSERT INTO sensors "\
	"VALUES "\
	"(NOW(), 'office', 30.2),"
	"(NOW(), 'office', 30.1);";

      work W(C);
      
      /* Execute SQL query */
      W.exec( sql );
      W.commit();
	
      std::cout << "Disconnecting from lsldb...\xd" << std::flush;
      C.disconnect ();
      std::cout << "Disconnected from lsldb.       " << std::endl;
    }
  catch (const std::exception &e)
    {
      cerr << e.what() << std::endl;
      return 1;
    }
}
