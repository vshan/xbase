#include <iostream>
#include <fstream>
#include <stdio.h>
#include <cstdlib>
#include <thread>
#include <utility>
#include <stdlib.h>
#include <fcntl.h>   /* For O_RDWR */
#include <unistd.h>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>                                                                                                                                                
#include <boost/asio.hpp>
#include <string>
#include "DS.h"

using namespace std;


DS_BufferManager::DS_BufferManager(int numPages, DS_RemoteManager *rem_mgr)
{
  this->numPages = numPages;
  pageSize = DS_PAGE_SIZE;
  bufTable = new DS_BufPageDesc[numPages];
  for (int i = 0; i < numPages; i++) {
    bufTable[i].pData = new char[pageSize];
    bufTable[i].ipAddr = new char[DS_SMALL_BUF_SIZE];
    bufTable[i].port = new char[DS_SMALL_BUF_SIZE];
    bufTable[i].fileName = new char[DS_SMALL_BUF_SIZE];
    memset((void *)bufTable[i].pData, 0, pageSize);
    bufTable[i].prev = i-1;
    bufTable[i].next = i+1;
  }
  rm = rem_mgr;
  bufTable[0].prev = bufTable[numPages - 1].next = DS_INVALID_SLOT;
  free = 0;
  first = last = DS_INVALID_SLOT;
}

DS_BufferManager::~DS_BufferManager()
{
  // Free up buffer pages and tables
  for (int i = 0; i < this->numPages; i++) {
    delete [] bufTable[i].pData;
  }
  delete [] bufTable;
}

StatusCode DS_BufferManager::getPage(int fd, int pageNum, char **ppBuffer)
{
  StatusCode sc;
  int slot;
  map<pair<int, int>, int>::iterator it;
  pair<int, int> p = make_pair(fd, pageNum);

  it = loc_slot_map.find(p);
  if (it != loc_slot_map.end()) {
    slot = it->second;
    bufTable[slot].pinCount++;
    unlink(slot);
    linkHead(slot);
  }

   // page does not exist in buffer
  else {
    if((sc = internalAlloc(slot)))
        return sc;

    if((sc = readPage(fd, pageNum, bufTable[slot].pData)))
        return sc;

    loc_slot_map.insert(make_pair(p, slot)); // make a new entry in the hashMap
    initPageDesc(fd, pageNum, slot); // initialize page description
  }

  *ppBuffer = bufTable[slot].pData;
  return DS_SUCCESS;
}

StatusCode DS_BufferManager::getPage(char *ipAddr, char *port, char *fileName, int pageNum, char **ppBuffer)
{
  StatusCode sc;
  int slot;
  map<pair<pair<string, string>, pair<string,int> >, int>::iterator it;
  pair<pair<string, string>, pair<string, int> > rp = make_pair(make_pair(string(ipAddr), string(port)), 
                                                               make_pair(string(fileName), pageNum));
  it = rem_slot_map.find(rp);

  if (it != rem_slot_map.end()) {
    slot = it->second;
    bufTable[slot].pinCount++;
    unlink(slot);
    linkHead(slot);
  }
  // page is in network
  else {
    internalAlloc(slot);
    readPage(ipAddr, port, fileName, pageNum, bufTable[slot].pData);
    rem_slot_map.insert(make_pair(rp, slot));
    initPageDesc(ipAddr, port, fileName, pageNum, slot);
  }

  *ppBuffer = bufTable[slot].pData;
  return DS_SUCCESS;
}

StatusCode DS_BufferManager::allocatePage(int fd, int pageNum, char **ppBuffer)
{
  StatusCode sc;
  int slot;

  if((sc = internalAlloc(slot)))
    return sc;

  if((sc = readPage(fd, pageNum, bufTable[slot].pData)))
    return sc;

  loc_slot_map[make_pair(fd, pageNum)] = slot; // make a new entry in the hashMap
  initPageDesc(fd, pageNum, slot); // initialize page description
  *ppBuffer = bufTable[slot].pData;
  return (sc = 0);
}

StatusCode DS_BufferManager::allocatePage(char *ipAddr, char *port, char *fileName, int pageNum, char **ppBuffer)
{
  //rm->remoteAllocatePage(ipAddr, port, fileName, pageNum);
  int slot;
  internalAlloc(slot);
  readPage(ipAddr, port, fileName, pageNum, bufTable[slot].pData); 
}

StatusCode DS_BufferManager::markDirty(int fd, int pageNum)
{
  StatusCode sc;
  int slot;

  map<pair<int, int>, int>::iterator it;
  pair<int, int> p = make_pair(fd, pageNum);
  it = loc_slot_map.find(p);
  if (it != loc_slot_map.end()) {
    slot = it->second;
    bufTable[slot].isDirty = true;
    unlink(slot);
    linkHead(slot);
  }
  return (sc = 0);
}

