#ifndef PSQL_H
#define PSQL_H
#include <pqxx/pqxx>
#include <string>
#include <lsl_cpp.h>
#include <iostream>


pqxx::connection* connect_db(std::string name, std::string user, std::string password,std::string host, std::string port);

bool add_stream_metadata(pqxx::connection *C, lsl::stream_info& info);

void create_stream_table_db(pqxx::connection *C, std::string name);

void insert_data_db(pqxx::connection *C, std::string name, std::vector<std::vector<float>>& chunk, std::vector<double>& timestamps );

#endif
