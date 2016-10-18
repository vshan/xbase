#ifndef RECORD_FILESCAN_H
#define RECORD_FILESCAN_H

class Record_FileScan {
public:
	Record_FileScan();
	~Record_FileScan();

	ErrCode openScan (const Record_FileHandle &fileHandle,
		              AttrType attrType,
		              int attrLength,
		              int attrOffset,
		              CompOp compOp,
		              void *value,
		              ClientHint pinHint = NO_HINT);
	ErrCode getNextRec(Record_Record &rec);
	ErrCode closeScan();
	bool isOpen() { return (isOpen && prmh != NULL && pred != NULL); }
	void resetState() { current = RID(1, -1); }
	ErrCode goToPage(int p);
	int getNumSlotsPerPage() { return prmh->getNumSlots(); }
private:
	Predicate *pred;
	Record_FileHandle *prmh;
	RID current;
	bool isOpen;
};

#endif