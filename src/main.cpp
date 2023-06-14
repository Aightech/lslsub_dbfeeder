/**
 * \file main.cpp
 * \brief Main file of the lslsub_dbfeeder project.
 * \author Alexis Devillard
 * \version 1.0
 * \date 14 june 2023
 */

#include <ctime>
#include <fstream>   // open file
#include <iostream>  //cout,cin
#include <lsl_cpp.h> //lsl library
#include <sstream>   //stream in file
#include <string.h>
#include <string>
#include <thread> // std::thread
#include <vector>

#include "lslrecorder.hpp" // function to streamore stream in postgres database

#include <boost/program_options.hpp>

/**
 * @brief scanstream Search for the lsl streams and add them to the lslinfo vector..
 * @param to Lslinfo vector.
 * @param verbose To activate the display of the parameters of each streams.
 */
void scanStream(std::vector<lsl::stream_info> &to, bool verbose = true)
{
    std::vector<lsl::stream_info> strm_info = lsl::resolve_streams();
    to.clear();
    if(strm_info.size() > 0)
    {
        std::cout << "[INFO] Available Streams:" << std::endl;
        for(int i = 0; i < strm_info.size(); i++)
        {
            std::cout << "-  " << strm_info[i].name() << " [ "
                      << strm_info[i].uid() << " ]" << std::endl;
            to.push_back(strm_info[i]);
        }
    }
}

/**
 * @brief get_conf Read a configuration file and extract the name of stream to record.
 * @param file Configuration file.
 * @param strm String vector of the stream to record's name.
 */
void get_conf(std::string file, std::vector<std::string> &strm)
{
    std::ifstream source;

    source.open(file, std::ios_base::in); // open data
    if(source)
    {
        std::cout << "[INFO] "
                  << "Streams to be recorded:" << std::endl;
        for(std::string line; std::getline(source, line);)
        {
            std::istringstream in(line);
            std::string name;
            in >> name;
            strm.push_back(name);
            std::cout << "\t- " << name << std::endl;
        }
    }
    else
        std::cout << "No configuration file [" << file
                  << "] found. No stream will be recorded" << std::endl;
}

namespace po = boost::program_options;
int main(int argc, char *argv[])
{
    //get arguments values with boost::program_options
    //supported arguments
    po::options_description desc("Allowed options");
    desc.add_options()("help,h", "produce help message")(
        "config,c", po::value<std::string>()->default_value("config/conf.cfg"),
        "Configuration file")(
        "id,i", po::value<std::string>()->default_value("uid_default"),
        "ID of the recording session.")("verbose,v", "Verbose mode.")(
        "trigger,t", "Trigger mode.");

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if(vm.count("help"))
    {
        //print basic usage and description
        std::cout << "Usage: " << argv[0] << " [options]\n";
        std::cout << desc << "\n";
        return 1;
    }

    //get the streams to record.
    std::vector<std::string> streams_to_get;
    std::string cfg_file = vm["config"].as<std::string>();
    std::string uid = vm["id"].as<std::string>();
    bool verbose = vm.count("verbose");
    bool trigger = vm.count("trigger");

    get_conf(cfg_file, streams_to_get);

    //scan the available streams.
    std::vector<lsl::stream_info> strm_info;
    scanStream(strm_info, false);

    //start to record the wanted streams that were available.
    std::vector<lsl::Recorder *> recorders;
    for(int i = 0; i < strm_info.size(); i++)
        for(int j = 0; j < streams_to_get.size(); j++)
            if(strm_info[i].name().compare(streams_to_get[j]) == 0)
            { //create a recorder for the stream
                recorders.push_back(
                    new lsl::Recorder(strm_info[i], verbose * 2));
            }

    //if at least one stream recording thread was launch
    if(recorders.size() > 0)
    { //wait for the user to stop the recording.
        if(trigger)
        {
            //searching for a trigger lsl stream
            std::cout << "[INFO] Searching for a trigger stream..."
                      << std::endl;
            bool trigger_found = false;
            lsl::stream_info trigger_info;
            while(!trigger_found)
            {
                std::vector<lsl::stream_info> streams_info =
                    lsl::resolve_streams();
                for(int i = 0; i < streams_info.size(); i++)
                {
                    if(streams_info[i].name().compare("trigger") == 0)
                    {
                        trigger_found = true;
                        std::cout << "[INFO] Trigger found." << std::endl;
                        trigger_info = streams_info[i];
                    }
                }
            }
            //create an inlet to receive the trigger
            lsl::stream_inlet trigger_inlet(trigger_info);
            //read the trigger: 1 = start recording, 0 = pause recording, -1 = stop recording
            std::string trigger_cmd = "";
            std::string trigger_value = "";
            std::vector<std::string> trigger_sample;
            bool recording = true;
            while(recording)
            {
                trigger_inlet.pull_sample(trigger_sample);
                trigger_cmd = trigger_sample[0];   //can be start, pause or stop
                trigger_value = trigger_sample[1]; //the uid of the session

                if(trigger_cmd.compare("start") == 0) //if the trigger is start
                {
                    std::cout << "[INFO] Trigger received: start recording."
                              << std::endl;
                    //start the recording of each thread.
                    for(int i = 0; i < recorders.size(); i++)
                        recorders[i]->start_recording(trigger_value);
                }
                else if(trigger_cmd.compare("pause") ==
                        0) //if the trigger is pause
                {
                    std::cout << "[INFO] Trigger received: pause recording."
                              << std::endl;
                    //pause the recording of each thread.
                    for(int i = 0; i < recorders.size(); i++)
                        recorders[i]->pause_recording();
                }
                else if(trigger_cmd.compare("stop") ==
                        0) //if the trigger is stop
                {
                    std::cout << "[INFO] Trigger received: stop recording."
                              << std::endl;
                    //stop the recording of each thread.
                    for(int i = 0; i < recorders.size(); i++)
                        recorders[i]->stop_recording();
                    recording = false;
                }
            }
        }
        else
        {
            std::cout << "[INFO] Press any key then <ENTER> to stop recording "
                         "and quit."
                      << std::endl;
            try
            {
                for(int i = 0; i < recorders.size(); i++)
                    recorders[i]->start_recording(uid);
                int a;
                std::cin >> a;
                //stop the main loop of recording of each thread.
                for(int i = 0; i < recorders.size(); i++)
                    recorders[i]->stop_recording();
            }
            catch(std::string e)
            {
                std::cout << e << std::endl;
            }
        }
    }
    else
        std::cout << "[INFO] None of the listed streams has been found."
                  << std::endl;

    return 0;
}
