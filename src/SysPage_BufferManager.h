class SysPage_BufferManager
{
public:
   SysPage_BufferManager();
   ~SysPage_BufferManager();

   ErrCo getPage();
   ErrCo allocatePage();
   ErrCo markDirty();
   ErrCo allocateBlock();
   ErrCo disposeBlock();
   
};