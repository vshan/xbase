class SysPage_PageHandle
{
public:
	SysPage_PageHandle();
	~SysPage_PageHandle();

	ErrCo getData();
	ErrCo getPageNum();
private:
	int pageNumber;
	char *pageData;
};