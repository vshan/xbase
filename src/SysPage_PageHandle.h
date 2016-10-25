#include "SysPage_Internal.h"

class SysPage_PageHandle
{
  friend class SysPage_FileHandle;
public:
   SysPage_PageHandle();
   ~SysPage_PageHandle();

   ErrCode getData(char* &pData);
   ErrCode getPageNum(int &pageNum);
private:
   int pageNumber;
   char *pageData;
};
