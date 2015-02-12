//============================================================================
// LastChangeTime : Time-stamp: <naturezhang 2015/02/09 15:11:08>
// Name           : socket.h
// Version        : 1.0
// Copyright      : 裸奔的鸡蛋
// Author         : 张健
// CreatDate      : 2015年02月 3日 PM  4:50 Tue
// Description    : socket.cpp 的头文件
//============================================================================


#ifndef SOCKET_H
#define SOCKET_H

#include <arpa/inet.h>
/* #include <netinet/in.h> */
/* #include <sys/types.h> */
/* #include <sys/socket.h> */
/* #include <stdio.h> */
/* #include <string> */
#include <string.h>
#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>


/* using namespace std; */

struct stSocketConfig
{
     uint32_t iSendBufsize;
     uint32_t iRecvBufsize;
     uint32_t iSendTimeout;
     uint32_t iRecvTimeout;
};

struct stSocketString
{
     const uint64_t iLength;
     const char * const pcString;
};

struct stSocketList
{
     int iClientSocket;
     uint64_t iLengthBuf;
     uint64_t iLengthStr;
     char *pcBuf;
     stSocketList *pstNext;
};

struct stSocketClientBuf
{
     uint64_t iLengthBuf;
     uint64_t iLengthStr;
     char *pcBuf;
};

class class_socket_server
{
public:
     class_socket_server();
     virtual ~class_socket_server();
     bool set_server_recv_buf_size(uint32_t iRecvBufsize);
     bool set_server_send_buf_size(uint32_t iSendBufsize);
     bool set_server_recv_timeout(uint32_t iRecvTimeout);
     bool set_server_send_timeout(uint32_t iSendTimeout);
     bool set_server_socket(int iSocket, int iParameter);
     bool init_server(uint16_t port, int iParameter);
     int server_accept();      /* 返回客户端句柄 <0 出错 */
     bool server_send(int iClientSocket, stSocketString stSendStr);
     stSocketList * find_client_socket_string(int iClientSocket);
     stSocketString server_receive(int iClientSocket);
     bool server_close_client(int iClientSocket);// 关闭客户端 并移出队列
     
private:
     stSocketConfig m_stConfigServer;
     int m_iServerSocket;       /* 服务端 句柄 */
     stSocketList *m_pstList;
};


class class_socket_client
{
public:
     class_socket_client();
     virtual ~class_socket_client();
     bool set_client_recv_buf_size(uint32_t iRecvBufsize);
     bool set_client_send_buf_size(uint32_t iSendBufsize);
     bool set_client_recv_timeout(uint32_t iRecvTimeout);
     bool set_client_send_timeout(uint32_t iSendTimeout);
     bool init_client(uint16_t iPort,const char *pcIp, int iParameter);
     bool set_client_socket(int iParameter);
     stSocketString client_receive();/* 开头 c 表示客户端 */
     bool client_send(stSocketString stSendStr);
     bool client_close();

private:
     stSocketConfig m_stConfigClient;
     int m_iClientSocket;
     stSocketClientBuf m_stClientBuf;
};

#endif /* SOCKET_H */
