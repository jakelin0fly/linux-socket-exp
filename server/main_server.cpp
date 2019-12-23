#include "server.h"
#include <stdlib.h> //atoi
#include <iostream>
using namespace std;
int main(int argc, char const *argv[])
{
	if(2 != argc){
		cout << "start server: ./server server_port" << endl;
		return -1;
	}
	Server my_server(atoi(argv[1]));
	my_server.work();
	return 0;
}