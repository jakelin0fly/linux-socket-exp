#ifndef SERVER_H
#define SERVER_H
#include "../common/selfdefine.h"
class Server
{
public:
	Server(const int &port);
	virtual ~Server();

public:
	void closeListenSocket();	//关闭监听socket
	void closeConnectedSocket();	//关闭已连接的socket
	void work();
	void response();	//响应请求

	static void *responseThread(void *); //响应请求子线程
	char buf[LISTEN_QUEUE_MAX_LENGTH];
private:
	bool initListenSocket();	//初始化m_listen_socket  used in runListen()
	bool runListen();	//监听服务器端口消息 阻塞

private:
	int m_port;
	int m_listen_socket;
	int m_connected_socket;

};

#endif
