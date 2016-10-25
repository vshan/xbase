Record_FileScan::Record_FileScan() : isOpen(false)
{
	pred = NULL;
	prmh = NULL;
	current = RID(1, -1);
}

Record_FileScan::~Record_FileScan()
{
}

ErrCode Record_FileScan::openScan(const Record_FileHandle &fileHandle,
	                              AttrType attrType,
	                              int attrLength,
	                              int attrOffset,
	                              CompOp compOp,
	                              void *value,
	                              ClientHint pinHint)
{
	if (isOpen)
	{
		return RECORD_HANDLEOPEN;
	}

	// const_cast can be used to change non-const class members inside a const member function.
	prmh = const_cast<Record_FileHandle*>(&fileHandle);
	isOpen = true;
	pred = new Predicate(attrType,
		                 attrLength,
		                 attrOffset,
		                 compOp,
		                 value,
		                 pinHint);
	return 0;
}

// go to the slot before the first slot with data
ErrCode Record_FileScan::goToPage(int p)
{
	if(!isOpen)
		return RECORD_FNOTOPEN;
	assert(prmh != NULL && pred!=NULL && isOpen);

	current = RID(p, -1);
	Record_Record rec;
	getNextRec(rec);
	RID rid;
	rec.getRid(rid);
	current = RID(p, rid.returnSlot() -1);
	return 0;
}

ErrCode Record_FileScan::getNextRec(Record_Record &rec)
{
	if(!isOpen)
		return RECORD_FNOTOPEN;
	assert(prmh != NULL && pred!=NULL && isOpen);

	SysPage_PageHandle sph;
	Record_PageHdr pHdr(prmh->getNumSlots());

	ErrCode ec;
	for (int j = current.returnPage(); j < prmh->getNumPages(); j++)
	{
		if((ec = prmh->syspHandle->getThisPage(j, ph))
			|| (ec = prmh->syspHandle->unpinPage(j)))
				return ec;

		if((ec = prmh->getPageHeader(sph, pHdr)))
			return ec;

		bitmap b(pHdr.freeSlotMap, prmh->getNumSlots());
		int i = -1;
		if (current.returnPage() == j)
			i = current.returnSlot()+1;
		else
			i = 0;
		for (; i < prmh->getNumSlots(); i++)
		{
			if (!b.test(i))
			{
				current = RID(j, i); // j=pageNum, i=slotNum
				prmh->getRec(current, rec);
				char * pData = NULL;
				rec.getData(pData);
				if (pred->eval(pData, pred->initOp()))
				{
					return 0;
				}
				else
				{
					// get next rec
				}
			}
		}
	}
	return RECORD_EOF;
}

ErrCode Record_FileScan::closeScan()
{
	if (!isOpen)
		return RECORD_FNOTOPNEN;
	assert(prmh!=NULL && pred!=NULL);

	isOpen = false;
	if (pred != NULL)
		delete pred;
	current = RID(1, -1);
	return 0;
}
