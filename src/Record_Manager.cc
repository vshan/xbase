#include "Record_Manager.h"

using namespace std;

// Constructor
Record_Manager::Record_Manager(SysPage_Manager &sysPageManager)
{
    this.sysPageManager = sysPageManager;
}

// Destructor
~Record_Manager()
{

}

ErrCode Record_Manager::createFile(const char *fileName, int recordSize)
{

}
