/* Implementation file of SysPage, BufferManager component */
#include "SysPage_BufferManager.h"

using namespace std;

SysPage_BufferManager::SysPage_BufferManager(int numPages)
{
   this->numPages = numPages;
   bufTable = new SysPage_BufPageDesc[numPages];
   pageSize = PAGE_SIZE + PAGE_SIZE_HEADER;
   for (int i = 0; i < numPages; i++) {
      bufTable[i].pData = new char[pageSize];
      memset((void *)bufTable[i].pData, 0, pageSize);
      bufTable[i].prev = i-1;
      bufTable[i].next = i+1;
   }

   //Mark previous of first & next of last as INVALID
   bufTable[0].prev = bufTable[numPages - 1].next = INVALID_SLOT;
   free = 0;
   first = last = INVALID_SLOT;
}

SysPage_BufferManager::~SysPage_BufferManager()
{
    // Free up buffer pages and tables
   for (int i = 0; i < this->numPages; i++) {
      delete [] bufTable[i].pData;
   }
   delete [] bufTable;
}

ErrCode SysPage_BufferManager::getPage(int fd, int pageNum, char **ppBuffer)
{
   ErrCode ec;
   int slot;
   pair<int,int> p = make_pair(fd, pageNum);
   map<pair<int,int>,int>::iterator it;

    // If page exists in buffer
   it = slot_map.find(p);
   if (it != slot_map.end()) {
      slot = it->second;
      *ppBuffer = bufTable[slot].pData;
      bufTable[slot].pinCount++;
      unlink(slot);
      linkHead(slot);
   }

   // page does not exist in buffer
   else {
      if((ec = internalAlloc(slot)))
            return ec;

      if((ec = readPage(fd, pageNum, bufTable[slot].pData)))
            return ec;

      slot_map[p] = slot; // make a new entry in the hashMap
      initPageDesc(fd, pageNum, slot); // initialize page description
   }

   return (ec = 0);
}

ErrCode SysPage_BufferManager::allocatePage(int fd, int pageNum, char **ppBuffer)
{
   ErrCode ec;
   int slot;

   if((ec = internalAlloc(slot)))
        return ec;

   if((ec = readPage(fd, pageNum, bufTable[slot].pData)))
        return ec;

   slot_map[make_pair(fd, pageNum)] = slot; // make a new entry in the hashMap
   initPageDesc(fd, pageNum, slot); // initialize page description

   return (ec = 0);
}

ErrCode SysPage_BufferManager::markDirty(int fd, int pageNum)
{
   ErrCode ec;
   int slot;

   map<pair<int, int>, int>::iterator it;
   pair<int, int> p = make_pair(fd, pageNum);
   it = slot_map.find(p);
   if (it != slot_map.end()) {
      slot = it->second;
      bufTable[slot].isDirty = TRUE;

      unlink(slot);
      linkHead(slot);
   }

   return (ec = 0);
}

ErrCode SysPage_BufferManager::unpinPage(int fd, int pageNum)
{
   ErrCode ec;
   int slot;

   map<pair<int, int>, int>::iterator it;
   pair<int, int> p = make_pair(fd, pageNum);
   it = slot_map.find(p);

   if (it != slot_map.end()) {
      slot = it->second;

      if (bufTable[slot].pinCount > 0)
         bufTable[slot].pinCount--;

      if (bufTable[slot].pinCount == 0) {
         unlink(slot);
         linkHead(slot);
      }
   }

   return (ec = 0);
}

ErrCode SysPage_BufferManager::allocateBlock(char *&buffer)
{
   ErrCode ec;
   int slot;

   if((ec = internalAlloc(slot)))
    return ec;

   slot_map[make_pair(MEMORY_FD, MEMORY_PAGENUM)] = slot;
   initPageDesc(MEMORY_FD, MEMORY_PAGENUM, slot);
   buffer = bufTable[slot].pData;

   return (ec = 0);
}

ErrCode SysPage_BufferManager::disposeBlock(char *buffer)
{
   ErrCode ec;

   unpinPage(MEMORY_FD, MEMORY_PAGENUM);
   return (ec = 0);
}

/*
    Release all pages for this file and put them onto the free list
    Returns a warning if any of the file's pages are pinned.
    A linear search of the buffer is performed.
    A better method is not needed because # of buffers are small.
*/
ErrCode SysPage_BufferManager::flushPages(int fd)
{
   ErrCode ec;
   int slot;
   map<pair<int, int>, int>::iterator it;
   slot = first;
   while (slot != INVALID_SLOT) {
      int next = bufTable[slot].next;

      if (bufTable[slot].fd == fd) {

         if (bufTable[slot].pinCount == 0) {
            // todo
         }

         if (bufTable[slot].isDirty == TRUE) {
            if((ec = writePage(fd, bufTable[slot].pageNum,
                 bufTable[slot].pData)))
                    return ec;

            bufTable[slot].isDirty = FALSE;
         }

         it = slot_map.find(make_pair(fd, bufTable[slot].pageNum));
         if (it != slot_map.end())
            slot_map.erase(p);

         unlink(slot);
         insertFree(slot);
      }
      slot = next;
   }
   return (ec = 0);
}

