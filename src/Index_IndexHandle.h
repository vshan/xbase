#ifndef INDEX_INDEXHANDLE_H
#define INDEX_INDEXHANDLE_H

struct Index_FileHdr {
	int numPages;
	int pageSize;
	int rootPage;
	int pairSize;
	int order;
	int height;
	AttrType attrType;
	int attrLength;
};

const int INDEX_PAGE_LIST_END = -1;

class Index_IndexHandle
{
	friend class Index_Manager;
public:
	Index_IndexHandle();
	~Index_IndexHandle();

	ErrorCode insertEntry(void *pData, const RID &rid);
	ErrorCode deleteEntry(void *pData, const RID &rid);

	ErrorCode search(void *pData, RID &rid);
	ErrorCode forcePages();

	ErrorCode open(SysPage_FileHandle * sfh, int pairSize, int p, int pageSize);
    ErrorCode getFileHeader(SysPage_PageHandle sph);
    ErrorCode setFileHeader(SysPage_PageHandle sph);
    bool hdrChanged() { return isHdrChanged; }
    int getNumPages() { return hdr.numPages; }
    AttrType getAttrType() { return hdr.attrType; }
    int getAttrLength() { return hdr.attrLength; }

    ErrorCode getNewPage(int& pageNum);
    ErrorCode disposePage(const int& pageNum);

    BTreeNode* findLeaf(const void *pData);
    BTreeNode* findSmallestLeaf();
    BTreeNode* findLargestLeaf();
    BTreeNode* dupScanLeftFind(BTreeNode* right,
    	                       void *pData,
    	                       const RID& rid);
    BTreeNode* findLeafForceRight(const void* pData);
    BTreeNode* fetchNode(RID r);
    BTreeNode* fetchNode(int pageNum);
    void resetNode(BTreeNode*& old, int pageNum);
    void resetNode(BTreeNode*& old, RID r);

    int getHeight();
    void setHeight(const int&);

    BTreeNode* getRoot();
    ErrorCode pin(int pageNum);
    ErrorCode unPin(int pageNum);
private:
	ErrorCode getThisPage(int pageNum, SysPage_PageHandle& sph);
	Index_FileHdr hdr;
	bool isFileOpen;
	SysPage_FileHandle *syspHandle;
	bool isHdrChanged;
	BTreeNode* root;
	BTreeNode** path;

	int *pathP;
	void *treeLargest;
};

#endif