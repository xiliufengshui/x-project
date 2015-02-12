//============================================================================
// LastChangeTime : Time-stamp: <naturezhang 2015/02/09 16:24:41>
// Name           : client.cpp
// Version        : 1.0
// Copyright      : 裸奔的鸡蛋
// Author         : 张健
// CreatDate      : 2015年02月 4日 PM  5:26 Wed
// Description    : client
//============================================================================


#include "socket.h"


int main(int argc, char *argv[])
{
     class_socket_client clClient;
     clClient.set_client_send_timeout(3);
     clClient.set_client_send_buf_size(128*1024);
     clClient.init_client(8456, "127.0.0.1", 63);
     std::cout << "connect finish!" << std::endl;
     char *pcBuf = (char*)malloc(sizeof(char) * 128 * 1024);
     memset(pcBuf, 'c', 128*1024);
     pcBuf[64*1024] = '\0';
     // stSocketString stSend = {iLength : 128*1024-1,
     //                          pcString : pcBuf};
     stSocketString stSend = {128*1024-1, pcBuf};
     if(!clClient.client_send(stSend))
     {
          std::cout << "error: send stirng failed!" << std::endl;
          return -1;
     }
     // stSocketString stRecv = clClient.client_receive();
     // clClient.client_send("nihaoma");
     std::cout << "finish..." << std::endl;
     // std::cout << "begin recevie string ..." << std::endl;
     // stSocketString stRecv = clClient.client_receive();
     // std::cout << stRecv.pcString << std::endl;
     sleep(1000*1000);
     return 0;
}
