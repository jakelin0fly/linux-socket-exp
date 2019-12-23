#include "adaptor.h"
#include "../common/selfdefine.h"
#include "../common/helper.h"
#include <vector>
#include <iostream>
#include <string.h>

using namespace std;

int main(int argc, char const *argv[])
{
	std::vector<ServerStruct> v;
	// ServerStruct server;
	// strcpy(server.str_ip, "127.0.0.1");
	// server.port = SERVER_LISTEN_PORT;
	// v.push_back(server);
	Helper::GetServersFromFile("config.txt", v);	//get servers from file

	Adaptor myAdaptor(v, ADAPTOR_LISTEN_PORT);
	myAdaptor.work();
	return 0;
}