class SysPage_Manager
{
public:
	SysPage_Manager();
	~SysPage_Manager();

	ErrCo createFile();
	ErrCo openFile();
	ErrCo closeFile();
	ErrCo disposeFile();

private:
   SysPage_BufferManager* mbfrmgr;
};