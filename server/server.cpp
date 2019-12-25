#include "server.h"
#include "../common/selfdefine.h"
#include "../common/helper.h"
#include <sys/socket.h>
#include <netinet/in.h>	//IPv4套接口地址结构
#include <memory.h> 	//内存操作函数
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/types.h>	//fork
#include <iostream>
#include <pthread.h>
#include <stdlib.h>	//itoa atoi
#include <string.h>
#include <string>
#include <stdio.h>
using namespace std;
Server::Server(const int &port) : m_port(port){
	m_listen_socket = -1;
	m_connected_socket = -1;
}

Server::~Server(){

}

void Server::closeListenSocket() {
	if(-1 != m_listen_socket){
		close(m_listen_socket); //close
		m_listen_socket = -1;
	}
}
void Server::closeConnectedSocket(){
	if(-1 != m_connected_socket){
		close(m_connected_socket);	//close
		m_connected_socket = -1;
	}
}

void Server::work(){
	if(false == runListen())
		return;
	pid_t childpid;	//fork child pid
	for(;;){
		sockaddr_in client_addr;
		socklen_t client_addr_len = sizeof(sockaddr_in);
		//监听到连接请求 accept
		m_connected_socket = accept(m_listen_socket,
							(sockaddr *)&client_addr,
							&client_addr_len);
		//连接失败
		if(-1 == m_connected_socket){
			cout << "error: server accept error!" << endl;
			continue;
		}
		//create child process 子进程处理已连接请求
		if(0 == (childpid = fork())) { 	//child process do
			closeListenSocket();	//child process close listen
			response();	//reponse client
			exit(0); //解决客户端关闭 服务端报错问题
		}
		//parent process do 父进程关闭连接，仅负责监听新的连接请求
		closeConnectedSocket();
	}
}

//child process response client request
void Server::response(){
	strcpy(buf, ACK);
	pthread_t my_response_thread;	//response thread
	int int_socket = m_connected_socket;
	void *pvoid_socket = static_cast<void *>(&int_socket);
	pthread_create(&my_response_thread, NULL, &Server::responseThread, pvoid_socket);
	for(;;){
		// char buf[LISTEN_QUEUE_MAX_LENGTH];
		//接收请求数据报，阻塞等待
		if(0 < recv(m_connected_socket, buf, LISTEN_QUEUE_MAX_LENGTH, 0)){
			cout << buf << endl;	//数据报信息
			//验证报文 验证服务器是否在线
			if(0 == strcmp(ACK, buf)){	//check message
				continue;
			}
			//进程等待休眠
			sleep(SERVER_WAIT_SLEEP);
			int one, two;
			char *p = strtok(buf, SPLIT_CHAR);
			Helper::charToInt(p, one);
			p = strtok(NULL, SPLIT_CHAR);
			Helper::charToInt(p, two);
			int ans = one + two;
			sprintf(buf, "%d", ans);
			if(-1 == send(m_connected_socket, buf, LISTEN_QUEUE_MAX_LENGTH, 0)){
				cout << "close m_connected_socket: send ans error" <<endl;
				closeConnectedSocket();
				break;
			}
		}else{	//接收请求失败 客户端关闭
			closeConnectedSocket();
			cout << "client close!" << endl;
			break;
		}
		// sleep(2);
	}

}

void* Server::responseThread(void *pvoid_socket){
	int *pint_socket = static_cast<int *>(pvoid_socket);
	for(;;){
		sleep(SERVER_THREAD_TIME);
		pthread_testcancel();
		if(-1 == send(*pint_socket, ACK, LISTEN_QUEUE_MAX_LENGTH, 0)){
			close(*pint_socket);
			break;
		}
	}
}

bool Server::initListenSocket(){
	//AF_INET		IPv4协议
	//SOCK_STREAM	字节流套接口
	//0，让系统根据family和type的取值，选取合适的protocol的值
	m_listen_socket = socket(AF_INET, SOCK_STREAM, 0);
	// cout << "m_listen_socket = " << m_listen_socket <<endl;
	if(-1 == m_listen_socket){
		cout << "error: server m_listen_socket error!" <<endl;
		return false;
	}
	//IPv4套接口地址结构的定义
	sockaddr_in serverAddr;
	//将该地址所占据的内存区域清0
	//void *memset(void *dest, int c, size_t len);
	memset(&serverAddr, 0, sizeof(sockaddr_in));

	//对sockaddr_in中的字段赋值
	serverAddr.sin_family = AF_INET;	//ipv4
	serverAddr.sin_port = htons(m_port);	//port
	//INADDR_ANY 表示服务器可以在任意网络接口上接受客户连接
	// serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);	//ip
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

		//即将套接口地址信息与套接口绑定
	if(-1 == bind(m_listen_socket, (sockaddr *)&serverAddr, sizeof(sockaddr_in))){
		cout << "error: server bind error!" << endl;
			closeListenSocket();
		return false;
	}
	return true;
}

bool Server::runListen() {
	if(false == initListenSocket())
		return false;
	//listen函数把一个未连接的套接口转换成一个被动套接口
	//指示内核应接收指向该套接口的连接请求
	if(-1 == listen(m_listen_socket, LISTEN_QUEUE_MAX_LENGTH)){
		cout << "error: server listen error!" << endl;
		closeListenSocket();
		return false;
	}
	return true;
}