StatusCode DS_BufferManager::markDirty(char *ipAddr, char *port, char *fileName, int pageNum)
{
  StatusCode sc;
  int slot;

  // pair<string, string> p = make_pair(string(ipAddr), string(port));
  map<pair<pair<string, string>, pair<string,int> >, int>::iterator it;
  pair<pair<string, string>, pair<string, int> > rp = make_pair(make_pair(string(ipAddr), string(port)), 
                                                 make_pair(string(fileName), pageNum));
  it = rem_slot_map.find(rp);
  if (it != rem_slot_map.end()) {
    slot = it->second;
    bufTable[slot].isDirty = true;
    unlink(slot);
    linkHead(slot);
  }
  return (sc = 0);
}

StatusCode DS_BufferManager::unpinPage(int fd, int pageNum)
{
  StatusCode sc;
  int slot;

  map<pair<int, int>, int>::iterator it;
  pair<int, int> p = make_pair(fd, pageNum);
  it = loc_slot_map.find(p);

  if (it != loc_slot_map.end()) {
    slot = it->second;

    if (bufTable[slot].pinCount > 0)
      bufTable[slot].pinCount--;

    if (bufTable[slot].pinCount == 0) {
      unlink(slot);
      linkHead(slot);
    }
  }

  return (sc = 0);
}

StatusCode DS_BufferManager::unpinPage(char *ipAddr, char *port, char *fileName, int pageNum)
{
  StatusCode sc;
  int slot;

  map<pair<pair<string, string>, pair<string,int> >, int>::iterator it;
  pair<pair<string, string>, pair<string, int> > rp = make_pair(make_pair(string(ipAddr), string(port)), 
                                                 make_pair(string(fileName), pageNum));
  it = rem_slot_map.find(rp);

  if (it != rem_slot_map.end()) {
    slot = it->second;

    if (bufTable[slot].pinCount > 0)
      bufTable[slot].pinCount--;

    if (bufTable[slot].pinCount == 0) {
      unlink(slot);
      linkHead(slot);
    }
  }

  return (sc = 0);
}

StatusCode DS_BufferManager::forcePage(int fd, int pageNum)
{
  StatusCode sc;
  int slot;
  map<pair<int, int>, int>::iterator it;
  pair<int, int> p = make_pair(fd, pageNum);

  it = loc_slot_map.find(p);

  if (it != loc_slot_map.end()) {
    slot = it->second;

    if (bufTable[slot].isDirty == true) {
      if((sc = writePage(fd, bufTable[slot].pageNum,
        bufTable[slot].pData)))
          return sc;

        bufTable[slot].isDirty = false;
    }
  }

  return (sc = 0);
}

StatusCode DS_BufferManager::forcePage(char *ipAddr, char *port, char *fileName, int pageNum)
{
  StatusCode sc;
  int slot;
  map<pair<pair<string, string>, pair<string,int> >, int>::iterator it;
  pair<pair<string, string>, pair<string, int> > rp = make_pair(make_pair(string(ipAddr), string(port)), 
                                                             make_pair(string(fileName), pageNum));
  
  it = rem_slot_map.find(rp);

  if (it != rem_slot_map.end()) {
    slot = it->second;

    if (bufTable[slot].isDirty == true) {
      if((sc = writePage(ipAddr, port, fileName, pageNum,
        bufTable[slot].pData)))
          return sc;

        bufTable[slot].isDirty = false;
    }
  }

  return (sc = 0);
}

// Insert a slot at the head of the free list
StatusCode DS_BufferManager::insertAtHead(int slot)
{
  bufTable[slot].next = free;
  free = slot;

  return (0);
}

StatusCode DS_BufferManager::linkHead(int slot)
{
  // Set next and prev pointers of slot entry
  bufTable[slot].next = first;
  bufTable[slot].prev = DS_INVALID_SLOT;

   // If list isn't empty, point old first back to slot
  if (first != DS_INVALID_SLOT)
    bufTable[first].prev = slot;

  first = slot; //make slot as the head now

   // if list was empty, set last to slot/first
  if (last == DS_INVALID_SLOT)
    last = first;

   // Return ok
  return (0);
}

