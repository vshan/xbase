Record_FileHandle::Record_FileHandle()
 :syspHandle(NULL), isFileOpen(false), isHdrChanged(false)
{

}

// Destructor
Record_FileHandle::~Record_FileHandle()
{
	if (syspHandle != NULL)
		delete syspHandle;
}

ErrCode Record_FileHandle::open(SysPage_FileHandle* spfh, int size)
{
	isFileOpen = true;
	spfh = new SysPage_FileHandle;
 	*syspHandle = *spfh;
	SysPage_PageHandle sph;
 	spfh->getThisPage(0, sph);
 	spfh->unpinPage(0); // needs to be called everytime getThisPage() is called
 	this->getFileHeader(sph);
 	isHdrChanged = true;
 	ErrCode invalid = isValid();
    if(invalid)
        return invalid;

 	return 0;
}

ErrCode Record_FileHandle::getSysPage_FileHandle(SysPage_FileHandle &lvalue)
{
 	lvalue = *syspHandle;
 	return 0;
}

ErrCode Record_FileHandle::getNextFreeSlot(SysPage_PageHandle &sh, int& pageNum, int& slotNum)
{
	ErrCode invalid = isValid(); if (invalid) return invalid;

 	ErrCode ec;
 	Record_PageHdr pHdr(this->getNumSlots()); // struct Record_FileHdr defined in Record_Internal.h

 	if((ec = this->getNextFreePage(pageNum))
        || (ec = syspHandle->getThisPage(pageNum, sh))
        || (ec = syspHandle->unpinPage(pageNum))
        || (ec = this->getPageHeader(sh, pHdr)))
            return ec;

 	bitmap b(pHdr.freeSlotMap, this->getNumSlots());

 	for (int i = 0; i < this->getNumSlots(); i++)
 	{
 		if (b.test(i)) {
 			slotNum = i;
 			return 0;
 		}
  	}
    // This page is full
  	return -1; // unexpected error
}

ErrCode Record_FileHandle::getNextFreePage(int& pageNum)
{
	ErrCode invalid = isValid(); if (invalid) return invalid;
	SysPage_PageHandle sph;
	Record_PageHdr pHdr(this->getNumSlots());
	int p;
    ErrCode ec;

	if (hdr.firstFree != RECORD_PAGE_LIST_END)
	{
		if((ec = syspHandle->getThisPage(hdr.firstFree, sph))
            || (ec = ph.getPageNum(p))
            || (ec = syspHandle->markDirty(p))
            || (ec = syspHandle->unpinPage(hdr.firstFree))
            || (ec = this->getPageHeader(sph, pHdr)))
                return ec;
	}

	if (hdr.numPages == 0 ||
	    hdr.firstFree == RECORD_PAGE_LIST_END ||
		pHdr.numFreeSlots == 0)
	{
		if(pHdr.nextFree == RECORD_PAGE_FULLY_USED)
		{
            //std::cerr<<Record_FileHandle :: getNextFreePage -Page Full!;
		}

		{
			char *pData;
            ErrCode ec;

			if((ec = syspHandle->allocatePage(sph))
                || (ec = sph.getData(pData))
                || (ec = sph.getPageNum(pageNum)))
                    return ec;

            // Add page header
			Record_PageHdr phdr(this->getNumSlots());
			phdr.nextFree = RECORD_PAGE_LIST_END;
			bitmap b(this->getNumSlots());
			b.set(); // Initially all slot are free
			b.to_char_buf(phdr.freeSlotMap, b.numChars());
			phdr.to_buf(pData);
			if((ec = syspHandle->unpinPage(pageNum)))
                return ec;
		}

        // Add page to the free list
		hdr.firstFree = pageNum;
		hdr.numPages++;
        assert(hdr.numPages > 1);
		isHdrChanged = true;
		return 0; // pageNum is set correctly
	}
    // return existing free page
	pageNum = hdr.firstFree;
	return 0;
}

