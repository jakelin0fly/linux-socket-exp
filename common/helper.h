#ifndef HELPER_H
#define HELPER_H
#include "selfdefine.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include<fcntl.h>
#include <iostream>
#include <fstream>
#include <string>
#include <string.h>
#include<vector>

using namespace std;

class Helper
{
public:
	Helper(){}
	virtual ~Helper(){}

	//Serialize Client request (file path, request object)
	//append
	static bool Serialize(const char *pfile, const int &request_num){
		int bytes;
		int handle;
		//open file append
		handle = open(pfile, O_WRONLY|O_APPEND|O_CREAT, S_IRWXU);
		if(-1 == handle)
			return false;
		bytes = write(handle, &request_num, sizeof(int));	//write to file
		if(0 >= bytes){
			cout << "Serialize error!" << endl;
			return false;
		}
		return true;
	}

	static bool Deserialize(const char *pfile, std::vector<int> &req_v){
		int bytes;
		int request_num;
		int handle;
		//open file only read
		handle = open(pfile, O_RDONLY, S_IREAD);
		if(-1 == handle)
			return false;
		for(;;){
			bytes = read(handle, &request_num, sizeof(int));
			if(0 >= bytes) {
				break;
			}
			req_v.push_back(request_num);
		}
		return true;
	}

//从配置文件中读取服务器器信息(ip+port)
	static void GetServersFromFile(const char *filePtah, std::vector<ServerStruct> &servers){
		fstream in(filePtah, ios::in);
		char line[100];
		if(in){
			while(in.getline(line, 100)){
				// cout << line << endl;
				ServerStruct ser;
				strcpy(ser.str_ip, strtok(line, SPLIT_CHAR));   //以SPLIT_CHAR分隔ip、port
				Helper::charToInt(strtok(NULL, SPLIT_CHAR), ser.port);
				servers.push_back(ser);
				cout << ser.str_ip << " " << ser.port << endl;
			}
		}else{
			cout << "no server config file!" << endl;
		}
	}

//字符串转 int
	static void  charToInt(const char *str, int &s){
		s = 0;
		for (int i = 0; '\0' != str[i]; ++i)
		{
			s = 10 * s + (str[i] - '0');
		}
	}
};
#endif
