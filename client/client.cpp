#include "client.h"
#include "../common/selfdefine.h"
#include <sys/socket.h>
#include <netinet/in.h>	//IPv4套接口地址结构
#include <memory.h> 	//内存操作函数
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>	//itoa atoi
#include <stdio.h>
using namespace std;
Client::Client(int server_port, const char *server_ip_str){
	if(NULL == server_ip_str){
			m_server_ip_str = NULL;
		}else{
			int len = strlen(server_ip_str) + 1;
			m_server_ip_str = new char[len];
			memcpy(m_server_ip_str, server_ip_str, len);	//复制ip字符串
		}
		m_server_port = server_port;
}

Client::~Client(){
	if(NULL != m_server_ip_str){
		delete[] m_server_ip_str;
	}
	if(-1 != m_socket){
		close(m_socket);
	}
}

void Client::work(){
	if(false == init())
		return;
	pthread_t my_p_thread;
	// *main_socket = m_socket;
	int int_socket = m_socket;
	void *pvoid_socket = static_cast<void *>(&int_socket);
	//启动子线程 接收服务端回复消息
	pthread_create(&my_p_thread, NULL, &Client::clientRecieveResponse, pvoid_socket);
	// thread listen_response(clientRecieveResponse, m_socket);
	for(;;){	//发送请求 子线程接收返回
		if(false == clientSendRequest())
			break;
	}
	// return;
}

//输入两整数，发送给服务器
bool Client::clientSendRequest() {
	char buf[LISTEN_QUEUE_MAX_LENGTH];
	int one, two;
	cin >> one >> two;	//input two int
	char str[16];
	sprintf(str, "%d,", one);
	strcpy(buf, str);		//"one"
	sprintf(str, "%d", two);
	strcat(buf, str);		//" one two"
	if(-1 == send(m_socket, buf, LISTEN_QUEUE_MAX_LENGTH, 0)){
		cout << "error: client send request error!" << endl;
		return false;
	}
	return true;
}

//静态函数  子线程 接收服务器返回值
void* Client::clientRecieveResponse(void *pvoid_socket){
	char buf[LISTEN_QUEUE_MAX_LENGTH];
	int *pint_socket = static_cast<int *>(pvoid_socket);
	for(;;){
		if(0 < recv(*pint_socket, buf, sizeof(buf), 0)){
			std::cout <<  buf << std::endl;
		}else{
			break;
		}
	}
}


bool Client::initSocket(){
	//AF_INET		IPv4协议
	//SOCK_STREAM	字节流套接口
	//0，让系统根据family和type的取值，选取合适的protocol的值
	m_socket = socket(AF_INET, SOCK_STREAM, 0);
	cout << "m_socket = " << m_socket << endl;
	if(-1 == m_socket){
		cout << "error: client create socket error!" <<endl;
		return false;
	}
	return true;
}

bool Client::initServerIP_Port() {
	if(false == initSocket())
		return false;
	//IPv4套接口地址结构的定义
	//m_server_addr
	//将该地址所占据的内存区域清0
	//void *memset(void *dest, int c, size_t len);
	memset(&m_server_addr, 0, sizeof(sockaddr_in));
	//对sockaddr_in中的字段赋值
	m_server_addr.sin_family = AF_INET;	//ipv4
	m_server_addr.sin_port = htons(m_server_port);	//port
	//将以字符串形式表示的IP地址，存储到sockaddr_in结构体中
	if(-1 == inet_pton(AF_INET, m_server_ip_str, &m_server_addr.sin_addr))
    {
		cout << "error: client inet_pton error!" << endl;
		close(m_socket);
		return false;
    }
    return true;
}

//初始化socket 并发送connect请求  TCP三次握手
bool Client::init(){
	if(false == initServerIP_Port())
		return false;
	//connect 客户向服务器发起TCP连接
	if(-1 == connect(m_socket, (sockaddr*)&m_server_addr, sizeof(sockaddr_in))){
		cout << "error: client connect server error!" << endl;
    	close(m_socket);
    	return false;
    }
	return true;
}
