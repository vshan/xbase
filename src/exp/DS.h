#include <iostream>
#include <ofstream>
#include <ifstream>
#include <boost/filesystem.hpp>
#include <boost/asio.hpp>
#include <string>

using namespace std;

typedef int StatusCode;

struct DS_FileHeader {
  int firstFree;
  int numPages;
};

struct ProtocolParseObj
{
  int code;
  int pageNum;
  char fileName[4096];
  char pageContents[4096];
  char ipAddr[20];
  char port[10];
};

class DS_Manager {
public:
  DS_Manager();
  ~DS_Manager();
  StatusCode createFile(const char *filename);
  StatusCode loadFile(const char *filename, DS_FileHandle &fileHandle);
private:
  DS_BufferManager *bm;
  DS_RemoteManager *rm;
};

class DS_FileHandle {
public:
  DS_FileHandle();
  ~DS_FileHandle();

  StatusCode getFirstPage(DS_PageHandle &pageHandle);
  StatusCode getNextPage(int pageNum, DS_PageHandle &pageHandle);
  StatusCode getThisPage(int pageNum, DS_PageHandle &pageHandle);
  StatusCode getLastPage(DS_PageHandle &pageHandle);
  StatusCode allocatePage(DS_PageHandle &pageHandle);
  StatusCode markDirty(int pageNum);
  StatusCode unpinPage(int pageNum);
private:
  DS_BufferManager *bm;
  DS_RemoteManager *rm;
  DS_FileHeader hdr;
  int unixfd;
  bool isRemote;
  string fileName;
  string ipaddr;
  string port;
};


class DS_RemoteManager {
public:
  DS_RemoteManager();
  ~DS_RemoteManager();

  StatusCode remoteLoadFile(const char *fileName,
                            DS_FileH &fileHandle);
  StatusCode getRemoteHeaderFile(const char *fileName,
                                 string &header_content);
  StatusCode sendRecvFrom(int server_code, string send_msg,
                          sstring &recv_msg);
  StatusCode rawSendRecv(char *host, char *port, 
                         char request[], char reply[]);
  boost::asio::ip::tcp::socket* enableConnection(char *host, char *port);
  size_t writeRead(tcp::socket *s, char request[], char reply[]);
  StatusCode getRemotePage(char *host, char *port, char *fileName,
                           int pageNum, char page_content[]);
  StatusCode makeProtocolMsg(int proto_type, void *value1, 
                             void *value2, void *value3, char msg[]);
  StatusCode makeProtoHelper(int status_code, void *value1, 
                             void *value2, char msg[]);
  StatusCode parseProtocolMsg(string msg, ProtocolParseObj &ppo);
  void server(boost::asio::io_service& io_service, unsigned short port);
  void session(boost::asio::ip::tcp::socket sock);
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

class DS_BufferManager
{
public:
  DS_BufferManager(int numPages);
  ~DS_BufferManager();

  StatusCode getPage(int fd, int pageNum, char **ppBuffer);
  StatusCode getPage(char *ipAddr, char *port, char *fileName, int pageNum, char **ppBuffer);
  StatusCode markDirty(int fd, int pageNum);
  StatusCode markDirty(char *ipAddr, char *port, char *fileName, int pageNum);
  StatusCode unpinPage(int fd, int pageNum);
  StatusCode unpinPage(char *ipAddr, char *port, char *fileName, int pageNum);
  StatusCode forcePage(int fd, int pageNum);
  StatusCode forcePage(char *ipAddr, char *port, char *fileName, int pageNum);
  StatusCode insertAtHead(int slot);
private:
  StatusCode linkHead(int slot);
  StatusCode unlink(int slot);
  StatusCode internalAlloc(bool isRemote, int &slot);
  StatusCode writePage(int fd, int pageNum, char *source);
  StatusCode writePage(char *ipAddr, char *port, char *fileName, int pageNum, char *source);
  StatusCode readPage(int fd, int pageNum, char *dest);
  StatusCode readPage(char *ipAddr, char *port, char *fileName, int pageNum, char *dest);
  StatusCode initPageDesc(int fd, int pageNum, int slot);
  StatusCode initPageDesc(char *ipAddr, char *port, char *fileName, int pageNum, int slot);
  map<pair<int, int>, int> loc_slot_map; // map<(file_descriptor, pageNumber), slot_number_in_buffer)>
  map<pair<string, string>, pair<string, int>> rem_slot_map;
  DS_BufPageDesc *bufTable;
  DS_RemoteManager *rm;
  int numPages;
  int pageSize;
  int first;
  int last;
  int free;
};