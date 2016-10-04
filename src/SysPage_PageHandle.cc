#include "SysPage_PageHandle.h"

using namespace std;

#define INVALID_PAGE   (-1)

SysPage_PageHandle::SysPage_PageHandle()
{
  pageNumber = INVALID_PAGE
  pageData = NULL
}

SysPage_PageHandle::~SysPage_PageHandle()
{

}

SysPage_PageHandle::SysPage_PageHandle(const SysPage_PageHandle &pageHandle)
{
  // Just copy the local variables since there is no local memory
  // allocation involved

  this->pageNumber = pageHandle.pageNumber
  this->pageData = pageHandle.pageData
}

SysPage_PageHandle& SysPage_PageHandle::operator = (const SysPage_PageHandle &pageHandle)
{
  // Check for self-assignment
  if (this != &pageHandle) {

    // Just copy the pointers since there is no local memory
    // allocation involved
    this->pageNumber = pageHandle.pageNumber;
    this->pageData = pageHandle.pageData;
  }

  // Return a reference to this
  return (*this);
}

ErrCode SysPage_PageHandle::getData(char *&pData)
{
  // Page must refer to a pinned page
  if(pageData == NULL)
    return SYSPAGE_PAGEUNPINNED;

  pData = pageData;

  return 0;
}

ErrCode SysPage_PageHandle::getPageNum(int &pageNum)
{
  // Page must refer to a pinned page
  if(pageData == NULL)
    return SYSPAGE_PAGEUNPINNED;

  pageNum = pageNumber

  return 0;
}
