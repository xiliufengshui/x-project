//============================================================================
// LastChangeTime : Time-stamp: <naturezhang 2015/02/09 16:26:38>
// Name           : socket.cpp
// Version        : 1.0
// Copyright      : 裸奔的鸡蛋
// Author         : 张健
// CreatDate      : 2015年02月 3日 PM  8:59 Tue
// Description    : socket 通讯 函数定义部分
//============================================================================

#include "socket.h"

#define BUFFER_SIZE (512*1024)// 初始化接收buffer的大小
#define BUFFER_MAX_SIZE (3*1024*1024) // 接收buffer限制的最大大小

#define SEND_BUFFER_MAX_SIZE ((uint64_t)1024*1024*1024*128) // 发送buffer最大为128G
#define SEND_HEAD_SIZE (30)                       // 序列化发送字符串的长度，所使用的字符串长度

// socket部分
#define SEND_TIMEOUT (10)      // 发送超时
#define RECV_TIMEOUT (10)      // 接收超时
#define SEND_BUFSIZE (32*1024)      // 设置缓冲区大小
#define RECV_BUFSIZE (32*1024)        // 设置缓冲区大小

#define RECV_BUFSIZE_MAX_CACHE (10*1024*1024) // 接收最大缓冲区 10M
#define SEND_BUFSIZE_MAX_CACHE (10*1024*1024) // 发送最大缓冲区 10M
#define RECV_TIMEOUT_MAX (24*3600)            // 最大接收超时 24小时
#define SEND_TIMEOUT_MAX (24*3600)            // 最大发送超时 24小时

// 构造函数
class_socket_server::class_socket_server()
{
     m_stConfigServer.iRecvBufsize = RECV_BUFSIZE;
     m_stConfigServer.iSendBufsize = SEND_BUFSIZE;
     m_stConfigServer.iRecvTimeout = RECV_TIMEOUT;
     m_stConfigServer.iSendTimeout = SEND_TIMEOUT;
     m_iServerSocket = -1;
     m_pstList = NULL;
}

// 析构函数
class_socket_server::~class_socket_server()
{
     if (m_iServerSocket != -1)
     {
          close(m_iServerSocket);
          m_iServerSocket = -1;
     }
     stSocketList *pstTemp = m_pstList;
     stSocketList *pstNext = NULL;
     while (pstTemp != NULL) 
     {
          if(pstTemp->pcBuf != NULL)
          {
               free(pstTemp->pcBuf);
          }
          pstNext = pstTemp->pstNext;
          free(pstTemp);
          pstTemp = pstNext;
     }
}

bool class_socket_server::set_server_recv_buf_size(uint32_t iRecvBufsize)
{
     if(iRecvBufsize > RECV_BUFSIZE_MAX_CACHE)
     {
          std::cout << "error: recevie cache buffer size too large!" << std::endl;
          return false;
     }
     m_stConfigServer.iRecvBufsize = iRecvBufsize;
     return true;
}

bool class_socket_server::set_server_send_buf_size(uint32_t iSendBufsize)
{
     if(iSendBufsize > SEND_BUFSIZE_MAX_CACHE)
     {
          std::cout << "error: send cache buffer size too large!" << std::endl;
          return false;
     }
     m_stConfigServer.iSendBufsize = iSendBufsize;
     return true;
}

bool class_socket_server::set_server_recv_timeout(uint32_t iRecvTimeout)
{
     if(iRecvTimeout > RECV_TIMEOUT_MAX)
     {
          std::cout << "error: receive timeout too large!" << std::endl;
          return false;
     }
     m_stConfigServer.iRecvTimeout = iRecvTimeout;
     return true;
}

bool class_socket_server::set_server_send_timeout(uint32_t iSendTimeout)
{
     if(iSendTimeout > SEND_TIMEOUT_MAX)
     {
          std::cout << "error: send timeout too large!" << std::endl;
          return false;
     }
     m_stConfigServer.iSendTimeout = iSendTimeout;
     return true;
}

