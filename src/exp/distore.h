typedef int StatusCode;

struct DS_FileHeader {
  int firstFree;
  int numPages;
};

class DS_Manager {
public:
  DS_Manager();
  ~DS_Manager();
  StatusCode createFile(const char *filename);
  StatusCode loadFile(const char *filename, DS_FileH &fileHandle);
private:
  DS_BufferManager *bm;
  DS_RemoteManager *rm;
};

class DS_FileH {
public:
  DS_FileH();
  ~DS_FileH();

  StatusCode getPage(int pageNum, DS_PageH &pageHandle);
  StatusCode allocatePage(DS_PageH &pageHandle);
  StatusCode markDirty(int pageNum);
  StatusCode unpinPage(int pageNum);
private:
  DS_BufferManager *bm;
  DS_RemoteManager *rm;
  DS_FileHeader hdr;
  bool isRemote;
  string ipaddr;
  string port;
};