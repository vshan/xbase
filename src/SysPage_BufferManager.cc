/* Implementation file of SysPage, BufferManager component */


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
   bufTable[0].prev = bufTable[numPages - 1].next = INVALID_SLOT;
   free = 0;
   first = last = INVALID_SLOT;
}

SysPage_BufferManager::~SysPage_BufferManager()
{
   delete [] bufTable;
   for (int i = 0; i < this->numPages; i++) {
      delete [] bufTable[i].pData;
   }
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

   else { // page does not exist in buffer
      internalAlloc(slot);
      readPage(fd, pageNum, bufTable[slot].pData);
      slot_map[p] = slot;
      initPageDesc(fd, pageNum, slot);
   }

   return (ec = 0);
}

ErrCode SysPage_BufferManager::allocatePage(int fd, int pageNum, char **ppBuffer)
{
   ErrCode ec;
   int slot;

   internalAlloc(slot);
   readPage(fd, pageNum, bufTable[slot].pData);
   slot_map[make_pair(fd, pageNum)] = slot;
   initPageDesc(fd, pageNum, slot);

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
      headLink(slot);
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

   internalAlloc(slot);
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
            writePage(fd, bufTable[slot].pageNum, bufTable[slot].pData);
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
         writePage(fd, bufTable[slot].pageNum, bufTable[slot].pData);
         bufTable[slot].isDirty = FALSE;
      }
   }

   return (ec = 0);
 }