// 配置server socket参数 iParameter
// SO_REUSEADDR 1
// SO_RCVBUF 2
// SO_SNDBUF 4
// SO_RCVTIMEO 8
// SO_SNDTIMEO 16
// SO_LINGER 32
// 多个选项取或    SO_REUSEADDR | SO_RCVBUF
bool class_socket_server::set_server_socket(int iSocket, int iParameter)
{
     if ((iParameter & 1) != 0)
     {
          int bReuse = 1;
          if (-1 == setsockopt(iSocket, SOL_SOCKET, SO_REUSEADDR, &bReuse, sizeof(int)))
          {
               std::cout << "error: setting socket of server option failed! (SO_REUSEADDR)" << std::endl;
               return false;
          }
     }
     if ((iParameter & 2) != 0)
     {
          int iRecvBufsize = m_stConfigServer.iRecvBufsize;
          if (-1 == setsockopt(iSocket, SOL_SOCKET, SO_RCVBUF, &iRecvBufsize, sizeof(int)))
          {
               std::cout << "error: setting socket of server option failed! (SO_RCVBUF)" << std::endl;
               return false;
          }
     }
     if ((iParameter & 4) != 0)
     {
          int iSendBufsize = m_stConfigServer.iSendBufsize;
          if (-1 == setsockopt(iSocket, SOL_SOCKET, SO_SNDBUF, &iSendBufsize, sizeof(int)))
          {
               std::cout << "error: setting socket of server option failed! (SO_SNDBUF)" << std::endl;
               return false;
          }
     }
     if ((iParameter & 8) != 0)
     {
          struct timeval tRecvTimeout = {(long)m_stConfigServer.iRecvTimeout, 0};
          if (-1 == setsockopt(iSocket, SOL_SOCKET, SO_RCVTIMEO, &tRecvTimeout, sizeof(struct timeval)))
          {
               std::cout << "error: setting socket of server option failed! (SO_RCVTIMEO)" << std::endl;
               return false;
          }
     }
     if ((iParameter & 16) != 0)
     {
          struct timeval tSendTimeout = {(long)m_stConfigServer.iSendTimeout, 0};
          if (-1 == setsockopt(iSocket, SOL_SOCKET, SO_SNDTIMEO, &tSendTimeout, sizeof(struct timeval)))
          {
               std::cout << "error: setting socket of server option failed! (SO_SNDTIMEO)" << std::endl;
               return false;
          }
     }
     if ((iParameter & 32) != 0)
     {
          struct linger l_slinger;
          l_slinger.l_onoff = 1;
          l_slinger.l_linger = 0;
          if (-1 == setsockopt(iSocket, SOL_SOCKET, SO_LINGER, &l_slinger, sizeof(struct linger)))
          {
               std::cout << "error: setting socket of server option failed! (SO_LINGER)" << std::endl;
               return false;
          }
     }
     return true;
}


bool class_socket_server::init_server(uint16_t port, int iParameter)
{
     if (m_iServerSocket != -1)
     {
          close(m_iServerSocket);
          m_iServerSocket = -1;
     }
     struct sockaddr_in stServerAddr;
     stServerAddr.sin_family=AF_INET;
     stServerAddr.sin_addr.s_addr=htons(INADDR_ANY);
     stServerAddr.sin_port=htons(port);
     m_iServerSocket=socket(AF_INET,SOCK_STREAM,0);
     if(m_iServerSocket == -1)
     {
          std::cout << "error: creat socket server failed!" << std::endl;
          return false;
     }
     if (!set_server_socket(m_iServerSocket, iParameter))
     {
          std::cout << "error: Setting Server Socket failed!" << std::endl;
          std::cout << "iParameter:" << iParameter << std::endl;
          return false;
     }
     if(-1 == bind(m_iServerSocket,(struct sockaddr*)&stServerAddr,sizeof(stServerAddr))) // 返回值 成功0  否则返回 socket_error
     {
          std::cout << "error: bind socket port failed!" << std::endl;
          return false;
     }
     if(-1 == listen(m_iServerSocket,5)) // 返回值 成功0 否则返回 socket_error
     {
          std::cout << "error: socket server listen failed!" << std::endl;
          return false;
     }
     std::cout << "waiting client..." << std::endl;
     return true;
}


