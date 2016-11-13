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
  char proto_msg[DS_CHAR_BUF_SIZE], repl_msg[DS_CHAR_BUF_SIZE];
  memset(proto_msg, 0, DS_CHAR_BUF_SIZE);
  memset(repl_msg, 0, DS_CHAR_BUF_SIZE);
  DS_Manager mgr;
  cout << "Starting server thread" << std::endl;
  std::thread(DS_RemoteManager::spawnServer, 7001, mgr.rm).detach();
  DS_FileHandle hdl;
  //mgr.loadFile("hodoriness", hdl);
  //DS_FileHandle hdl2;
  //mgr.createRemoteFile("hoodi", "127.0.0.1", "7003");
  mgr.loadFile("hoodi", hdl);
  cout << "curr: " << hdl.hdr.firstFree << std::endl;
  cout << "dirr: " << hdl.hdr.numPages << std::endl;
  DS_PageHandle ph;
  hdl.
  // cout << "Local processing started" << std::endl;
  // mgr.createFile("hodoriness");
  // //int *n = new int;
  // //*n = 13;
  // //char *fileName = "gooblidddgoobli";
  // //mgr.rm->makeProtocolMsg(DS_PROTO_ALLOC_PAGE, (void *)n, (void *)fileName, (void *)"hey there bro", proto_msg);
  // char *req = "71|7003|hodoriness|127.0.0.1";
  // // 10.50.42.233
  // memset(proto_msg, 0, DS_CHAR_BUF_SIZE);
  // strcpy(proto_msg, req);
  // cout << "made msg: " << proto_msg << std::endl;
  // mgr.rm->rawSendRecv("127.0.0.1", "7005", proto_msg, repl_msg);
  // cout << "Recvd msg! : " << repl_msg << std::endl;
  // ProtocolParseObj ppo;
  // mgr.rm->parseProtocolMsg(string(repl_msg), ppo);
  // cout << "hogaya saar" << std::endl;
  // cout << ppo.code << ", " << ppo.port << ", " << ppo.fileName << ", " << ppo.ipAddr << "." << std::endl;
  // memset(proto_msg, 0, DS_CHAR_BUF_SIZE);
  // mgr.rm->getRemotePage(ppo.ipAddr, ppo.port, ppo.fileName, 0, proto_msg);
  // cout << "Mila yaar : " << proto_msg << std::endl;
  // DS_FileHeader hdr;
  // memcpy((void *)&hdr, (void *)proto_msg, DS_FILE_HDR_SIZE);
  // cout << "udiki: " << hdr.firstFree << std::endl;
  // cout << "idiki: " << hdr.numPages << std::endl;
  return 0;
}