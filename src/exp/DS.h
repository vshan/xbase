using namespace std;

#define DS_SUCCESS 0
#define DS_SMALL_BUF_SIZE 1028
#define DS_NOBUF 2
#define DS_INVALIDPAGE (-1)
#define DS_PAGE_LIST_END (-2)
#define DS_PAGE_SIZE 4096
#define DS_BUF_SIZE 4096
#define DS_NULL_PARAM 3
#define DS_NO_OPEN_FILE 4
#define DS_UNPINNED_PAGE 5
#define DS_CHAR_BUF_SIZE 8192
#define DS_PROTO_NAME_REQ 6
#define DS_PROTO_NAME_REQ_CODE 70
#define DS_NAME_SERVER 8
#define DS_NAME_SERVER_PORT "6000"
#define DS_PROTO_LOAD_PAGE 10
#define DS_PROTO_LOAD_PAGE_CODE 50
#define DS_PROTO_WRITE_PAGE 12 
#define DS_PROTO_WRITE_PAGE_CODE 60
#define DS_REMOTE_WRITE_ERROR 14
#define DS_PROTO_ALLOC_PAGE 15
#define DS_PROTO_ALLOC_PAGE_CODE 16
#define DS_SUCC_REM_READ_CODE 17
#define DS_SUCC_REM_WRI_CODE 18
#define DS_UNIX 20
#define DS_INCOMPLETEREAD 21
#define DS_INCOMPLETEWRITE 22
#define DS_INVALID_SLOT (-1)
#define DS_INVALID_PAGE (-5)
#define DS_END_FILE (-27)
#define DS_EOF (-2)
#define DS_BFRMGR_NUMPAGES 40


typedef int StatusCode;

struct DS_FileHeader {
  int firstFree;
  int numPages;
};

const int DS_FILE_HDR_SIZE = sizeof(DS_FileHeader);

struct ProtocolParseObj
{
  int code;
  int pageNum;
  char fileName[4096];
  char pageContents[4096];
  char ipAddr[20];
  char port[10];
};

struct DS_BufPageDesc {
  char *pData;
  int next;
  int prev;
  int isDirty;
  int pinCount;
  int pageNum;
  int fd;
  char *ipAddr;
  char *port;
  char *fileName;
  bool isRemote;
};

class DS_RemoteManager {
public:
  DS_RemoteManager();
  ~DS_RemoteManager();

  StatusCode getRemoteHeaderFile(char *fileName,
                                 string &header_content);
  StatusCode rawSendRecv(char *host, char *port, 
                         char request[], char reply[]);
  boost::asio::ip::tcp::socket* enableConnection(char *host, char *port);
  size_t writeRead(boost::asio::ip::tcp::socket *s, char request[], char reply[]);
  StatusCode getRemotePage(char *host, char *port, char *fileName,
                           int pageNum, char page_content[]);
  StatusCode setRemotePage(char *host, char *port, char *fileName,
                           int pageNum, char page_content[]);
  StatusCode makeProtocolMsg(int proto_type, void *value1, 
                             void *value2, void *value3, char msg[]);
  StatusCode makeProtoHelper(int status_code, void *value1, 
                             void *value2, char msg[]);
  StatusCode parseProtocolMsg(string msg, ProtocolParseObj &ppo);
  StatusCode parseIncomingMsg(char *msg, ProtocolParseObj &ppo);
  StatusCode handleProtoReq(ProtocolParseObj &ppo, char repbuf[]);
  StatusCode getLocalPage(int fd, int pageNum, char *dest);
  StatusCode setLocalPage(int fd, int pageNum, char *source);
  void server(boost::asio::io_service& io_service, unsigned short port);
  static void spawnServer(unsigned short port, DS_RemoteManager *window);
  static void session(DS_RemoteManager *window, boost::asio::ip::tcp::socket sock);
};


class DS_BufferManager {
public:
  DS_BufferManager(int numPages, DS_RemoteManager *rem_mgr);
  ~DS_BufferManager();

  StatusCode getPage(int fd, int pageNum, char **ppBuffer);
  StatusCode getPage(char *ipAddr, char *port, char *fileName, int pageNum, char **ppBuffer);
  StatusCode markDirty(int fd, int pageNum);
  StatusCode markDirty(char *ipAddr, char *port, char *fileName, int pageNum);
  StatusCode unpinPage(int fd, int pageNum);
  StatusCode unpinPage(char *ipAddr, char *port, char *fileName, int pageNum);
  StatusCode forcePage(int fd, int pageNum);
  StatusCode forcePage(char *ipAddr, char *port, char *fileName, int pageNum);
  StatusCode allocatePage(int fd, int pageNum, char **ppBuffer);
  StatusCode allocatePage(char *ipAddr, char *port, char *fileName, int pageNum, char **ppBuffer);
  StatusCode insertAtHead(int slot);
private:
  StatusCode linkHead(int slot);
  StatusCode unlink(int slot);
  StatusCode internalAlloc(int &slot);
  StatusCode writePage(int fd, int pageNum, char *source);
  StatusCode writePage(char *ipAddr, char *port, char *fileName, int pageNum, char *source);
  StatusCode readPage(int fd, int pageNum, char *dest);
  StatusCode readPage(char *ipAddr, char *port, char *fileName, int pageNum, char *dest);
  StatusCode initPageDesc(int fd, int pageNum, int slot);
  StatusCode initPageDesc(char *ipAddr, char *port, char *fileName, int pageNum, int slot);
  map<pair<int, int>, int> loc_slot_map; // map<(file_descriptor, pageNumber), slot_number_in_buffer)>s
  map<pair<pair<string, string>, pair<string,int> >, int> rem_slot_map;
  DS_BufPageDesc *bufTable;
  DS_RemoteManager *rm;
  int numPages;
  int pageSize;
  int first;
  int last;
  int free;
};

class DS_PageHandle {
  friend class DS_FileHandle;
public:
   DS_PageHandle();
   DS_PageHandle(DS_PageHandle &pageHandle);
   ~DS_PageHandle();

   StatusCode getData(char* &pData);
   StatusCode getPageNum(int &pageNum);
private:
   int pageNumber;
   char *pageData;
};

class DS_FileHandle {
public:
  DS_FileHandle();
  ~DS_FileHandle();

  StatusCode getFirstPage(DS_PageHandle &pageHandle);
  StatusCode getNextPage(int pageNum, DS_PageHandle &pageHandle);
  StatusCode getPrevPage(int pageNum, DS_PageHandle &pageHandle);
  StatusCode getThisPage(int pageNum, DS_PageHandle &pageHandle);
  StatusCode getLastPage(DS_PageHandle &pageHandle);
  StatusCode allocatePage(DS_PageHandle &pageHandle);
  StatusCode markDirty(int pageNum);
  StatusCode unpinPage(int pageNum);
  bool isRemote;
// private:
  DS_BufferManager *bm;
  DS_RemoteManager *rm;
  DS_FileHeader hdr;
  bool isHdrChanged;
  int unixfd;
  string fileName;
  string ipaddr;
  string port;
};

class DS_Manager {
  friend class DS_FileHandle;
  friend class DS_BufferManager;
public:
  DS_Manager();
  ~DS_Manager();
  StatusCode createFile(char *filename);
  StatusCode loadFile(char *filename, DS_FileHandle &fileHandle);
// private:
  DS_BufferManager *bm;
  DS_RemoteManager *rm;
};