int class_socket_server::server_accept() // <0 则错误
{
     struct sockaddr_in stClientAddr;
     socklen_t Length = sizeof(stClientAddr);
     int iClientSocket;
     iClientSocket = accept(m_iServerSocket, (struct sockaddr*)&stClientAddr, &Length);
     if(iClientSocket == -1)
     {
          std::cout << "error: accept client failed!" << std::endl;
          return iClientSocket;
     }
     stSocketList *pstClient = (stSocketList*)malloc(sizeof(stSocketList));
     pstClient->iClientSocket = iClientSocket;
     pstClient->iLengthBuf = BUFFER_SIZE;
     pstClient->iLengthStr = 0;
     pstClient->pcBuf = (char*)malloc(BUFFER_SIZE * sizeof(char));
     pstClient->pstNext = m_pstList;
     m_pstList = pstClient;
     return iClientSocket;
}


bool class_socket_server::server_send(int iClientSocket, stSocketString stSendStr)
{
     if(iClientSocket < 0)
     {
          std::cout << "error: client socket handle wrong!" << std::endl;
          return false;
     }
     if(stSendStr.iLength > SEND_BUFFER_MAX_SIZE)
     {
          std::cout << "error: send buffer size too large!" << std::endl;
          return false;
     }
     char cArraySendBufLength[SEND_HEAD_SIZE+1];
     snprintf(cArraySendBufLength, (SEND_HEAD_SIZE+1), "%30llu", stSendStr.iLength);
     uint64_t iSendCount = 0;
     iSendCount = send(iClientSocket, cArraySendBufLength, SEND_HEAD_SIZE, 0);
     if (iSendCount != SEND_HEAD_SIZE)
     {
          std::cout << "error: send string length failed!" << std::endl;
          return false;
     }
     if(stSendStr.pcString == NULL)
     {
          std::cout << "error: send string conent is NULL!" << std::endl;
          return false;
     }
     iSendCount = send(iClientSocket, stSendStr.pcString, stSendStr.iLength, 0);
     if (iSendCount != stSendStr.iLength)
     {
          std::cout << "error: send text buffer failed!" << std::endl;
          return false;
     }
     return true;
}

stSocketList * class_socket_server::find_client_socket_string(int iClientSocket)
{
     if(iClientSocket < 0)
     {
          std::cout << "error: client socket handle wrong!" << std::endl;
          return NULL;
     }
     stSocketList * pstTemp = m_pstList;
     while (pstTemp != NULL) 
     {
          if(pstTemp->iClientSocket == iClientSocket)
          {
               return pstTemp;
          }
          pstTemp = pstTemp->pstNext;
     }
     return NULL;
}

stSocketString class_socket_server::server_receive(int iClientSocket)
{
     stSocketString stRecvStrError = {0, NULL};
     if(iClientSocket < 0)
     {
          std::cout << "error: client socket handle wrong!" << std::endl;
          return stRecvStrError;
     }
     stSocketList *pstListClient = find_client_socket_string(iClientSocket);
     if(pstListClient == NULL)
     {
          std::cout << "error: can not find client socket string!" << std::endl;
          return stRecvStrError;
     }
     char cArrayLenStr[SEND_HEAD_SIZE + 1];
     int iRecvLen = recv(iClientSocket, cArrayLenStr, SEND_HEAD_SIZE, MSG_WAITALL);
     cArrayLenStr[SEND_HEAD_SIZE] = '\0';
     if (iRecvLen != SEND_HEAD_SIZE)
     {
          std::cout << "error: receive buffer length failed!" << std::endl;
          return stRecvStrError;
     }
     uint64_t iLength = strtoull(cArrayLenStr, 0, 10);
     pstListClient->iLengthStr = iLength;
     if (iLength >= pstListClient->iLengthBuf || pstListClient->iLengthBuf > BUFFER_MAX_SIZE)
     {
          if(pstListClient->pcBuf != NULL)
          {
               free(pstListClient->pcBuf);
          }
          pstListClient->pcBuf = NULL;
          pstListClient->pcBuf = (char*)malloc((iLength + 1) * sizeof(char));
          if(pstListClient->pcBuf == NULL)
          {
               std::cout << "error: malloc buffer failed!" << std::endl;
               return stRecvStrError;
          }
          pstListClient->iLengthBuf = iLength + 1;
     }
     pstListClient->pcBuf[pstListClient->iLengthStr] = '\0';
     int64_t ilRecvLen = recv(iClientSocket, pstListClient->pcBuf, iLength, MSG_WAITALL);
     if (ilRecvLen != (int64_t)pstListClient->iLengthStr)
     {
          std::cout << "error: receive buffer content failed!" << std::endl;
          return stRecvStrError;
     }
     stSocketString stRecvStr = {pstListClient->iLengthStr, pstListClient->pcBuf};
     return stRecvStr;
}

