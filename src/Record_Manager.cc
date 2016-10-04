#include "Record_Internal.h"
#include "Record_Manager.h"
#include "SysPage_FileHandle.h"

// Constructor
Record_Manager::Record_Manager(SysPage_Manager &spm)
{
    this->spm = spm;
}

// Destructor
~Record_Manager()
{

}

ErrCode Record_Manager::createFile(const char *fileName, int recordSize)
{
    if(recordSize<0)
        return RECORD_BADRECSIZE;

    if(recordSize >= SYSPAGE_PAGE_SIZE - (int) sizeof(Record_PageHdr)
    {
        return RECORD_SIZETOOBIG;
    }

    // call to SysPage createFile method
    int ec = sysPageManager.createFile(fileName);

    if(ec < 0)
    {
        RECORD_printError(ec);
        return ec;
    }

    SysPage_FileHandle spfh;

    ec = spm.openFile(fileName, spfh);

    if(ec < 0)
    {
        SysPage_printError(ec);
        return rc;
    }

    SysPage_PageHandle spph;
    char *data;






}
