#include "Record_Internal.h"

class Record_Record
{
public:
  Record_Record();
  ~Record_Record();

  ErrCode getdata(char *pData) const;
  ErrCode setData(char *pData, int size, RID id);
  ErrCode getRID(RID &rid) const;
private:
  int recordSize;
  char *data;
  RID rid;
}
