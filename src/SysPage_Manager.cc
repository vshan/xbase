#include "SysPage_FileHandle.h"
#include "SysPage_Manager.h"

using namespace std;

// Constructor
SysPage_Manager::SysPage_Manager()
{
    // Create Buffer Manager
    bufferMgr = new SysPage_BufferManager(SYSPAGE_BUFFER_SIZE); // SYSPAGE_BUFFER_SIZE is the number of pages in a file
}

// Destructor
SysPage_Manager::~SysPage_Manager()
{
    // destroy the buffer manager objects
    delete bufferMgr;
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
    if(fd=open(fileName,
    O_BINARY | O_CREAT | O_EXCL | O_WRONLY ,
    CREATION_PERM) < 0)
    {
        return SYSPAGE_UNIX;
    }

    // Initialize the file header - must reserve FileHdrSize bytes in memory
    // though the actual size of FileHdr is smaller
    char hdrBuffer[SYSPAGE_FILE_HDR_SIZE];

    //
    memset(hdrBuffer, 0, SYSPAGE_FILE_HDR_SIZE);

    // Initialize
    SysPage_FileHeader *header = (SysPage_FileHeader*) hdrBuffer;
    header->firstFree = SYSPAGE_PAGE_LIST_END;
    header->numPages = 0;

    /*
        write header to file
        ssize_t write(int fd, const void *buf, size_t count);
    */
    numBytes=write(fd,hdrBuffer,SYSPAGE_FILE_HDR_SIZE);

    // Error while writing
    if(numBytes!=SYSPAGE_FILE_HDR_SIZE)
    {
        // close and remove the file
        close(fd);
        unlink(fileName);

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
    }

    // OK
    if(close(fd)<0)
        return SYSPAGE_UNIX;

    unlink(fileName);//why???????

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

ErrCode SysPage_Manager::openFile(char* fileName, SysPage_FileHandle &fileHandle)
{
    int ec; // return the error code

    // Check whether the file is not already openFile
    if(fileHandle.isFileOpen)
    {
        return SYSPAGE_FILEOPEN;
    }

    // Open the file
    if((fileHandle.unixfd = open(fileName,
        O_BINARY | O_RDWR ))<0)
    {
        return SYSPAGE_UNIX;
    }

    // syntax of read :
    // Read the file header
    int numBytes = read(fileHandle.unixfd, (char *)&fileHandle.hdr,sizeof(SysPage_FileHeader));

    if(numBytes!=sizeof(SysPage_FileHeader))
    {
        if(numBytes<0)
            ec = PF_UNIX;
        else
            ec = PF_HDRREAD;

        // close the file
        close(fileHandle.unixfd);
        fileHandle.isFileOpen = FALSE;

        // Return the error code;
        return ec;
    }

    else
    {
        // Set the file header to be not changed
        fileHandle.isHeaderChanged = FALSE;

        // set the attributes of the fileHandle object to refer to the open file
        fileHandle.bufferMgr = this.bufferMgr;
        fileHandle.isFileOpen = TRUE;

        // return ok
        return 0;
    }
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
