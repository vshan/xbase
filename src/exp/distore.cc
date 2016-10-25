#include <iostream>
#include <ofstream>
#include <ifstream>
#include <boost/filesystem.hpp>
#include "distore.h"

DS_Manager::DS_Manager()
{
  bm = new DS_BufferManager();
  rm = new DS_RemoteManager();
}

DS_Manager::~DS_Manager()
{

}

StatusCode DS_Manager::createFile(const char *fileName)
{
  StatusCode sc;
  if (fileName == NULL) {
    sc = DS_NULL_PARAM;
    return sc;
  }
  ofstream file(fileName, ios::out | ios::binary);
  if (file.is_open()) {
    char hdr_buf = new char[DS_FILE_HDR_SIZE];
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

StatusCode DS_Manager::loadFile(const char *fileName,
                                DS_FileH &fileHandle)
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
    fileHandle.rm = rm;
    fileHandle.bm = bm;
    sc = DS_SUCCESS;
    return sc;
  }
  else {
    sc = rm->remoteLoadFile(fileName, fileHandle);
    return sc;
  }
}

StatusCode DS_RemoteManager::remoteLoadFile(const char *fileName,
                                            DS_FileH &fileHandle)
{

}