#include "adaptor.h"
#include "../common/selfdefine.h"
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>	//IPv4套接口地址结构
#include <memory.h> 	//内存操作函数
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/types.h>	//fork
#include <iostream>
#include <pthread.h>
#include <stdlib.h>	//itoa atoi
#include <signal.h>	//int pthread_kill(pthread_t thread, int sig);
#include <fcntl.h>
#include <sys/select.h>
#include <errno.h>
using namespace std;

Adaptor::Adaptor(const std::vector<ServerStruct> &servers, int port){
	//save servers
	for (int i = 0; i < servers.size(); ++i)
	{
		m_servers.push_back(servers[i]);
	}
	m_port = port;
}

Adaptor::~Adaptor(){

}

void Adaptor::work(){
	if(false == initListenClient())
		return;
	pid_t child_pid;	//fork child pid
	for(;;){
		sockaddr_in client_addr;
		socklen_t client_addr_len = sizeof(sockaddr_in);
		m_connected_client_socket = accept(m_listen_socket,
									(sockaddr *)&client_addr,
									&client_addr_len);
		if(-1 == m_connected_client_socket){
			cout << "error: adaptor accept error!" << endl;
			continue;
		}
		//create child process 子进程
		if(0 == (child_pid = fork())){
			//child process do
			closeListenSocket(); //子进程 关闭监听
			responseClient();	//子进程 响应已连接客户端
			exit(0);	//close child
		}
		//parent process do
		closeClientSocket();	//父进程 继续监听、关闭已连接的客户端连接
	}
}

	//for client
void Adaptor::responseClient(){
	// pthread_t my_client_thread;
	// pthread_t check_server_thread;
	// int int_client_socket = m_connected_client_socket;
	int server_index = 0;	//mark server
	//adaptor connect server
	if(false == connectServerByIndex(server_index)){
		send(m_connected_client_socket, "error: can not connect servers! all", 50, 0);
		closeClientSocket();
		return;
	}
	char buf[LISTEN_QUEUE_MAX_LENGTH] = "connect server success! ";
	{
		char str[16];
		sprintf(str, ":%d", m_servers[server_index].port);
		strcat(buf, m_servers[server_index].str_ip);
		strcat(buf, str);
	}
	if(-1 == send(m_connected_client_socket, buf, LISTEN_QUEUE_MAX_LENGTH, 0)){
		closeServerSocket();
		closeClientSocket();
		cout << "close server & client" << endl;
		return;	//send to client error
	}

	for(;;){
		//recieve client request
		if(0 < recv(m_connected_client_socket, buf, LISTEN_QUEUE_MAX_LENGTH, 0)){
			strcpy(this->static_buf, ACK);
			if(false == forwardServer(buf)){
				cout << "forwardServer error!" <<endl;
				break;
			}
		}else{
			break;
		}
	}
	closeServerSocket();
	closeClientSocket();
}

void *Adaptor::checkServerThread(void *pvoid_this){
	Adaptor *pthis = static_cast<Adaptor *>(pvoid_this);
	int server_socket = pthis->getServerSocket();
	for(;;){
		//wait server response
		// if(0 < recv(server_socket, pthis->static_buf, LISTEN_QUEUE_MAX_LENGTH, 0)){
		//
		cout << "thread check run" <<endl;
		{
			pthread_testcancel();
			// cout << "recieve ";
			// cout << recv(server_socket, pthis->static_buf, LISTEN_QUEUE_MAX_LENGTH, 0)
			// <<": " << pthis->static_buf << endl;
			// if(0 != strcmp(ACK, pthis->static_buf))
			// 	return NULL;
			if(0 < recv(server_socket, pthis->static_buf, LISTEN_QUEUE_MAX_LENGTH, MSG_DONTWAIT)){
				pthis->static_send_num = 0;
			}
		}
		sleep(ADAPTOR_THREAD_SLEEP_TIME);
	}
	return NULL;
}

void *Adaptor::sendToServerThread(void *pvoid_this){
	Adaptor *pthis = static_cast<Adaptor *>(pvoid_this);
	int server_socket = pthis->getServerSocket();
	for(;;){
		//send to server
		sleep(ADAPTOR_THREAD_SLEEP_TIME);
		// cout << "thread send run" <<endl;
		// if((pthis->static_send_num >= MAX_CHECK_NUM) || (0 != strcmp(ACK, pthis->static_buf))){
		// 	return NULL;
		// }
		pthread_testcancel();
		// send(server_socket, ACK, LISTEN_QUEUE_MAX_LENGTH, 0);
		pthis->static_send_num += 1;
		cout << "send num: " << pthis->static_send_num << endl;
		if((pthis->static_send_num >= MAX_CHECK_NUM) || (0 != strcmp(ACK, pthis->static_buf))){
			return NULL;
		}
	}
	return NULL;
}

	//for server
