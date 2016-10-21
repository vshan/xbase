#ifndef INDEX_INDEX_SCAN_H
#define INDEX_INDEX_SCAN_H

class Index_IndexScan
{
public:
	Index_IndexScan();
	~Index_IndexScan();

	ErrorCode openScan(const Index_IndexHandle &indexHandle,
		               CompOp compOp,
		               void *value,
		               ClientHint pinHint = NO_HINT,
		               bool desc = false);

	ErrorCode getNextEntry(RID &rid);
	ErrorCode getNextEntry(void *& key, RID &rid, int& numScanned);
	ErrorCode closeScan();
	ErrorCode resetState();

	bool isOpen() { return (isOpen && pred != NULL && pixh != NULL); }
	bool isDesc() { return desc; }
private:
	Predicate *pred;
	Index_IndexHandle *pixh;
	BTreeNode* currNode;
	int currPos;
	void *currKey;
	RID currRid;
	bool isOpen;
	bool desc;
	bool eof;
	bool foundOne;
	BTreeNode* last_node;
	CompOp c;
	void *value;
};

#endif