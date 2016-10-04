#include "SysPage_Internal.h"
#include "SysPage_PageHandle.h"

#define INVALID_PAGE   (-1)

//
// SysPage_PageHandle
//
// Desc: Default constructor for a page handle object
//       A page handle object provides access to the contents of a page
//       and the page's page number.  The page handle object is constructed
//       here but it must be passed to one of the SysPage_FileHandle methods to
//       have it refer to a pinned page before it can be used to access the
//       contents of a page.  Remember to call SysPage_FileHandle::UnpinPage()
//       to unpin the page when you are finished accessing it.
//
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
