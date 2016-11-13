#include <iostream>
#include <fstream>
#include <stdio.h>
#include <cstdlib>
#include <thread>
#include <utility>
#include <stdlib.h>
#include <fcntl.h>   /* For O_RDWR */
#include <unistd.h>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>                                                                                                                                                
#include <boost/asio.hpp>
#include <string>
#include "DS.h"

int main()
{
  StatusCode sc;
  // char proto_msg[DS_CHAR_BUF_SIZE], repl_msg[DS_CHAR_BUF_SIZE];
  // memset(proto_msg, 0, DS_CHAR_BUF_SIZE);
  // memset(repl_msg, 0, DS_CHAR_BUF_SIZE);
  // DS_Manager mgr;
  // cout << "Starting server thread" << std::endl;
  // std::thread(DS_RemoteManager::spawnServer, 7003, mgr.rm).detach();
  // cout << "Local processing started" << std::endl;
  // mgr.createFile("hodoriness");
  // //int *n = new int;
  // //*n = 13;
  // //char *fileName = "gooblidddgoobli";
  // //mgr.rm->makeProtocolMsg(DS_PROTO_ALLOC_PAGE, (void *)n, (void *)fileName, (void *)"hey there bro", proto_msg);
  // char *req = "71|7003|tapli.txt|127.0.0.1";
  // memset(proto_msg, 0, DS_CHAR_BUF_SIZE);
  // strcpy(proto_msg, req);
  // cout << "made msg: " << proto_msg << std::endl;
  // mgr.rm->rawSendRecv("127.0.0.1", "7005", proto_msg, repl_msg);
  // cout << "Recvd msg! : " << repl_msg << std::endl;
  // ProtocolParseObj ppo;
  // mgr.rm->parseProtocolMsg(string(repl_msg), ppo);
  // cout << "hogaya saar" << std::endl;
  // cout << ppo.code << ", " << ppo.port << ", " << ppo.fileName << ", " << ppo.ipAddr << "." << std::endl;

  DS_Manager mgr;
  cout << "Starting server thread" << std::endl;
  mgr.rm->spawnServer(7003, mgr.rm);
  return 0;
}