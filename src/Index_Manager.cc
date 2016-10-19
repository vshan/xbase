Index_Manager::Index_Manager(SysPage_Manager &spm):spm(spm)
{

}

Index_Manager::~Index_Manager();

ErrorCode Index_Manager::createIndex(const char *fileName, int indexNo,
	                                 AttrType attrType, int attrLength,
	                                 int pageSize)
{
	stringstream newname;
	newname << fileName << "." << indexNo;

	ErrorCode ec = spm.createFile(newname.str().c_str());

	SysPage_FileHandle sfh;
    ec = spm.openFile(newname.str().c_str(), sfh);
    SysPage_PageHandle headerPage;
    char * pData;
    ec = sfh.allocatePage(headerPage);
    headerPage.getData(pData);

    Index_FileHdr hdr;
    hdr.numPages = 1;
    hdr.pageSize = pageSize;
    hdr.pairSize = attrLength + sizeof(RID);
    hdr.order = -1;
    hdr.height = 0;
    hdr.rootPage = -1;
    hdr.attrType = attrType;
    hdr.attrLength = attrLength;

    memcpy(pData, &hdr, sizeof(hdr));

    int headerPageNum;
    headerPage.getPageNum(headerPageNum);
    sfh.markDirty(headerPageNum);
    sfh.unpinPage(headerPageNum);
    spm.closeFile(sfh);

    return (0);
}

ErrorCode Index_Manager::destroyIndex(const char *fileName, int indexNo)
{
	stringstream newname;
	newname << fileName << "." << indexNo;

	ErrorCode ec = spm.destroyFile(newname.str().c_str());
	return (ec = 0);
}

ErrorCode Index_Manager::openIndex(const char *fileName, int indexNo, Index_Handle& ixh)
{
	SysPage_FileHandle sfh;
	stringstream newname;
	newname << fileName << "." << indexNo;

	spm.openFile(newname.str().c_str(), sfh);
	SysPage_PageHandle ph;
	char * pData;
	sfh.getThisPage(0, ph);
	ph.getData(pData);

	Index_FileHdr hdr;
	memcpy(&hdr, pData, sizeof(hdr));
	ixh.open(&sfh, hdr.pairSize, hdr.rootPage, hdr.pageSize);
	sfh.unpinPage(0);
	return 0;
}

ErrorCode Index_Manager::closeIndex(Index_IndexHandle &ixh)
{
	if (ixh.hdrChanged())
	{
		SysPage_PageHandle ph;
		ixh.syspHandle->getThisPage(0, ph);
		ixh.setFileHeader(ph);
		ixh.syspHandle->markDirty(0);
		ixh.syspHandle->unpinPage(0);
		ixh.forcePages();
	}
	spm.closeFile(*ixh.syspHandle);
	ixh.~Index_IndexHandle();
	ixh.syspHandle = NULL;
	ixh.isFileOpen = false;
	return 0;
}