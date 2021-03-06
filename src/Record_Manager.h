#ifndef RECORD_MANAGER_H
#define RECORD_MANAGER_H

#include "Record_Internal.h"

class Record_Manager {
public:
    Record_Manager(SysPage_Manager &spm);
    ~Record_Manager();

    ErrCode createFile(const char *fileName, int recordSize);

    ErrCode destroyFile(const char* fileName);

    ErrCode openFile(const char *fileName, Record_FileHandle &fileHandle );

    ErrCode closeFile(Record_FileHandle & fileHandle);

private:
    SysPage_Manager &spm; // A reference to external SysPage_Manager
};

#endif  // RECORD_MANAGER_H
