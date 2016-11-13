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

using namespace std;

DS_Manager::DS_Manager()
{
  rm = new DS_RemoteManager();
  bm = new DS_BufferManager(DS_BFRMGR_NUMPAGES, rm);
}

DS_Manager::~DS_Manager()
{

}

StatusCode DS_Manager::createFile(char *fileName)
{
  StatusCode sc;
  if (fileName == NULL) {
    sc = DS_NULL_PARAM;
    return sc;
  }
  ofstream file(fileName, ios::out | ios::binary);
  if (file.is_open()) {
    char *hdr_buf = new char[DS_FILE_HDR_SIZE];
    memset(hdr_buf, 0, DS_FILE_HDR_SIZE);

    DS_FileHeader *hdr = (DS_FileHeader*) hdr_buf;
    hdr->firstFree = DS_END_FILE;
    hdr->numPages = 0;

    ofstream file(fileName, ios::out | ios::binary);
    file.write(hdr_buf, DS_FILE_HDR_SIZE);
    file.close();
    sc = DS_SUCCESS;
    return sc;
  }
  else {
    sc = DS_NO_OPEN_FILE;
    return sc;
  }
}

StatusCode DS_Manager::loadFile(char *fileName,
                                DS_FileHandle &fileHandle)
{
  StatusCode sc;
  if (fileName == NULL) {
    sc = DS_NULL_PARAM;
    return sc;
  }
  boost::filesystem::path p(fileName);
  if (boost::filesystem::exists(p)) {
    fileHandle.isRemote = false;
    ifstream file(fileName, ios::in | ios::binary);
    file.read((char *)&fileHandle.hdr, DS_FILE_HDR_SIZE);
    file.close();
    fileHandle.fileName.copy(fileName, strlen(fileName));
    fileHandle.rm = rm;
    fileHandle.bm = bm;
    fileHandle.unixfd = open(fileName, O_RDWR);
    sc = DS_SUCCESS;
  }
  else {
    char headerContent[DS_CHAR_BUF_SIZE], ipAddrStr[DS_CHAR_BUF_SIZE], portStr[DS_CHAR_BUF_SIZE];
    sc = rm->getRemoteHeaderFile(fileName, headerContent, ipAddrStr, portStr);
    fileHandle.isRemote = true;
    cout << "got : " << headerContent << std::endl;
    cout << "got2 : " << ipAddrStr << std::endl << portStr << std::endl << strlen(ipAddrStr) << std::endl;
    memcpy((void *)&fileHandle.hdr, (void *)headerContent, DS_FILE_HDR_SIZE);
    fileHandle.ipaddr = string(ipAddrStr);
    fileHandle.port = string(portStr);
  }
  fileHandle.fileName = string(fileName);
  fileHandle.rm = this->rm;
  fileHandle.bm = this->bm;
  return sc;
}

StatusCode DS_Manager::createRemoteFile(char *fileName, char *ipAddr, char *port)
{
  StatusCode sc;
  if (fileName == NULL || ipAddr == NULL || port == NULL) {
    sc = DS_NULL_PARAM;
    return sc;
  }
  sc = rm->createRemoteFile(fileName, ipAddr, port);
  return sc;
}