ErrCode SysPage_BufferManager::forcePage(int fd, int pageNum)
{
   ErrCode ec;
   int slot;
   map<pair<int, int>, int>::iterator it;
   pair<int, int> p = make_pair(fd, pageNum);

   it = slot_map.find(p);

   if (it != slot_map.end()) {
      slot = it->second;

      if (bufTable[slot].isDirty == TRUE) {
         if((ec = writePage(fd, bufTable[slot].pageNum,
             bufTable[slot].pData)))
                return ec;
                
         bufTable[slot].isDirty = FALSE;
      }
   }

   return (ec = 0);
}

/*
    insert a slot at the head of the list, making it the most recently used slot
*/
ErrCode SysPage_BufferManager::linkHead(int slot)
{
   // Set next and prev pointers of slot entry
   bufTable[slot].next = first;
   bufTable[slot].prev = INVALID_SLOT;

   // If list isn't empty, point old first back to slot
   if (first != INVALID_SLOT)
      bufTable[first].prev = slot;

   first = slot; //make slot as the head now

   // if list was empty, set last to slot/first
   if (last == INVALID_SLOT)
      last = first;

   // Return ok
   return (0);
}

/*  unlink the slot from the used list.  Assume that slot is valid.
    Set prev and next pointers to INVALID_SLOT.
    The caller is responsible to either place the unlinked page into
    the free list or the used list.
*/
ErrCode SysPage_BufferManager::unlink(int slot)
{
   // If slot is at head of list, set first to next element
   if (first == slot)
      first = bufTable[slot].next;

   // If slot is at end of list, set last to previous element
   if (last == slot)
      last = bufTable[slot].prev;

   // If slot not at end of list, point next back to previous
   if (bufTable[slot].next != INVALID_SLOT)
      bufTable[bufTable[slot].next].prev = bufTable[slot].prev;

   // If slot not at head of list, point prev forward to next
   if (bufTable[slot].prev != INVALID_SLOT)
      bufTable[bufTable[slot].prev].next = bufTable[slot].next;

   // Set next and prev pointers of slot entry
   bufTable[slot].prev = bufTable[slot].next = INVALID_SLOT;

   // Return ok
   return (0);
}

/*
    Allocate a buffer slot. The slot is inserted at the
    head of the used list.  Here's how it chooses which slot to use:
    If there is something on the free list, then use it.
    Otherwise, choose a victim to replace.  If a victim cannot be
    chosen (because all the pages are pinned), then return an error.
Output : slot - set to newly-allocated slot
*/
ErrCode SysPage_BufferManager::internalAlloc(int &slot)
{
   ErrCode ec;
   map<pair<int, int>, int>::iterator it;

   // If the free list is not empty, choose a slot from the free list
   if (free != INVALID_SLOT) {
      slot = free;
      free = bufTable[slot].next;
   }
   else
   {
        // Choose the least-recently used page that is unpinned, that is why we start from last
        for (slot = last; slot != INVALID_SLOT; slot = bufTable[slot].prev)
        {
            if (bufTable[slot].pinCount == 0)
                break;
        }

        // return error if all pages are pinned
        if(slot == INVALID_SLOT)
            return SYSPAGE_NOBUF;

        // write page to disk if it is dirty
        if (bufTable[slot].isDirty == TRUE)
        {
            if((ec = writePage(bufTable[slot].fd, bufTable[slot].pageNum,
                bufTable[slot].pData)))
                    return ec;

            bufTable[slot].isDirty = FALSE;
        }

        pair<int, int> p = make_pair(bufTable[slot].fd, bufTable[slot].pageNum);

        it = slot_map.find(p);

        // remove page from the hashMap
        if (it != slot_map.end())
        {
            slot_map.erase(p);

            // remove page from the used buffer list
            unlink(slot);
        }
   }
   // link slot at the head of the used list
   linkHead(slot);

   return (ec = 0);
}

// write a page to disk
ErrCode SysPage_BufferManager::writePage(int fd, int pageNum, char *source)
{
    ErrCode ec;

    // seek to the appropriate place (cast to long for PC's)
    long offset = pageNum*pageSize + SYSPAGE_FILE_HDR_SIZE;

    if(lseek(fd, offset, L_SET) <0)
        return SYSPAGE_UNIX;

    // write the data
    int numBytes = (fd, source, pageSize);
    if(numBytes < 0)
        return SYSPAGE_UNIX;
    if(numBytes!=pageSize)
        return SYSPAGE_INCOMPLETEWRITE;

    return (ec = 0);
}

// read a page from disk
ErrCode SysPage_BufferManager::readPage(int fd, int pageNum, char *dest)
{
    ErrCode ec;
    long offset = pageNum*pageSize + SYSPAGE_FILE_HDR_SIZE;

    if(lseek(fd, offset, L_SET) < 0)
        return SYSPAGE_UNIX;

    // read the data
    int numBytes = read(fd, source, pageSize);
    if(numBytes < 0)
        return SYSPAGE_UNIX;
    if(numBytes!=pageSize)
        return SYSPAGE_INCOMPLETEREAD;

    return (ec = 0);
}

ErrCode SysPage_BufferManager::initPageDesc(int fd, int pageNum, int slot)
{
   bufTable[slot].fd       = fd;
   bufTable[slot].pageNum  = pageNum;
   bufTable[slot].bDirty   = FALSE;
   bufTable[slot].pinCount = 1;

   return 0;
}