bool Adaptor::recieveServer(char *buf){
	//wait server response
	if(0 < recv(m_connected_server_socket, buf, LISTEN_QUEUE_MAX_LENGTH, 0)){
		return true;
	}
	return false;
}

void Adaptor::closeListenSocket() {
	if(-1 != m_listen_socket){
		close(m_listen_socket);
		m_listen_socket = -1;
	}
}
void Adaptor::closeClientSocket(){
	if(-1 != m_connected_client_socket){
		close(m_connected_client_socket);
		m_connected_client_socket = -1;
	}
}
void Adaptor::closeServerSocket(){
	if(-1 != m_connected_server_socket){
		close(m_connected_server_socket);
		m_connected_server_socket = -1;
	}
}

bool Adaptor::initListenClient(){
	//AF_INET		IPv4协议
	//SOCK_STREAM	字节流套接口
	//0，让系统根据family和type的取值，选取合适的protocol的值
	m_listen_socket = socket(AF_INET, SOCK_STREAM, 0);
	// cout << "m_listen_socket = " << m_listen_socket <<endl;
	if(-1 == m_listen_socket){
		cout << "error: adaptor m_listen_socket error!" <<endl;
		return false;
	}
	//IPv4套接口地址结构的定义
	sockaddr_in server_addr;
	//将该地址所占据的内存区域清0
	//void *memset(void *dest, int c, size_t len);
	memset(&server_addr, 0, sizeof(sockaddr_in));

	//对sockaddr_in中的字段赋值
	server_addr.sin_family = AF_INET;	//ipv4
	server_addr.sin_port = htons(m_port);	//port
	//INADDR_ANY 表示服务器可以在任意网络接口上接受客户连接
	// server_addr.sin_addr.s_addr = htonl(INADDR_ANY);	//ip
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	//即将套接口地址信息与套接口绑定
	if(-1 == bind(m_listen_socket, (sockaddr *)&server_addr, sizeof(sockaddr_in))){
		cout << "error: adaptor bind error!" << endl;
			closeListenSocket();
		return false;
	}
	//listen函数把一个未连接的套接口转换成一个被动套接口
	//指示内核应接收指向该套接口的连接请求
	if(-1 == listen(m_listen_socket, LISTEN_QUEUE_MAX_LENGTH)){
		cout << "error: adaptor listen error!" << endl;
		closeListenSocket();
		return false;
	}
	return true;
}

bool Adaptor::initConnectServer(const ServerStruct &server) {
	//AF_INET		IPv4协议
	//SOCK_STREAM	字节流套接口
	//0，让系统根据family和type的取值，选取合适的protocol的值
	m_connected_server_socket = socket(AF_INET, SOCK_STREAM, 0);
	cout << "server " << server.str_ip << ":" << server.port << endl;
	if(-1 == m_connected_server_socket){
		cout << "error: adaptor create m_connected_server_socket error!" <<endl;
		return false;
	}
	//IPv4套接口地址结构的定义
	sockaddr_in server_addr;
	//将该地址所占据的内存区域清0
	//void *memset(void *dest, int c, size_t len);
	memset(&server_addr, 0, sizeof(sockaddr_in));
	//对sockaddr_in中的字段赋值
	server_addr.sin_family = AF_INET;	//ipv4
	server_addr.sin_port = htons(server.port);	//port
	//将以字符串形式表示的IP地址，存储到sockaddr_in结构体中
	if(-1 == inet_pton(AF_INET, server.str_ip, &server_addr.sin_addr))
    {
		cout << "error: adaptor inet_pton error!" << endl;
		closeServerSocket();
		return false;
    }
    //connect 向服务器发起TCP连接  阻塞模式
	if(-1 == connect(m_connected_server_socket, (sockaddr*)&server_addr, sizeof(sockaddr_in))){
		cout << "error: client connect server error!" << endl;
    	closeServerSocket();
    	return false;
    }
		//  非阻塞模式(O_NONBLOCK)
	 	int oldSocketFlag = fcntl(m_connected_server_socket, F_GETFL, 0);
	 	fcntl(m_connected_server_socket, F_SETFL,  oldSocketFlag | O_NONBLOCK); //O_NONBLOCK

 // //  非阻塞模式(O_NONBLOCK) 连接服务器
 //  int oldSocketFlag = fcntl(m_connected_server_socket, F_GETFL, 0);
	// fcntl(m_connected_server_socket, F_SETFL,  oldSocketFlag | O_NONBLOCK); //O_NONBLOCK
	// int n;
	// if(0 > (n = connect(m_connected_server_socket, (sockaddr*)&server_addr, sizeof(sockaddr_in)))){
	// 	if(errno != EINPROGRESS){
	// 		cout << "errno = " << errno << endl;
	// 		closeServerSocket();
	// 		return false;
	// 	}
	// }
	// if(0 == n){
	// 	cout << "connect success n=0!" <<endl;
	// 	return true;
	// }
	// //select 非阻塞模式
	// {
	// 	fd_set rset, wset;
	// 	struct timeval tval;
	// 	FD_ZERO(&rset);
	// 	FD_SET(m_connected_server_socket, &rset);
	// 	wset = rset;
	// 	tval.tv_sec = 3;	//连接超时 3秒
	// 	tval.tv_usec = 0;
	// 	if((n = select(m_connected_server_socket+1, &rset, &wset,NULL,&tval)) == 0){
	// 		cout << "connect select time out!" <<endl;
	// 		// errno = ETIMEOUT;
	// 		closeServerSocket();
	// 		return false;
	// 	}
	// 	if(FD_ISSET(m_connected_server_socket, &rset) ||
	// 	 FD_ISSET(m_connected_server_socket, &wset)){
	// 		 int error;
	// 		 socklen_t len = sizeof(error);
	// 		 if(getsockopt(m_connected_server_socket,SOL_SOCKET,SO_ERROR,&error,&len) < 0){
	// 			 closeServerSocket();
	// 			 return false;
	// 		 }
	// 	}else{
	// 		closeServerSocket();
	// 		return false;
	// 	}
	// }
    cout << "connect success! O_NONBLOCK " << m_connected_server_socket << endl;
    return true;
}

