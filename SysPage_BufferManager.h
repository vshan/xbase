class SysPage_BufferManager
{
public:
	SysPage_BufferManager();
	~SysPage_BufferManager();

	ErrCode getPage();
	ErrCode allocatePage();
	ErrCode markDirty();
	ErrCode allocateBlock();
	ErrCode disposeBlock();
	
};

ErrCode SysPage_BufferManager::getPage()
{
	
}