ErrCode Record_FileHandle::getPageHeader(SysPage_PageHandle sph, Record_PageHdr& pHdr)
{
	char * buf;
    ErrCode ec;
    if((ec = sph.getData(buf))
        return ec;
    pHdr.from_buf(buf);
    return 0;
}

ErrCode Record_FileHandle::setPageHeader(SysPage_PageHandle sph, Record_PageHdr& pHdr)
{
	char * buf;
	ErrCode ec;
    if((ec = sph.getData(buf))
        return ec;
	pHdr.to_buf(buf);
	return 0;
}

// void *memcpy(void* destination, const void* source, size_t num);
// get header from the first page of a newly opened file
ErrCode Record_FileHandle::getFileHeader(SysPage_PageHandle sph)
{
	char * buf;
    ErrCode ec;
	if((ec = sph.getData(buf))
        return ec;
	memcpy(&hdr, buf, sizeof(hdr));
	return 0;
}

ErrCode Record_FileHandle::setFileHeader(SysPage_PageHandle sph)
{
	char * buf;
    ErrCode ec;
	if((ec = sph.getData(buf))
        return ec;
	memcpy(buf, &hdr, sizeof(hdr));
	return 0;
}

ErrCode Record_FileHandle::getSlotPointer(SysPage_PageHandle sph, int slotNum, char *& pData)
{
    ErrCode invalid = isValid(); if(invalid) return invalid;
    ErrCode ec = sph.getData(pData);
	if(ec >= 0)
    {
        bitmap b(this->getNumSlots());
    	pData = pData + (Record_PageHdr(this->getNumSlots()).size());
    	pData = pData + slotNum * this->fullRecordSize();
    }
	return ec;
}

int Record_FileHandle::getNumSlots()
{
	if (this->fullRecordSize() != 0)
	{
		int bytes_available = SYSPAGE_PAGE_SIZE - sizeof(Record_PageHdr);
		int slots = floor(1.0 * bytes_available / (this->fullRecordSize() + 1/8));
		int r = sizeof(Record_PageHdr) + bitmap(slots).numChars();

		while ((slots*this->fullRecordSize()) + r > SYSPAGE_PAGE_SIZE) {
			slots--;
			r = sizeof(Record_PageHdr) + bitmap(slots).numChars();
		}
		return slots;
	}
	else {
		return RECORD_RECSIZEMISMATCH;
	}
}

ErrCode Record_FileHandle::getRec(const RID &rid, Record_Record &rec)
{
	ErrCode inalid = isValid(); if (invalid) return invalid;
	if (!this->isValidRID(rid))
		return RECORD_BAD_RID;
	int p;
	int s;
	rid.getPageNum(p);
	rid.getSlotNum(s);
	SysPage_PageHandle sph;
	Record_PageHdr pHdr(this->getNumSlots());
	syspHandle->getThisPage(p, sph);
	syspHandle->unpinPage(p);
	this->getPageHeader(sph, pHdr);
	bitmap b(pHdr.freeSlotMap, this->getNumSlots());

	char * pData  = NULL;
	this->getSlotPointer(sph, s, pData);

	rec.setData(pData, hdr.extRecordSize, rid);
	return 0;
}

ErrCode Record_FileHandle::insertRec(const char * pData, RID &rid)
{
	ErrCode invalid = isValid(); if (invalid) return invalid;
	SysPage_PageHandle sph;
	Record_PageHdr pHdr(this->getNumSlots());
	int p; // pagenum
	int s; // slotnum
	char * pSlot;
	this->getNextFreeSlot(sph, p, s);
	this->getPageHeader(sph, pHdr);
	bitmap b(pHdr.freeSlotMap, this->getNumSlots());
	this->getSlotPointer(sph, s, pSlot);
	rid = RID(p, s);
	memcpy(pSlot, pData, this->fullRecordSize());
	b.reset(s);
	pHdr.numFreeSlots--;
	if (pHdr.numFreeSlots == 0) {
		hdr.firstFree = pHdr.nextFree;
		pHdr.nextFree = RECORD_PAGE_FULLY_USED;
	}
	b.to_char_buf(pHdr.freeSlotMap, b.numChars());
	this->setPageHeader(sph, pHdr);
	return 0;
}

ErrCode Record_FileHandle::deleteRec(const RID &rid)
{
	int p; // pagenum
	int s; // slotnum
	rid.getPageNum(p);
	rid.getSlotNum(s);
	SysPage_PageHandle sph;
	Record_PageHdr pHdr(this->getNumSlots());

	syspHandle->getThisPage(p, sph);
	syspHandle->markDirty(p);
	syspHandle->unpinPage(p);
	this->getPageHeader(sph, pHdr);

	bitmap b(pHdr.freeSlotMap, this->getNumSlots());

	b.set(s);
	if (pHdr.numFreeSlots == 0)
	{
		pHdr.nextFree = hdr.firstFree;
		hdr.firstFree = p;
	}
	pHdr.numFreeSlots++;
	b.to_char_buf(pHdr.freeSlotMap, b.numChars());
	this->setPageHeader(sph, pHdr);
	return 0;
}

ErrCode Record_FileHandle::updateRec(const Record_Record &rec)
{
	RID rid;
	rec.getRid(rid);
	int p;
	int s;
	rid.getPageNum(p);
	rid.getSlotNum(s);

	SysPage_PageHandle sph;
	char * pSlot;

	Record_PageHdr pHdr(this->getNumSlots());
	syspHandle->getThisPage(p, sph);
	syspHandle->markDirty(p);
	syspHandle->unpinPage(p);
	this->getPageHeader(sph, pHdr);

	char * pData = NULL;
	rec.getData(pData);
	this->getSlotPointer(sph, s, pSlot);
	memcpy(pSlot, pData, this->fullRecordSize());
	return 0;
}

ErrCode Record_FileHandle::forcePages(int pageNum)
{
    ErrCode ec = invalid = IsValid(); if(invalid) return invalid;
    if(!this->isValidPageNum(pageNum) && pageNum != ALL_PAGES)
        return RECORD_BAD_RID;

	return syspHandle->forcePages(pageNum);
}

bool Record_FileHandle::isValidPageNum(const int pageNum)
{
	return (isFileOpen   &&
		    pageNum >= 0 &&
		    pageNum < hdr.numPages);
}

bool Record_FileHandle::isValidRID(const RID rid)
{
	int p;
	int s;
	rid.getPageNum(p);
	rid.getSlotNum(s);

	return (isValidPageNum(p) &&
	        s >= 0            &&
	        s < this->getNumSlots());
}

ErrCode Record_FileHandle::isValid()
{
	if ((syspHandle == NULL) || !isFileOpen)
		return RECORD_FNOTOPEN;
	if (getNumSlots() <= 0)
		return RECORD_RECSIZEMISMATCH;
	return 0;
}
