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
};

#define RECORD_PAGE_LIST_END   -1       // end of list of free pages
#define RECORD_PAGE_FULLY_USED -2       // page is fully used with no free slots
