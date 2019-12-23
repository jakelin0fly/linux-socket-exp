#ifndef SELFDEFINE_H
#define SELFDEFINE_H

#define LISTEN_QUEUE_MAX_LENGTH  	1024
// #define SERVER_LISTEN_PORT			9001

#define	ADAPTOR_LISTEN_PORT			9000
#define SPLIT_CHAR					","

//for check server status
#define	ACK							"ack"
#define MAX_CHECK_NUM				10

//sleep
#define ADAPTOR_THREAD_SLEEP_TIME	2
#define SERVER_WAIT_SLEEP					3
#define SERVER_THREAD_TIME				5

typedef struct ServerStruct
{
	char str_ip[16];
	int port;
}ServerStruct;


#endif