bool Adaptor::connectServerByIndex(int &server_index){
	std::cout << "server size : " << m_servers.size() << '\n';
	for (; server_index < m_servers.size(); ++server_index)
	{	//response client connect server success
		if(true == initConnectServer(m_servers[server_index])){
			return true;	//connect server success
		}
	}
	return false;
}

bool Adaptor::sendToServer(char *buf){
	if(-1 == send(m_connected_server_socket, buf, LISTEN_QUEUE_MAX_LENGTH, 0))
		return false;
	return true;
}

bool Adaptor::forwardServer(char *buf){
	strcpy(this->static_buf, ACK);
	//send to server
	if(-1 == send(m_connected_server_socket, buf,
		LISTEN_QUEUE_MAX_LENGTH, MSG_DONTWAIT)){
		//server error, now request don't send to server
		for(;;){
			int server_index = 0;
			if(false == connectServerByIndex(server_index)){
				send(m_connected_client_socket, "error: connect servers error!", 50, 0);
				closeServerSocket();
				closeClientSocket();
				return false;
			}
		}
	}
	//wait server response
	this->static_send_num = 0;
	pthread_t check_server_thread, send_to_server_thread;
	void *p = static_cast<void *>(this);
	pthread_create(&check_server_thread, NULL,
				&Adaptor::checkServerThread, p);
	pthread_create(&send_to_server_thread, NULL,
				&Adaptor::sendToServerThread, p);
//wait server response
	while(true){ // && this->static_send_num < MAX_CHECK_NUM
		if(0 != strcmp(ACK, this->static_buf)){
			strcpy(buf, this->static_buf);
			// strcpy(this->static_buf, ACK);
			cout << "OK " << buf << endl;
			break;
		}
		// cout <<"this->static_send_num =" <<  this->static_send_num << endl;
		if(this->static_send_num >= MAX_CHECK_NUM){
			cout << "time out!" <<endl;
			pthread_cancel(send_to_server_thread);
			pthread_cancel(check_server_thread);
			//do connect anther server
			closeServerSocket();
			int server_index = 0;
			if(true == connectServerByIndex(server_index)){
				//connect anther server success
				strcpy(this->static_buf, ACK);
				return forwardServer(buf);
			}
			send(m_connected_client_socket, "error: can not connect another servers!", 60, 0);
			closeServerSocket();
			closeClientSocket();
			return false;
		}
		// cout << "send to server : " << sendToServer(this->static_buf)
		// << "signal_send = :" << this->static_send_num <<endl;
		// signal_send += 1;
		sleep(2);	//sleep
	} //end wait server response
	// strcpy(this->static_buf, ACK);
	pthread_cancel(send_to_server_thread);
	pthread_cancel(check_server_thread);
	//response client answer
	if(0 >= send(m_connected_client_socket, buf, LISTEN_QUEUE_MAX_LENGTH, 0)){
		closeServerSocket();
		closeClientSocket();
		cout << "client disconnect!" << endl;
		return false;
	}
	return true;
}
