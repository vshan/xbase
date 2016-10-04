#include "Record_Record.h"

using namespace std;

Record_Record::Record_Record()
  :recordSize(-1), data(NULL), rid(-1,-1)
{
}

Record_Record::~Record_Record()
{
	if (data != NULL) {
		delete [] data;
	}
}

// Allows a resetting as long as size matches.
ErrCode Record_Record::setData(char *pData, int size, RID rid_)
{
	if(recordSize != -1 && (size != recordSize))
		return RECORD_RECSIZEMISMATCH;
	recordSize = size;
  this->rid = rid_;
	if (data == NULL)
		data = new char[recordSize];
  memcpy(data, pData, size);
	return 0;
}

ErrCode Record_Record::getData(char *&pData) const
{
	if (data != NULL && recordSize != -1)
  {
		pData = data;
		return 0;
	}
	else
		return RECORD_NULLRECORD;
}

ErrCode Record_Record::getRid(RID &rid) const
{
	if (data != NULL && recordSize != -1)
	{
    rid = this->rid;
    return 0;
  }
	else
		return RECORD_NULLRECORD;
}
