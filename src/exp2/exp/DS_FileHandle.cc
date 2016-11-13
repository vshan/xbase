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

DS_FileHandle::DS_FileHandle()
{
  bm = NULL;
  rm = NULL;
}

DS_FileHandle::~DS_FileHandle()
{

}

StatusCode DS_FileHandle::getFirstPage(DS_PageHandle &pageHandle)
{
  return (getNextPage(-1, pageHandle));
}

StatusCode DS_FileHandle::getLastPage(DS_PageHandle &pageHandle)
{
  return (getPrevPage(hdr.numPages, pageHandle));
}

StatusCode DS_FileHandle::getNextPage(int pageNum, DS_PageHandle &pageHandle)
{
  StatusCode sc;
  for (pageNum++; pageNum < hdr.numPages; pageNum++)
  {
    if (!(sc = getThisPage(pageNum, pageHandle)))
      return (0);
    
    if (sc != DS_INVALIDPAGE)
      return (sc);
  }
  return DS_EOF;
}

StatusCode DS_FileHandle::allocatePage(DS_PageHandle &pageHandle)
{
  StatusCode sc;
  int pageNum;
  char *pPageBuf;

  if (hdr.firstFree != DS_PAGE_LIST_END) {
    pageNum = hdr.firstFree;

    if (isRemote) {
      bm->getPage((char *)ipaddr.c_str(), (char *)port.c_str(),
        (char *)fileName.c_str(), pageNum, &pPageBuf);
    }
    else {
      bm->getPage(unixfd, pageNum, &pPageBuf);
    }

    hdr.firstFree = pageNum+1;

  }
  else {
    // free list is empty
    pageNum = hdr.numPages;

    if (isRemote) {
      bm->allocatePage((char *)ipaddr.c_str(), (char *)port.c_str(),
           (char *)fileName.c_str(), pageNum, &pPageBuf);
    }
    else {
      bm->allocatePage(unixfd, pageNum, &pPageBuf);
    }

    hdr.numPages++;
  }

  isHdrChanged = true;
  memset(pPageBuf, 0, DS_PAGE_SIZE);

  markDirty(pageNum);

  pageHandle.pageNumber = pageNum;
  pageHandle.pageData = pPageBuf;

  return 0;
}

StatusCode DS_FileHandle::getPrevPage(int pageNum, DS_PageHandle &pageHandle)
{
  StatusCode sc;
  for (pageNum--; pageNum >= 0; pageNum--)
  {
    if (!(sc = getThisPage(pageNum, pageHandle)))
      return (0);
    
    if (sc != DS_INVALIDPAGE)
      return (sc);
  }
  return DS_EOF;
}

StatusCode DS_FileHandle::getThisPage(int pageNum, DS_PageHandle &pageHandle)
{
  char *msg_con = new char[DS_BUF_SIZE];
  if (isRemote) {
    bm->getPage((char *)ipaddr.c_str(), (char *)port.c_str(), (char *)fileName.c_str(), pageNum, &msg_con);
  }
  else {
    bm->getPage(unixfd, pageNum, &msg_con);
  }

  pageHandle.pageNumber = pageNum;
  pageHandle.pageData = msg_con;

  return DS_SUCCESS;
}

StatusCode DS_FileHandle::markDirty(int pageNum)
{
  if (isRemote)
    bm->markDirty((char *)ipaddr.c_str(), (char *)port.c_str(), (char *)fileName.c_str(), pageNum);
  else
    bm->markDirty(unixfd, pageNum);

  return DS_SUCCESS;
}

StatusCode DS_FileHandle::unpinPage(int pageNum)
{
  if (isRemote)
    bm->unpinPage((char *)ipaddr.c_str(), (char *)port.c_str(), (char *)fileName.c_str(), pageNum);
  else
    bm->unpinPage(unixfd, pageNum);

  return DS_SUCCESS;
}