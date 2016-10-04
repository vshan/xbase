struct SysPage_FileHeader {
   int firstFree;
   int numPages;
};

class SysPage_FileHandle
{
  friend class SysPage_Manager;
public:
   SysPage_FileHandle();
   ~SysPage_FileHandle();

   ErrCode getFirstPage(SysPage_PageHandle &pageHandle);
   ErrCode getNextPage(int pageNum, SysPage_PageHandle &pageHandle);
   ErrCode getThisPage(int pageNum, SysPage_PageHandle &pageHandle);
   ErrCode getLastPage(SysPage_PageHandle &pageHandle);

   ErrCode allocatePage(SysPage_PageHandle &pageHandle);
   ErrCode disposePage(int pageNum);
   ErrCode markDirty(int pageNum);
   ErrCode unpinPage(int pageNum);
   ErrCode isValidPageNum(int pageNum);

private:
   SysPage_BufferManager* bufferMgr;
   SysPage_FileHeader hdr;
   int unixfd;
   int isFileOpen;
   int isHeaderChanged;

};
