/**
 * \file main.cpp
 * \brief TODO.
 * \author Alexis Devillard
 * \version 1.0
 * \date 08 may 2019
 */

#include <iostream> //cout,cin
#include <string> 
#include <lsl_cpp.h> //lsl library
#include <thread> // std::thread
#include <fstream> // open file
#include <sstream> //stream in file
#include <vector>
#include <ctime>
#include <string.h>

#include "psql.h" // function to streamore stream in postgres database
#include "tools.h" // args, usage, error


/**
 * @brief scanstream Search for the lsl streams and add them to the lslinfo vector..
 * @param to Lslinfo vector.
 * @param verbose To activate the display of the parameters of each streams.
 */
void scanStream(std::vector<lsl::stream_info>& to, bool verbose=true)
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
      
      std::cout << "[INFO] Available Streams:" << std::endl;
      for (int i = 0; i<strm_info.size(); i++)
	{
	  std::cout <<"-  " <<  strm_info[i].name()  << " [ "<< strm_info[i].uid() << " ]" << std::endl;
	  if(verbose)
	    {
	      
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


/**
 * @brief store_stream Connect to the database and start storing the incoming stream..
 * @param strm_info the info of stream to record.
 * @param rec_on Boolean that stop the main loop of recording.
 */
void store_stream(lsl::stream_info strm_info, bool *rec_on, std::string uid, int n)
{
  PGconn *C= connect_db("lsldb",
  			 "lsldb_user",
  			 "azerty",
  			 "127.0.0.1",
  			 "5432");
  
  add_stream_metadata(C,strm_info);
  create_stream_table_db(C,strm_info.name());
  lsl::stream_inlet inlet(strm_info, 360, 0, false);
  std::string spacer ="";
  for(int i =0;i<n;i++)
    spacer+="\t\t\t\t\t";

  try {

    // and retrieve the chunks (note: this can of course also be done with pure std::vectors
    // instead of stereo_samples)
    std::vector<std::vector<float>> chunk;
    std::vector<double> timestamps;

    unsigned long int t=0;
    clock_t begin;
    clock_t end;
    double elapsed_secs;
    double rate;
    
    std::cout << "[" << strm_info.name() << "] Started." << std::endl;
    while (*rec_on)
      {
	chunk.clear();
	timestamps.clear();
	if (inlet.pull_chunk(chunk, timestamps))
	  {
	    begin = clock();
	    copy_data_db(C, strm_info.name(), chunk, timestamps, uid, &t);
	    end = clock();
	    elapsed_secs = double(end - begin) / CLOCKS_PER_SEC*1000000;
	    if(t%100==0)
	      std::cout <<spacer <<"t:"<< elapsed_secs <<"   \xd" << std::flush;
	  }
    }
    
    std::cout << "[" << strm_info.name() <<"] Disconnecting from lsldb...\xd" << std::flush;
    PQfinish(C);
    std::cout << "[" << strm_info.name() <<"] Disconnected from lsldb." <<  std::endl;
    std::cout << "[" << strm_info.name() <<"] Nb sample: " << t << std::endl;
  }
  catch (std::exception& e)
    {
      std::cerr << "[EXEPTION] Got an exception: " << e.what() << std::endl;
    }
  
	
  
}

/**
 * @brief get_conf Read a configuration file and extract the name of stream to record.
 * @param file Configuration file.
 * @param strm String vector of the stream to record's name.
 */
int get_conf(std::string file, std::vector<std::string>& strm)
{
  std::ifstream source;
  
  source.open(file, std::ios_base::in);  // open data
  if (source)
    {
      std::cout << "[INFO] " << "Streams to be recorded:" << std::endl;
      for(std::string line; std::getline(source, line); )
	{
	  std::istringstream in(line);
	  std::string name;
	  in>>name;
	  strm.push_back(name);
	  std::cout << "\t- " << name<< std::endl;
	}
    }
  else
    std::cout << "No configuration file [" << file << "] found. No stream will be recorded" << std::endl;
}




int main(int argc, char* argv[])
{
 std::vector<std::string> opt_flag(
				    {"-c",
					"-id"});
  std::vector<std::string> opt_label(
				     {"Configuration file",
					 "ID of the recording session."});
  std::vector<std::string> opt_value(
				     {"config/conf.cfg",
					 "0"});
  
  get_arg(argc, argv, opt_flag, opt_label, opt_value);

  //get the streams to record.
  std::vector<std::string> streams_to_get;
  std::string cfg_file = opt_value[0];
  std::string uid = opt_value[1];
  get_conf(cfg_file, streams_to_get);
  
  //scan the available streams.
  std::vector<lsl::stream_info> strm_info;
  scanStream(strm_info, false);

  //start to record the wanted streams that were available.
  std::vector<std::thread> strm_thread;
  bool rec_on = false;
  for(int i =0; i < strm_info.size(); i++)
      for(int j =0; j < streams_to_get.size(); j++)
  	  if(strm_info[i].name().compare( streams_to_get[j] ) == 0 )
	    {//launch a thread
	      rec_on = true;
	      strm_thread.push_back(std::thread(store_stream, strm_info[i], &rec_on, uid, i));
	      std::cout << "[INFO] Start to record: " <<  streams_to_get[j] << std::endl;
	    }

  //if at least one stream recording thread was launch 
  if(rec_on)
    {//wait for the user to stop the recording.
      std::cout << "[INFO] Press any key then <ENTER> to stop recording and quit."<< std::endl;
      int a;
      std::cin >> a;
      //stop the main loop of recording of each thread.
      rec_on=false;
      for (auto& th : strm_thread) th.join();
    }
  else
    std::cout << "[INFO] None of the listed streams has been found."<< std::endl;

  return 0;
  
  
  
}
