#ifndef INDEX_MANAGER_H
#define INDEX_MANAGER_H

class Index_Manager
{
public:
	Index_Manager(SysPage_Manager &spm);
	~Index_Manager();

	ErrorCode createIndex(const char *fileName, int indexNo,
		                  AttrType attrType, int attrLength,
		                  int pageSize = SYSPAGE_PAGE_SIZE);

	ErrorCode destroyIndex(const char* fileName, int indexNo);

	ErrorCode openIndex(const char* fileName, int indexNo,
		                Index_IndexHandle &indexHandle);

	ErrorCode closeIndex(Index_IndexHandle &indexHandle);
private:
	SysPage_Manager &spm;
};

#endif