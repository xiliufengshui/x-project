//============================================================================
// LastChangeTime : Time-stamp: <naturezhang 2015/02/09 16:08:45>
// Name           : server.cpp
// Version        : 1.0
// Copyright      : 裸奔的鸡蛋
// Author         : 张健
// CreatDate      : 2015年02月 4日 PM  4:59 Wed
// Description    : server
//============================================================================

#include "socket.h"

int main(int argc, char *argv[])
{
     class_socket_server clServer;
     clServer.init_server(8456, 39);
     int iClientSocket = -1;
here:
     iClientSocket = clServer.server_accept();
     if(iClientSocket < 0)
     {
          std::cout << "2222222" << std::endl;
          goto here;
     }
     std::cout << "begin receive!" << std::endl;
     // stSocketString stRecv = clServer.server_receive(iClientSocket);
     // if(stRecv.pcString == NULL || stRecv.iLength == 0)
     // {
     //      std::cout << "error: receive string failed!" << std::endl;
     //      return 0;
     // }
     // std::cout << stRecv.pcString << std::endl;
     // char * pTmp = (char*)stRecv.pcString;
     // std::cout << pTmp << std::endl;
     // std::cout << "finish..." << std::endl;
     // std::cout << "begin send string ..." << std::endl;
     // if(!clServer.server_send(iClientSocket, stRecv))
     // {
     //      std::cout << "error: send string failed!" << std::endl;
     //      return -1;
     // }
     sleep(1000*1000);
     return 0;
}
