#include "DS.h"

DS_PageHandle::DS_PageHandle()
{
  pageNumber = INVALID_PAGE;
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