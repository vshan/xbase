#ifndef RECORD_INTERNAL_H
#define RECORD_INTERNAL_H

#include "xbase.h"
#include "SysPage_Internal.h"

struct Record_FileHdr {
  int firstFree;
  int numPages;
  int extRecordSize;
};

struct Record_PageHdr {
  int nextFree;       // nextFree can be any of these values:
                      //  - the number of the next free page
                      //  - RECORD_PAGE_LIST_END if this is last free page
                      //  - RECORD_PAGE_FULLY_USED if the page is not free
  char * freeSlotMap; // A bitmap that tracks the free slots within the page
  int numSlots;
  int numFreeSlots;

  Record_PageHdr(int numSlots) : numSlots(numSlots), numFreeSlots(numSlots)
      { freeSlotMap = new char[this->mapsize()];}

  ~Record_PageHdr()
      { delete [] freeSlotMap; }

  int size() const
      { return sizeof(nextFree) + sizeof(numSlots) + sizeof(numFreeSlots)
          + bitmap(numSlots).numChars()*sizeof(char); }
  int mapsize() const
      { return this->size() - sizeof(nextFree)
          - sizeof(numSlots) - sizeof(numFreeSlots);}
  int to_buf(char *& buf) const
      {
        memcpy(buf, &nextFree, sizeof(nextFree));
        memcpy(buf + sizeof(nextFree), &numSlots, sizeof(numSlots));
        memcpy(buf + sizeof(nextFree) + sizeof(numSlots),
               &numFreeSlots, sizeof(numFreeSlots));
        memcpy(buf + sizeof(nextFree) + sizeof(numSlots) + sizeof(numFreeSlots),
               freeSlotMap, this->mapsize()*sizeof(char));
        return 0;
      }
  int from_buf(const char * buf)
      {
        memcpy(&nextFree, buf, sizeof(nextFree));
        memcpy(&numSlots, buf + sizeof(nextFree), sizeof(numSlots));
        memcpy(&numFreeSlots, buf + sizeof(nextFree) + sizeof(numSlots),
               sizeof(numFreeSlots));
        memcpy(freeSlotMap,
               buf + sizeof(nextFree) + sizeof(numSlots) + sizeof(numFreeSlots),
               this->mapsize()*sizeof(char));
        return 0;
      }
};

#define RECORD_PAGE_LIST_END   -1       // end of list of free pages
#define RECORD_PAGE_FULLY_USED -2       // page is fully used with no free slots

#define RM_BADRECSIZE      (START_RM_WARN + 0)  // rec size invalid <= 0
#define RM_NORECATRID      (START_RM_WARN + 1)  // This rid has no record

#define RM_LASTWARN RM_NORECATRID

#define RM_SIZETOOBIG      (START_RM_ERR - 0)  // record size too big
#define RM_PF              (START_RM_ERR - 1)  // error in PF
#define RM_NULLRECORD      (START_RM_ERR - 2)
#define RM_RECSIZEMISMATCH (START_RM_ERR - 3)  // record size mismatch
#define RM_HANDLEOPEN      (START_RM_ERR - 4)
#define RM_FCREATEFAIL     (START_RM_ERR - 5)
#define RM_FNOTOPEN        (START_RM_ERR - 6)
#define RM_BAD_RID         (START_RM_ERR - 7)
#define RM_EOF             (START_RM_ERR - 8)  // end of file

#define RM_LASTERROR RM_EOF

#endif
