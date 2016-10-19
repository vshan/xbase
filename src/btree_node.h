#ifndef BTREE_NODE_H
#define BTREE_NODE_H

class BTreeNode
{
public:
	BTreeNode(AttrType attrType, int attrLength,
		      SysPage_PageHandle &sph, bool newPage = true,
		      int pageSize = PF_PAGE_SIZE);
	~BTreeNode();
	ErrorCode ResetBTreeNode(SysPage_PageHandle &sph, const BTreeNode& rhs);
	int destroy();
	friend class Index_IndexHandle;
	int getMaxKeys();

	int getNumKeys();
	int setNumKeys(int newNumKeys);
	int getLeft();
	int setLeft(int pageNum);
	int getRight();
	int setRight(int pageNum);

	ErrorCode getKey(int pos, void* &key);
	int setKey(int pos, const void* newkey);
	int copyKey(int pos, void* toKey);

	int insert(const void* newkey, const RID& newrid);
	int remove(const void* newkey, int kpos = -1);
	int findKey(const void* &key, const RID& r = RID(-1,-1));
	RID findAddr(const void* &key);
	RID getAddr(const int pos);
	int findKeyPosition(const void* &key);
	RID findAddrAtPosition(const void* &key);

	ErrorCode split(BTreeNode* rhs);
	ErrorCode merge(BTreeNode* rhs);

	RID getPageRID();
	void setPageRID(const RID&);
	int cmpKey(const void* k1, const void* k2);
	void* largest_key();
private:
	char *keys;
	RID *rids;
	int numKeys;
	int attrLength;
	AttrType attrType;
	int order;
	RID pageRID;	
};

#endif // BTREE_NODE_H