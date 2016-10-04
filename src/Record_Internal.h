#include "xbase.h"
#include "SysPage_Internal.h"

struct Record_FileHdr {
  int firstFree;
  int numPages;
  int extRecordSize;
};
