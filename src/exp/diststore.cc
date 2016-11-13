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
  // mgr.loadFile("hodoriness", hdl);
  //DS_FileHandle hdl2;
  mgr.createRemoteFile("dapandudidal", "127.0.0.1", "7003");
  // mgr.loadFile("DS_RemoteManager.cc", hdl);
  // cout << "curr: " << hdl.hdr.firstFree << std::endl;
  // cout << "dirr: " << hdl.hdr.numPages << std::endl;
  // DS_PageHandle ph;
  // hdl.getThisPage(1, ph);
  return 0;
}