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
  char proto_msg[DS_CHAR_BUF_SIZE];
  memset(proto_msg, 0, DS_CHAR_BUF_SIZE);
  DS_Manager mgr;
  cout << "Starting server thread" << std::endl;
  std::thread(DS_RemoteManager::spawnServer, 7001, mgr.rm).detach();
  cout << "Local processing started" << std::endl;
  mgr.createFile("hodor");
  int *n = new int;
  *n = 13;
  char *fileName = "gooblidddgoobli";
  mgr.rm->makeProtocolMsg(DS_PROTO_LOAD_PAGE, (void *)n, (void *)fileName, NULL, proto_msg);
  cout << "made msg: " << proto_msg << std::endl;
  return 0;
}