bool class_socket_server::server_close_client(int iClientSocket) // 关闭客户端 并移出队列
{
     if (iClientSocket < 0 || m_pstList == NULL)
     {
          std::cout << "error: close socket failed!" << std::endl;
          return false;
     }
     stSocketList *pstTemp = m_pstList;
     stSocketList *pstNext = NULL;
     if(m_pstList->iClientSocket == iClientSocket)
     {
          pstTemp = m_pstList;
          m_pstList = m_pstList->pstNext;
          free(pstTemp->pcBuf);
          free(pstTemp);
          close(iClientSocket);
     }
     else
     {
          while (pstTemp->pstNext != NULL)
          {
               pstNext = pstTemp->pstNext;
               if(pstNext->iClientSocket == iClientSocket)
               {
                    pstTemp->pstNext = pstNext->pstNext;
                    free(pstNext->pcBuf);
                    free(pstNext);
                    close(iClientSocket);
               }
               pstTemp = pstTemp->pstNext;
          }
     }
     return true;
}


//  ****************************客户端部分

// 构造函数
class_socket_client:: class_socket_client()
{
     m_stConfigClient.iRecvBufsize = RECV_BUFSIZE;
     m_stConfigClient.iSendBufsize = SEND_BUFSIZE;
     m_stConfigClient.iRecvTimeout = RECV_TIMEOUT;
     m_stConfigClient.iSendTimeout = SEND_TIMEOUT;
     m_iClientSocket = -1;
     m_stClientBuf.iLengthBuf = 0;
     m_stClientBuf.iLengthStr = 0;
     m_stClientBuf.pcBuf = NULL;
}

// 析构函数
class_socket_client::~class_socket_client()
{
     if (m_iClientSocket != -1)
     {
          close(m_iClientSocket);
          m_iClientSocket = -1;
     }
     if(m_stClientBuf.pcBuf != NULL)
     {
          free(m_stClientBuf.pcBuf);
          m_stClientBuf.pcBuf = NULL;
          m_stClientBuf.iLengthBuf = 0;
          m_stClientBuf.iLengthStr = 0;
     }
}

bool class_socket_client::set_client_recv_buf_size(uint32_t iRecvBufsize)
{
     if(iRecvBufsize > RECV_BUFSIZE_MAX_CACHE)
     {
          std::cout << "error: receive cache buffer size too large!" << std::endl;
          return false;
     }
     m_stConfigClient.iRecvBufsize = iRecvBufsize;
     return true;
}

bool class_socket_client::set_client_send_buf_size(uint32_t iSendBufsize)
{
     if(iSendBufsize > SEND_BUFSIZE_MAX_CACHE)
     {
          std::cout << "error: send cache buffer is too large!" << std::endl;
          return false;
     }
     m_stConfigClient.iSendBufsize = iSendBufsize;
     return true;
}

