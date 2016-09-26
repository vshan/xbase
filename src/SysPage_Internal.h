#ifndef SYSPAGE_INTERNHAL_H
#define SYSPAGE_INTERNHAL_H

#include <cstring>

const int SYSPAGE_BUFFER_SIZE = 40;

struct SysPage_PageHeader {
   int nextFree;
}

const int SYSPAGE_PAGE_SIZE = 4096 - sizeof(int);
const int SYSPAGE_FILE_HDR_SIZE = SYSPAGE_PAGE_SIZE + sizeof(SysPage_PageHeader);


#endif