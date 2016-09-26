class SysPage_PageHandle
{
public:
   SysPage_PageHandle();
   ~SysPage_PageHandle();

   ErrCode getData(char *&pData);
   ErrCode getPageNum(int &pageNum);
private:
   int pageNumber;
   char *pageData;
};