struct SysPage_FileHeader {
   int firstFree;
   int numPages;
};

class SysPage_FileHandle
{
public:
	SysPage_FileHandler();
	~SysPage_FileHandle();

	ErrCo getFirstPage();
	ErrCo getNextPage();
	ErrCo getThisPage();
	ErrCo getLastPage();
	ErrCo getPreviousPage();

   ErrCo allocatePage();
   ErrCo disposePage();
   ErrCo markDirty();
   ErrCo forcePage();

private:
   SysPage_BufferManager* bfrmgr;
   SysPage_FileHeader hdr;
   int unixfd;
   int isFileOpen;
   int isHeaderChanged;

};