#include "Record_Internal.h"
#include "Record_Manager.h"
#include "SysPage_FileHandle.h"
#include "SysPage_PageHandle.h"

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

    // call to SysPage openFile
    SysPage_FileHandle spfh;
    ec = spm.openFile(fileName, spfh);

    if(ec < 0)
    {
        SysPage_printError(ec);
        return rc;
    }

    // call to SysPage_FileHandle allocatePage
    SysPage_PageHandle headerPage;
    char *pageData;

    ec = spfh.allocatePage(headerPage);
    if(ec < 0)
    {
        SysPage_printError(ec);
        return ec;
    }

    // get the data
    ec = headerPage.getData(pageData);
    if(ec < 0)
    {
        SysPage_printError(ec);
        return ec;
    }

    Record_FileHdr hdr;
    hdr.firstFree = RECORD_PAGE_LIST_END;
    hdr.numPages = 1; // header page
    hdr.extRecordSize = recordSize;

    // void *memcpy(void* destination, const void* source, size_t num);
    memcpy(pageData, &hdr, sizeof(hdr));

    int headerPageNum;
    headerPage.getPageNum(headerPageNum);
    assert(headerPageNum==0);

    ec = spfh.markDirty(headerPageNum);
    if(ec < 0)
    {
        SysPage_printError(ec);
        return RECORD_SYSPAGE;
    }
    ec = spfh.unpinPage(headerPageNum);
    if(ec < 0)
    {
        SysPage_printError(ec);
        return RECORD_SYSPAGE;
    }

    ec = spm.closeFile(spfh);
    if(ec < 0)
    {
        SysPage_printError(ec);
        return RECORD_SYSPAGE;
    }
    // return ok;
    return 0;

}
