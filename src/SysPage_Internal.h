#ifndef SYSPAGE_INTERNHAL_H
#define SYSPAGE_INTERNHAL_H

#include <cstring>

const int SYSPAGE_BUFFER_SIZE = 40;

struct SysPage_PageHeader {
   int nextFree;
}

const int SYSPAGE_PAGE_SIZE = 4096 - sizeof(int);
const int SYSPAGE_FILE_HDR_SIZE = SYSPAGE_PAGE_SIZE + sizeof(SysPage_PageHeader);

#define SYSPAGE_PAGEPINNED      (START_SYSPAGE_WARN + 0) // page pinned in buffer
#define SYSPAGE_PAGENOTINBUF    (START_SYSPAGE_WARN + 1) // page isn't pinned in buffer
#define SYSPAGE_INVALIDPAGE     (START_SYSPAGE_WARN + 2) // invalid page number
#define SYSPAGE_FILEOPEN        (START_SYSPAGE_WARN + 3) // file is open
#define SYSPAGE_CLOSEDFILE      (START_SYSPAGE_WARN + 4) // file is closed
#define SYSPAGE_PAGEFREE        (START_SYSPAGE_WARN + 5) // page already free
#define SYSPAGE_PAGEUNPINNED    (START_SYSPAGE_WARN + 6) // page already unpinned
#define SYSPAGE_EOF             (START_SYSPAGE_WARN + 7) // end of file
#define SYSPAGE_TOOSMALL        (START_SYSPAGE_WARN + 8) // Resize buffer too small
#define SYSPAGE_LASTWARN        SYSPAGE_TOOSMALL

#define SYSPAGE_NOMEM           (START_SYSPAGE_ERR - 0)  // no memory
#define SYSPAGE_NOBUF           (START_SYSPAGE_ERR - 1)  // no buffer space
#define SYSPAGE_INCOMPLETEREAD  (START_SYSPAGE_ERR - 2)  // incomplete read from file
#define SYSPAGE_INCOMPLETEWRITE (START_SYSPAGE_ERR - 3)  // incomplete write to file
#define SYSPAGE_HDRREAD         (START_SYSPAGE_ERR - 4)  // incomplete read of header
#define SYSPAGE_HDRWRITE        (START_SYSPAGE_ERR - 5)  // incomplete write to header

// Internal errors
#define SYSPAGE_PAGEINBUF       (START_SYSPAGE_ERR - 6) // new page already in buffer
#define SYSPAGE_HASHNOTFOUND    (START_SYSPAGE_ERR - 7) // hash table entry not found
#define SYSPAGE_HASHPAGEEXIST   (START_SYSPAGE_ERR - 8) // page already in hash table
#define SYSPAGE_INVALIDNAME     (START_SYSPAGE_ERR - 9) // invalid PC file name

// Error in UNIX system call or library routine
#define SYSPAGE_UNIX            (START_SYSPAGE_ERR - 10) // Unix error
#define SYSPAGE_LASTERROR       SYSPAGE_UNIX

// Printing of the errors
void SysPage_printError(int ec)
{


}

#endif
