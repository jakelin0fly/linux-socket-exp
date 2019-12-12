#ifndef FILEHELPER_H
#define FILEHELPER_H
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include<fcntl.h>
#include <iostream>
#include<vector>

using namespace std;

class FileHelper
{
public:
	FileHelper(){}
	virtual ~FileHelper(){}

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

};
#endif