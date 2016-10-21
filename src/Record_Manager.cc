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
    SysPage_PageHandle hpHandle; //header page handle
    char *pageData;

    ec = spfh.allocatePage(hpHandle);
    if(ec < 0)
    {
        SysPage_printError(ec);
        return ec;
    }

    // get the data
    ec = hpHandle.getData(pageData);
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
    hpHandle.getPageNum(headerPageNum);
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

ErrCode Record_Manager::destroyFile(const char* fileName)
{
    ErrCode ec;

    ec = spm.destroyFile(fileName);

    if(ec < 0)
    {
        //print ec;
        return RECORD_SYSPAGE;
    }
    return (ec=0);
}

ErrCode Record_Manager::openFile(const char* fileName, Record_FileHandle &rfh)
{
    SysPage_FileHandle spfh;
    ErrCode ec;

    ec = spm.openFile(fileName,spfh);

    if(ec < 0)
    {
        // printf error
        return RECORD_SYSPAGE;
    }

    SysPage_PageHandle ph;
    char *pData;

    // header page is at 0
    if((ec = spfh.getThisPage(0,ph)) || (ec = spfh.getData(pData)))
    {
        return ec;
    }

    Record_FileHdr rhdr;
    // void *memcpy(void* destination, const void* source, size_t num);
    memcpy(&rhdr, pData, sizeof(hdr));

    ec = rfh.open(&spfh, rhdr.extRecordSize);
    if(ec<0)
    {
        return ec;
    }

    ec = spfh.unpinPage(0); // unpin the header page at 0
    if(ec < 0)
    {
        return ec;
    }

    return (ec=0);
}

ErrCode Record_Manager::closeFile(Record_FileHandle &rfh)
{
    if(!rfh.isFileOpen || rfh.syspHandle==NULL)
        return RECORD_FNOTOPEN;

    if(rfh.isHeaderChanged)
    {
        // write header to disk
        SysPage_PageHandle pfh;
        rfh.syspHandle->getThisPage(0, pfh);
        rfh.setFileHeader(pfh); // write header into file

        ErrCode ec = rfh.syspHandle->markDirty(0);
        if(ec<0)
        {
            return rc;
        }

        ec=rfh.syspHandle->unpinPage(0);
        if(ec<0)
        {
            return ec;
        }

        ec = rfh.forcePages();
        if(ec<0)
        {
            return ec;
        }
    }

    ErrCode ec2 = spm.CloseFile(rfh->syspHandle);
    if(ec2<0)
    {
        return ec2;
    }

    delete rfh->syspHandle;
    rfh->syspHandle=NULL;
    rfh.isFileOpen=FALSE;

    return 0;
}
