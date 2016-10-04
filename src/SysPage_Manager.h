#include "SysPage_Internal.h"

class SysPage_Manager
{
public:
   SysPage_Manager();
   ~SysPage_Manager();

   ErrCode createFile(const char *fileName);
   ErrCode openFile(const char *fileName, SysPage_FileHandle &fileHandle);
   ErrCode closeFile(SysPage_FileHandle &fileHandle);
   ErrCode destroyFile(const char *fileName);

private:
   SysPage_BufferManager* bfrmgr;
};
