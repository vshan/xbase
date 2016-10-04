#ifndef RECORD_MANAGER_H
#define RECORD_MANAGER_H

class Record_Manager {
public:
    Record_Manager(SysPage_Manager &sys);
    ~Record_Manager();

    ErrCode createFile(const char *fileName, int recordSize);

    ErrCode destroyFile(const char* fileName);

    ErrCode openFile(const char *fileName, Record_FileHandle &fileHandle );

    ErrCode closeFile(Record_FileHandle & fileHandle);

private:
    SysPage_Manager &sysPageManager; // A reference to external SysPage_Manager
};

#endif  // RECORD_MANAGER_H
