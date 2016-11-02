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


StatusCode getFirstPage(DS_PageHandle &pageHandle)
{
  return (getNextPage(-1, pageHandle));
}

StatusCode getLastPage(DS_PageHandle &pageHandle)
{
  return (getPrevPage(hdr.numPages, pageHandle));
}

StatusCode getNextPage(int pageNum, DS_PageHandle &pageHandle)
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

StatusCode getPrevPage(int pageNum, DS_PageHandle &pageHandle)
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