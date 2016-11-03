class DS_FileHandle {
public:
  DS_FileHandle();
  ~DS_FileHandle();

  StatusCode getFirstPage(DS_PageHandle &pageHandle);
  StatusCode getNextPage(int pageNum, DS_PageHandle &pageHandle);
  StatusCode getThisPage(int pageNum, DS_PageHandle &pageHandle);
  StatusCode getLastPage(DS_PageHandle &pageHandle);
  StatusCode allocatePage(DS_PageHandle &pageHandle);
  StatusCode markDirty(int pageNum);
  StatusCode unpinPage(int pageNum);
private:
  DS_BufferManager *bm;
  DS_RemoteManager *rm;
  DS_FileHeader hdr;
  int unixfd;
  bool isRemote;
  string fileName;
  string ipaddr;
  string port;
};

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
      bm->getPage(ipaddr.c_str(), port.c_str(),
        fileName.c_str(), pageNum, &pPageBuf);
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
      bm->allocatePage(ipaddr.c_str(), port.c_str(),
           fileName.c_str(), pageNum, &pPageBuf);
    }
    else {
      bm->allocatePage(unixfd, pageNum, &pPageBuf);
    }

    hdr.numPages++;
  }

  isHdrChanged = TRUE;
  memset(pPageBuf, 0, DS_PAGE_SIZE);

  markDirty(pageNum);

  pageHandle.pageNum = pageNum;
  pageHandle.pPageData = pPageBuf;

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

StatusCode getThisPage(int pageNum, DS_PageHandle &pageHandle)
{
  char msg_con[DS_BUF_SIZE];
  if (isRemote) {
    bm->getPage(ipaddr.c_str(), port.c_str(), fileName.c_str(), pageNum, msg_con);
  }
  else {
    bm->getPage(unixfd, pageNum, msg_con);
  }

  pageHandle.pageNum = pageNum;
  pageHandle.pPageData = msg_con;

  return DS_SUCCESS;
}

StatusCode markDirty(int pageNum)
{
  if (isRemote)
    bm->markDirty(ipaddr.c_str(), port.c_str(), fileName.c_str(), pageNum);
  else
    bm->markDirty(unixfd, pageNum);

  return DS_SUCCESS;
}

StatusCode unpinPage(int pageNum)
{
  if (isRemote)
    bm->unpinPage(ipaddr.c_str(), port.c_str(), fileName.c_str(), pageNum);
  else
    bm->unpinPage(unixfd, pageNum);

  return DS_SUCCESS;
}