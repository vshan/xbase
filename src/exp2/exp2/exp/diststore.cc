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
  DS_Manager mgr;
  cout << "Starting server thread" << std::endl;
  mgr.rm->spawnServer(7003, mgr.rm);
  return 0;
}