bool class_socket_client::set_client_recv_timeout(uint32_t iRecvTimeout)
{
     if(iRecvTimeout > RECV_TIMEOUT_MAX)
     {
          std::cout << "error: receive timeout is too large!" << std::endl;
          return false;
     }
     m_stConfigClient.iRecvTimeout = iRecvTimeout;
     return true;
}

bool class_socket_client::set_client_send_timeout(uint32_t iSendTimeout)
{
     if(iSendTimeout > SEND_TIMEOUT_MAX)
     {
          std::cout << "error: send timeout is too large!" << std::endl;
          return false;
     }
     m_stConfigClient.iSendTimeout = iSendTimeout;
     return true;
}

bool class_socket_client::init_client(uint16_t iPort, const char *pcIp, int iParameter)
{
     if (m_iClientSocket != -1)
     {
          close(m_iClientSocket);
          m_iClientSocket = -1;
     }
     sockaddr_in ServerAddr;
     ServerAddr.sin_family = AF_INET;
     ServerAddr.sin_addr.s_addr = inet_addr(pcIp);
     ServerAddr.sin_port = htons(iPort);
     m_iClientSocket = socket(AF_INET,SOCK_STREAM,0);
     if (m_iClientSocket == -1)
     {
          std::cout << "error: creat client socket failed!" << std::endl;
          return false;
     }
     if (!set_client_socket(iParameter))
     {
          std::cout << "error: Setting Client Socket failed!     iParameter:" << iParameter  << std::endl;
          return false;
     }
     if(connect(m_iClientSocket, (struct sockaddr*)&ServerAddr, sizeof(ServerAddr)) == -1)
     {
          std::cout << "error: connect server failed!" << std::endl;
          return false;
     }
     return true;
}

// 配置client socket参数
// SO_REUSEADDR 1
// SO_RCVBUF 2
// SO_SNDBUF 4
// SO_RCVTIMEO 8
// SO_SNDTIMEO 16
// SO_LINGER 32
// 多个选项取或    SO_REUSEADDR | SO_RCVBUF 
bool class_socket_client::set_client_socket(int iParameter)
{
     if ((iParameter & 1) != 0)
     {
          int iReuse = 1;
          if (-1 == setsockopt(m_iClientSocket, SOL_SOCKET, SO_REUSEADDR, &iReuse, sizeof(int)))
          {
               std::cout << "error: setting socket of server option failed! (SO_REUSEADDR)" << std::endl;
               return false;
          }
     }
     if ((iParameter & 2) != 0)
     {
          int iRecvBufsize = m_stConfigClient.iRecvBufsize;
          if (-1 == setsockopt(m_iClientSocket, SOL_SOCKET, SO_RCVBUF, &iRecvBufsize, sizeof(int)))
          {
               std::cout << "error: setting socket of server option failed! (SO_RCVBUF)" << std::endl;
               return false;
          }  
     }
     if ((iParameter & 4) != 0)
     {
          int iSendBufsize = m_stConfigClient.iSendBufsize;
          if (-1 == setsockopt(m_iClientSocket, SOL_SOCKET, SO_SNDBUF, &iSendBufsize, sizeof(int)))
          {
               std::cout << "error: setting socket of server option failed! (SO_SNDBUF)" << std::endl;
               return false;
          }
     }
     if ((iParameter & 8) != 0)
     {
          struct timeval stRecvTimeout = {(long)m_stConfigClient.iRecvTimeout , 0};
          if (-1 == setsockopt(m_iClientSocket, SOL_SOCKET, SO_RCVTIMEO, &stRecvTimeout, sizeof(struct timeval)))
          {
               std::cout << "error: setting socket of server option failed! (SO_RCVTIMEO)" << std::endl;
               return false;
          }
     }
     if ((iParameter & 16) != 0)
     {
          struct timeval stSendTimeout = {(long)m_stConfigClient.iSendTimeout , 0};
          if (-1 == setsockopt(m_iClientSocket, SOL_SOCKET, SO_SNDTIMEO, &stSendTimeout, sizeof(struct timeval)))
          {
               std::cout << "error: setting socket of server option failed! (SO_SNDTIMEO)" << std::endl;
               return false;
          }
     }
     if ((iParameter & 32) != 0)
     {
          struct linger l_slinger;
          l_slinger.l_onoff = 1;
          l_slinger.l_linger = 0;
          if (-1 == setsockopt(m_iClientSocket, SOL_SOCKET, SO_LINGER, &l_slinger, sizeof(struct linger)))
          {
               std::cout << "error: setting socket of server option failed! (SO_LINGER)" << std::endl;
               return false;
          }
     }
     return true;
}


