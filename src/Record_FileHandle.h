#ifndef RECORD_FILEHANDLE_H
#define RECORD_FILEHANDLE_H

class Record_FileHandle {
public:
	Record_FileHandle();
	ErrCode open(SysPage_FileHandle*, int recordSize);
	ErrCode setHdr(Record_FileHdr h) { hdr = h; return 0};
	~Record_FileHandle();

	ErrCode getRec(const RID &rid, Record_Record &rec);
	ErrCode insertRec(const char *pData, RID &rid);
	ErrCode deleteRec(const RID &rid);
	ErrCode updateRec(const Record_Record &rec);

	ErrCode getSysPage_FileHandle(SysPage_FileHandle &);

	bool hdrChanged() { return isHdrChanged; }
	int fullRecordSize() { return hdr.extRecordSize; }
	int getNumPages() { return hdr.numPages; }
	int getNumSlots();
private:
	bool isValidPageNum(const int pageNum);
	bool isValidRID(const RID rid);
	ErrCode isValid();

	ErrCode getNextFreePage(int& pageNum);
	ErrCode getNextFreeSlot(SysPage_PageHandle& sh, int& pageNum, int& slotNum);
	ErrCode getPageHeader(SysPage_PageHandle sh, Record_PageHdr& pHdr);
	ErrCode setPageHeader(SysPage_PageHandle sh, const Record_PageHdr& pHdr);
	ErrCode getSlotPointer(SysPage_PageHandle sh, int& s, char *& pData);
	ErrCode getFileHeader(SysPage_PageHandle sh);
	ErrCode setFileHeader(SysPage_PageHandle sh);

	SysPage_FileHandle* syspHandle;
    SysPage_FileHdr hdr;
    bool isFileOpen;
    bool isHdrChanged;
};

#endif
