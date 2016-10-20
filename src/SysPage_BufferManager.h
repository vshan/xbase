#include "SysPage_Internal.h"

#define MEMORY_FD -1
#define MEMORY_PAGENUM 1

// L_SET is used to indicate the "whence" argument of the lseek call
// defined in "/usr/include/unistd.h".  A value of 0 indicates to
// move to the absolute location specified.
#ifndef L_SET
#define L_SET  0
#endif

struct SysPage_BufPageDesc {
   char *pData;
   int next;
   int prev;
   int isDirty;
   int pinCount;
   int pageNum;
   int fd;
};

class SysPage_BufferManager
{
public:
   SysPage_BufferManager(int numPages);
   ~SysPage_BufferManager();

   ErrCode getPage(int fd, int pageNum, char **ppBuffer);
   ErrCode allocatePage(int fd, int pageNum, char **ppBuffer);
   ErrCode markDirty(int fd, int pageNum);
   ErrCode unpinPage(int fd, int pageNum);
   ErrCode allocateBlock(char *&buffer);
   ErrCode disposeBlock(char *buffer);
   ErrCode flushPages(int fd);
   // Force a page, but do not remove from buffer
   ErrCode forcePage(int fd, int pageNum);
   ErrCode clearBuffer();

private:
   ErrCode linkHead(int slot);
   ErrCode unlink(int slot);
   ErrCode internalAlloc(int &slot);
   ErrCode writePage(int fd, int pageNum, char *source);
   ErrCode readPage(int fd, int pageNum, char *dest);
   ErrCode initPageDesc(int fd, int pageNum, int slot);
   map<pair<int, int>, int> slot_map; // map<(file_descriptor, pageNumber), slot_number_in_buffer)>
   SysPage_BufPageDesc *bufTable;
   int numPages;
   int pageSize;
   int first;
   int last;
   int free;
};
