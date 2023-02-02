#ifndef PSQL_H
#define PSQL_H
#include <libpq-fe.h>

#include <string>
#include <lsl_cpp.h>
#include <iostream>
#include <vector>

PGconn* connect_db(std::string name, std::string user, std::string password,std::string host, std::string port);

bool add_stream_metadata(PGconn *C, lsl::stream_info& info);

void create_stream_table_db(PGconn *C, std::string name);

int copy_data_db(PGconn *C, std::string name, std::vector<std::vector<float>>& chunk, std::vector<double>& timestamps, unsigned long int* index );
#endif
