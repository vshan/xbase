#include "SysPage_FileHandle.h"

using namespace std;

SysPage_FileHandle::SysPage_FileHandle()
{
  //Set local variables
  isFileOpen = FALSE;
  bufferMgr = NULL;
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
  if(pageNum != -1 && (!isValidPageNum(pageNum)))
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

  if(pageNum != -1 && (!isValidPageNum(pageNum)))
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

ErrCode SysPage_PageHandle::allocatePage(SysPage_PageHandle &pageHandle)
{
  int     ec,pageNum;
  char    *pageBuf;

  if (!isFileOpen)
     return (SYSPAGE_CLOSEDFILE);

  // check if the free list is empty
  if (hdr.firstFree != SYSPAGE_PAGE_LIST_END)
  {
      pageNum = hdr.firstFree;

      // Get the first free page into the buffer
      if ((ec = bufferMgr->getPage(unixfd, pageNum, &pPageBuf)))
        return (ec);

      // Set the first free page to the next page on the free list
      hdr.firstFree = ((SysPage_PageHdr*)pageBuf)->nextFree;
  }

  else
  {
     pageNum = hdr.numPages;

     // Allocate a new page in the file
     if ((ec = bufferMgr->allocatePage(unixfd,
           pageNum,
           &pPageBuf)))
        return (ec);
     hdr.numPages++;
  }

  // Mark the header as changed
  isHdrChanged = TRUE;

  // Mark this page as used
  ((SysPage_PageHdr *)pageBuf)->nextFree = SYSPAGE_PAGE_USED;

  // Zero out the page data
  memset(pageBuf + sizeof(SysPage_PageHdr), 0, SYSPAGE_PAGE_SIZE);

  // Mark the page dirty because we changed the next pointer
  if ((ec = markDirty(pageNum)))
     return (ec);

  // Set the pageHandle local variables
  pageHandle.pageNum = pageNum;
  pageHandle.pageData = pageBuf + sizeof(SYSPAGE_PageHdr);

  return (0);
}

ErrCode SysPage_PageHandle::disposePage(int pageNum)
{
  int     ec;
  char    *pageBuf;

  if (!isFileOpen)
     return (SYSPAGE_CLOSEDFILE);

  if(!isValidPageNum(pageNum))
     return (SYSPAGE_INVALIDPAGE);

  // Get the page (but don't re-pin it if it's already pinned)
  if ((ec = bufferMgr->getPage(unixfd,
        pageNum,
        &pageBuf,
        FALSE)))
     return (ec);

  // Page must be valid (used)
  if (((SysPage_PageHdr *)pageBuf)->nextFree != SYSPAGE_PAGE_USED) {

     // Unpin the page
     if ((ec = unpinPage(pageNum)))
        return (ec);

     // Return page already free
     return (SYSPAGE_PAGEFREE);
  }

  // Put this page onto the free list
  ((SysPage_PageHdr *)pageBuf)->nextFree = hdr.firstFree;
  hdr.firstFree = pageNum;
  isHdrChanged = TRUE;

  // Mark the page dirty because we changed the next pointer
  if ((ec = markDirty(pageNum)))
     return (ec);

  // Unpin the page
  if ((ec = unpinPage(pageNum)))
     return (ec);

  return (0);
}

ErrCode SysPage_PageHandle::markDirty(int pageNum)
{
  if (!isFileOpen)
     return (SYSPAGE_CLOSEDFILE);

  if (!isValidPageNum(pageNum))
     return (SYSPAGE_INVALIDPAGE);

  // Tell the buffer manager to mark the page dirty
  return (bufferMgr->markDirty(unixfd, pageNum));
}

ErrCode SysPage_PageHandle::unpinPage(int pageNum)
{
  if (!isFileOpen)
     return (SYSPAGE_CLOSEDFILE);

  if (!isValidPageNum(pageNum))
     return (SYSPAGE_INVALIDPAGE);

  // Tell the buffer manager to mark the page dirty
  return (bufferMgr->unpinPage(unixfd, pageNum));
}

ErrCode SysPage_PageHandle::isValidPageNum(int pageNum)
{
  return (isFileOpen &&
        pageNum >= 0 &&
        pageNum < hdr.numPages);
}
