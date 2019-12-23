#ifndef ClIENT_H
#define ClIENT_H
// #define	MAX_BUFFER	200
#include <netinet/in.h>	//IPv4套接口地址结构
// #include <iostream>
typedef struct socketStruct
{
	int socket;
}socketStruct;
class Client
{
public:
	Client(int server_port, const char *server_ip_str);
	virtual ~Client();

public:
	bool clientSendRequest();
	static void* clientRecieveResponse(void *);	//线程函数 接收服务器返回消息
	// static void* threadClient(void *obj);
	void work();	

private:
	bool initSocket();	//初始化 socket
	bool initServerIP_Port();	//初始化 服务器（adaptor）ip port
	bool init();	//完成所有初始化操作

private:
	int m_server_port;	//客户端所连接服务器（adaptor）端口
	char *m_server_ip_str;	//ip字符串
	int m_socket;
	sockaddr_in m_server_addr;
};

#endif
