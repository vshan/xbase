/* File to print all the warning & error messages */

#include<cstdio>
#include<cerrno>
#include<iostream>

using namespace std;

char* SYSPAGE_WarningMessage[] = {
    (char*) "page is pinned in buffer",
    (char*) "page is not pinned in buffer",
    (char*) "invalid page number",
    (char*) "file is open",
    (char*) "invalid file descriptor (file is closed)",
    (char*) "page already free",
    (char*) "page already unpinned",
    (char*) "end of file",
    (char*) "attempting to resize the buffer too small"
    //(char*) "invalid page number"
};

char* SYSPAGE_ErrorMessage[] = {
    (char*) "no enough memory",
    (char*) "no buffer space",
    (char*) "incomplete read from file",
    (char*) "incomplete write to file",
    (char*) "incomplete read of header",
    (char*) "incomplete write to header",
    (char*) "new page already in buffer",
    (char*) "hash table entry not found",
    (char*) "page already in hash table",
    (char*) "invalid file name"
};
// method to send a message corresponding to the error code
void SysPage_printError(ErrCode ec)
{
    // WARNINGS
    if(ec >= START_SYSPAGE_WARN && ec <= END_SYSPAGE_WARN)
    {
        cerr<< "SYSPAGE warning: " << SYSPAGE_WarningMessage[ec - START_SYSPAGE_WARN] << "\n";
    }

    // ERRORS
    else if(-ec >= -START_SYSPAGE_ERR && -ec <= -END_SYSPAGE_ERR)
    {
        cerr<< "SYSPAGE error: " << SYSPAGE_ErrorMessage[-ec + START_SYSPAGE_ERR] << "\n";
    }

    else if(ec == SYSPAGE_UNIX)
    {
        #ifdef PC
            cerr <<"OS Error\n";
        #else
            cerr <<strerror(errno)<<"\n";
        #endif
    }

    // return OK
    else if(rc==0)
    {
        cerr << "Return code 0\n";
    }

    else
    {
        cerr << "SYSPAGE Error code : " <<ec<< " out of bounds\n";
    }
}
