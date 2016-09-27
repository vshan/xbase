SysPage_BufferManager::SysPage_BufferManager(int numPages)
{
	this->numPages = numPages;
	bufTable = new SysPage_BufPageDesc[numPages];
	pageSize = PAGE_SIZE + PAGE_SIZE_HEADER;
	for (int i = 0; i < numPages; i++) {
		bufTable[i].pData = new char[pageSize];
		memset((void *)bufTable[i].pData, 0, pageSize);
		bufTable[i].prev = i-1;
		bufTable[i].next = i+1;
	}
	bufTable[0].prev = bufTable[numPages - 1].next = INVALID_SLOT;
	free = 0;
	first = last = INVALID_SLOT;
}