/*  unlink the slot from the used list.  Assume that slot is valid.
    Set prev and next pointers to INVALID_SLOT.
    The caller is responsible to either place the unlinked page into
    the free list or the used list.
*/
StatusCode DS_BufferManager::unlink(int slot)
{
   // If slot is at head of list, set first to next element
  if (first == slot)
    first = bufTable[slot].next;

   // If slot is at end of list, set last to previous element
  if (last == slot)
    last = bufTable[slot].prev;

   // If slot not at end of list, point next back to previous
  if (bufTable[slot].next != DS_INVALID_SLOT)
    bufTable[bufTable[slot].next].prev = bufTable[slot].prev;

   // If slot not at head of list, point prev forward to next
  if (bufTable[slot].prev != DS_INVALID_SLOT)
    bufTable[bufTable[slot].prev].next = bufTable[slot].next;

   // Set next and prev pointers of slot entry
  bufTable[slot].prev = bufTable[slot].next = DS_INVALID_SLOT;

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
StatusCode DS_BufferManager::internalAlloc(int &slot)
{
  StatusCode sc;
  
  map<pair<int, int>, int>::iterator it;
  pair<int, int> p;
  map<pair<pair<string, string>, pair<string,int> >, int>::iterator rit;
  pair<pair<string, string>, pair<string, int> > rp;
  
  // map<pair<pair<string, string>, int>, int>::iterator rit;
  // pair<pair<string, string>, int> rp;

   // If the free list is not empty, choose a slot from the free list
  if (free != DS_INVALID_SLOT) {
    slot = free;
    free = bufTable[slot].next;
  }
  else
  {
    // Choose the least-recently used page that is unpinned, that is why we start from last
    for (slot = last; slot != DS_INVALID_SLOT; slot = bufTable[slot].prev)
    {
      if (bufTable[slot].pinCount == 0)
        break;
    }
    // return error if all pages are pinned
    if(slot == DS_INVALID_SLOT)
      return DS_NOBUF;

        // write page to disk if it is dirty
    if (bufTable[slot].isDirty == true) {
      if (bufTable[slot].isRemote)
        writePage(bufTable[slot].ipAddr, bufTable[slot].port,
          bufTable[slot].fileName, bufTable[slot].pageNum, bufTable[slot].pData);
      else
        writePage(bufTable[slot].fd, bufTable[slot].pageNum,
            bufTable[slot].pData);

      bufTable[slot].isDirty = false;
    }

    if (bufTable[slot].isRemote) {
      rp = make_pair(make_pair(string(bufTable[slot].ipAddr), string(bufTable[slot].port)), 
                     make_pair(string(bufTable[slot].fileName), bufTable[slot].pageNum));
      rit = rem_slot_map.find(rp);

      if (rit != rem_slot_map.end()) {
        rem_slot_map.erase(rp);
        unlink(slot);
      }
    }

    else {
      p = make_pair(bufTable[slot].fd, bufTable[slot].pageNum);
      it = loc_slot_map.find(p);

      if (it != loc_slot_map.end()) {
        loc_slot_map.erase(p);
        unlink(slot);
      }
    }
  }
   // link slot at the head of the used list
  linkHead(slot);

  return (sc = 0);
}

StatusCode DS_BufferManager::writePage(int fd, int pageNum, char *source)
{
  StatusCode sc;

  sc = rm->setLocalPage(fd, pageNum, source);

  return (sc);
}

StatusCode DS_BufferManager::writePage(char *ipAddr, char *port, char *fileName, int pageNum, char *source)
{
  StatusCode sc;

  sc = rm->setRemotePage(ipAddr, port, fileName, pageNum, source);

  return (sc);
}


// read a page from disk
StatusCode DS_BufferManager::readPage(int fd, int pageNum, char *dest)
{
  StatusCode sc;
  sc = rm->getLocalPage(fd, pageNum, dest);
  return (sc = 0);
}

StatusCode DS_BufferManager::readPage(char *ipAddr, char *port, char *fileName, int pageNum, char *dest)
{
  StatusCode sc;
  sc = rm->getRemotePage(ipAddr, port, fileName, pageNum, dest);
  return sc;
}

StatusCode DS_BufferManager::initPageDesc(int fd, int pageNum, int slot)
{
  bufTable[slot].fd       = fd;
  bufTable[slot].pageNum  = pageNum;
  bufTable[slot].isDirty   = false;
  bufTable[slot].pinCount = 1;
  bufTable[slot].isRemote = false;
  return 0;
}

StatusCode DS_BufferManager::initPageDesc(char *ipAddr, char *port, char *fileName, int pageNum, int slot)
{
  strcpy(bufTable[slot].ipAddr, ipAddr);
  strcpy(bufTable[slot].port, port);
  strcpy(bufTable[slot].fileName, fileName);
  
  bufTable[slot].pageNum  = pageNum;
  bufTable[slot].isDirty   = false;
  bufTable[slot].pinCount = 1;
  bufTable[slot].isRemote = true;
  return 0;
}