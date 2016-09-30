#include<cstdio>
#include<fcntl>
#include<unistd.h>

#include "SYSPAGE_internal.h"
#include "SysPage_FileHandle.h"

// Constructor
SysPage_Manager::SysPage_Manager()
{

}

// Destructor
SysPage_Manager::~SysPage_Manager()
{

}

ErrCode SysPage_Manager::createFile(char* fileName)
{
    int fd; // unix file descriptor

    /*
        O_CREAT
          If the file does not exist, it will be created.

        O_EXCL
          Ensure that this call creates the file: if this flag is
          specified in conjunction with O_CREAT, and pathname already
          exists, then open() will fail.
    */
    if(fd=open(fileName,O_CREAT|O_EXCL|O_WRONLY ,CREATION_PERM)<0)
    {
        return SYSPAGE_UNIX;
    }

    char hdrBuffer[SYSPAGE_FILE_HDR_SIZE];

    //

    /*
        write header to file
        ssize_t write(int fd, const void *buf, size_t count);
    */
    numBytes=write(fd,hdrBuffer,SYSPAGE_FILE_HDR_SIZE);

    // Nothing was written
    if(numBytes==0)
    {
        return SYSPAGE_HDRWRITE;
    }

    // some error occured
    else if(numBytes<0)
    {
        return SYSPAGE_UNIX;
    }

    // OK

    if(close(fd)<0)
        return SYSPAGE_UNIX;

    unlink(fileName);//?????????

    // return OK
    return 0;
}

/*
    Destroy a Page File named fileName (fileName must exist and not open)
    Check the above conditions while making call to the method
*/
ErrCode SysPage_Manager::destroyFile(char* fileName)
{
    // Remove the file from ?
    if(unlink(fileName)<0)
        return SYSPAGE_UNIX;

    // return OK
    return 0;
}

ErrCode SysPage_Manager::openFile(char* fileName)
{

}

/*
    Close file associated with fileHandle
    The file should have been opened with OpenFile().
    Also, flush all pages for the file from the page buffer
    It is an error to close a file with pages still fixed in the buffer.
*/
ErrCode SysPage_Manager::closeFile(SysPage_FileHandle &fileHandle)
{
    ErrCode ec;
    // check if file is not already closed
    if(!fileHandle.isFileOpen)
        return SYSPAGE_CLOSEDFILE;

    // Flush all the pages of this file and (write out the header)?
    if((ec=fileHandle.flushPages()))
        return ec;

    // Close the file
    if(Close(fileHandle.unixfd) < 0)
        return SYSPAGE_UNIX;

    // mark the file as closed
    fileHandle.isFileOpen = FALSE;

    // reset the buffer manager pointer in the fileHandle ????????
    fileHandle.bfrmgr = NULL;
}
