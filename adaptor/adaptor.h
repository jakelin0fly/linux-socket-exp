#ifndef ADAPTOR_H
#define ADAPTOR_H
#include "../common/selfdefine.h"
#include <vector>

class Adaptor
{
public:
	//初始化 服务器列表、adaptor监听端口
	Adaptor(const std::vector<ServerStruct> &servers, int port);
	virtual ~Adaptor();

public:
	void work();
	void closeListenSocket();	//关闭监听socket
	void closeClientSocket();	//关闭客户端连接socket
	void closeServerSocket();	//关闭服务器连接socket

	//for client
	void responseClient();
	// static void *responseClientThread(void *);
	int getServerSocket() { return m_connected_server_socket; }

	//连接指定服务器
	bool initConnectServer(const ServerStruct &server);
	// bool connectToServer(const ServerStruct &server);

	//check server status
	static void *checkServerThread(void *pvoid_this);
	// int signal_send;	//send to server number
	char static_buf[LISTEN_QUEUE_MAX_LENGTH] = ACK;
	static void *sendToServerThread(void *pvoid_this);
	int static_send_num;	//记录验证消息发送数目

private:
	bool initListenClient();	//init for listen client
	bool connectServerByIndex(int &server_index);	//向vector数组中指定服务器发送连接请求

	bool sendToServer(char *buf);	//向已连接服务器发送数据
	//for server
	bool recieveServer(char *buf); //接收服务器返回数据

	bool forwardServer(char *buf);	//数据报转发服务器

private:
	std::vector<ServerStruct> m_servers;	//server vector
	int m_listen_socket;	//adaptor listen socket
	int m_port;				//adaptor port
	int m_connected_client_socket;	//connect client
	int m_connected_server_socket;	//connect server
};
#endif