stSocketString class_socket_client::client_receive()
{
     stSocketString stRecvStrError = {0, NULL};
     char cArrayLenStr[SEND_HEAD_SIZE + 1];
     int iRecvLen = recv(m_iClientSocket, cArrayLenStr, SEND_HEAD_SIZE, MSG_WAITALL);
     cArrayLenStr[SEND_HEAD_SIZE] = '\0';
     if (iRecvLen != SEND_HEAD_SIZE)
     {
          std::cout << "error: receive buffer failed!" << std::endl;
          return stRecvStrError;
     }
     uint64_t iLength = strtoull(cArrayLenStr, 0, 10);
     m_stClientBuf.iLengthStr = iLength;
     if(iLength > m_stClientBuf.iLengthBuf || m_stClientBuf.iLengthBuf > BUFFER_MAX_SIZE)
     {
          if(m_stClientBuf.pcBuf != NULL)
          {
               free(m_stClientBuf.pcBuf);
          }
          m_stClientBuf.pcBuf = NULL;
          m_stClientBuf.pcBuf = (char*)malloc((iLength + 1) * sizeof(char));
          if(m_stClientBuf.pcBuf == NULL)
          {
               std::cout << "error: malloc buffer failed!" << std::endl;
               return stRecvStrError;
          }
          m_stClientBuf.iLengthBuf = iLength + 1;
     } 
     m_stClientBuf.pcBuf[m_stClientBuf.iLengthStr] = '\0';

     int64_t ilRecvLen = recv(m_iClientSocket, m_stClientBuf.pcBuf, iLength, MSG_WAITALL);
     if (ilRecvLen != (int64_t)m_stClientBuf.iLengthStr)
     {
          std::cout << "error: receive buffer content failed!" << std::endl;
          return stRecvStrError;
     }
     stSocketString stRecvStr = {m_stClientBuf.iLengthStr, m_stClientBuf.pcBuf};
     return stRecvStr;
}

bool class_socket_client::client_send(stSocketString stSendStr)
{
     if(stSendStr.iLength > SEND_BUFFER_MAX_SIZE)
     {
          std::cout << "error: send buffer size too large!" << std::endl;
          return false;
     }
     char cArraySendBufLength[SEND_HEAD_SIZE+1];
     snprintf(cArraySendBufLength, (SEND_HEAD_SIZE+1), "%30llu", stSendStr.iLength);
     int iSendCount = 0;
     iSendCount = send(m_iClientSocket, cArraySendBufLength, SEND_HEAD_SIZE, 0);
     if (iSendCount != SEND_HEAD_SIZE)
     {
          std::cout << "error: send string failed!" << std::endl;
          return false;
     }
     if(stSendStr.pcString == NULL)
     {
          std::cout << "error: send string conent is NULL!" << std::endl;
          return false;
     }
     int64_t ilSendCount = send(m_iClientSocket, stSendStr.pcString, stSendStr.iLength, 0);
     if (ilSendCount != (int64_t)stSendStr.iLength)
     {
          std::cout << "error: send text buffer failed!" << std::endl;
          return false;
     }
     return true;
}


bool class_socket_client::client_close()
{
     if (m_iClientSocket != -1)
     {
          close(m_iClientSocket);
          m_iClientSocket = -1;
     }
     if(m_stClientBuf.pcBuf != NULL)
     {
          free(m_stClientBuf.pcBuf);
          m_stClientBuf.pcBuf = NULL;
          m_stClientBuf.iLengthBuf = 0;
          m_stClientBuf.iLengthStr = 0;
     }
}

