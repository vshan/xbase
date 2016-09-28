#include "SysPage_FileHandle.h"
#include "SysPage_Internal.h"
#include "iostream"


SysPage_FileHandle::SysPage_FileHandle()
{
  //Set local variables
  isFileOpen = FALSE;
  bfrmgr = NULL;
}
~SysPage_FileHandle()
{

}

ErrCode SysPage_PageHandle::getFirstPage(SysPage_PageHandle &pageHandle)
{
  // Get next page from the 0th page
  return (getNextPage((int)-1),pageHandle);
}

ErrCode SysPage_PageHandle::getLastPage(SysPage_PageHandle &pageHandle)
{
  //get previous page from last + 1 page
  return (getPreviousPage((int)hdr.numPages,pageHandle))
}

ErrCode SysPage_PageHandle::getNextPage(int pageNum, SysPage_PageHandle &pageHandle)
{
  int ec    // error code

  // Check if file open
  if(!isFileOpen)
    return (SYSPAGE_CLOSEDFILE);

  // Is page valid?
  if(pageNum != -1 && (!IsValidPageNum(pageNum)))
    return (SYSPAGE_INVALIDPAGE);

  // iterate till you get a valid page
  for (pageNum++; pageNum < hdr.numPages; pageNum++)
  {
    if(!(ec = getThisPage(pageNum,pageHandle)))
      return (0)

    if(ec != SYSPAGE_INVALIDPAGE)
      return (ec)
  }
  // return EOF if othing is found
  return SYSPAGE_EOF
}

ErrCode SysPage_PageHandle::getPreviousPage(int pageNum, SysPage_PageHandle &pageHandle)
{
  int ec

  if(!isFileOpen)
    return (SYSPAGE_CLOSEDFILE);

  if(pageNum != -1 && (!IsValidPageNum(pageNum)))
    return (SYSPAGE_INVALIDPAGE);

  for (pageNum--; pageNum >= 0; pageNum--)
  {
    if(!(ec = getThisPage(pageNum,pageHandle)))
      return (0)

    if(ec != SYSPAGE_INVALIDPAGE)
      return (ec)
  }

  return SYSPAGE_EOF
}

ErrCode SysPage_PageHandle::getThisPage(int pageNum, SysPage_PageHandle &pageHandle)
{
  int  ec;               // return code
  char *pageBuf;        // address of page in buffer pool

  if (!isFileOpen)
     return (SYSPAGE_CLOSEDFILE);

  if (!isValidPageNum(pageNum))
     return (SYSPAGE_INVALIDPAGE);

  // Get this page from the buffer manager
  if ((ec = bufferMgr->getPage(unixfd, pageNum, &pageBuf)))
     return (ec);

  // If the page is valid, then set pageHandle to this page and return ok
  if (((SysPage_PageHdr*)pageBuf)->nextFree == SYSPAGE_PAGE_USED) {

     // Set the pageHandle local variables
     pageHandle.pageNumber = pageNum;
     pageHandle.pageData = pageBuf + sizeof(SysPage_PageHdr);

     return (0);
  }

  // If the page is not a valid one, then unpin the page
  if ((ec = unpinPage(pageNum)))
     return (ec);

  return (SYSPAGE_INVALIDPAGE);
}
