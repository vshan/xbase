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

DS_PageHandle::DS_PageHandle()
{
  pageNumber = DS_INVALID_PAGE;
  pageData = NULL;
}

DS_PageHandle::~DS_PageHandle()
{

}

DS_PageHandle::DS_PageHandle(DS_PageHandle &pageHandle)
{
  this->pageNumber = pageHandle.pageNumber;
  this->pageData = pageHandle.pageData;
}

StatusCode DS_PageHandle::getData(char* &pData)
{
  if (pageData == NULL)
    return DS_UNPINNED_PAGE;

  pData = pageData;

  return DS_SUCCESS;
}

StatusCode DS_PageHandle::getPageNum(int &pageNum)
{
  if (pageData == NULL)
    return DS_UNPINNED_PAGE;

  pageNum = pageNumber;

  return DS_SUCCESS;
}