#include <iostream>
#include <ofstream>
#include <ifstream>
#include <boost/filesystem.hpp>
#include <boost/asio.hpp>
#include <string>

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
};

DS_Manager::DS_Manager()
{
  bm = new DS_BufferManager();
  rm = new DS_RemoteManager();
}

DS_Manager::~DS_Manager()
{

}

StatusCode DS_Manager::createFile(const char *fileName)
{
  StatusCode sc;
  if (fileName == NULL) {
    sc = DS_NULL_PARAM;
    return sc;
  }
  ofstream file(fileName, ios::out | ios::binary);
  if (file.is_open()) {
    char hdr_buf = new char[DS_FILE_HDR_SIZE];
    memset(hdr_buf, 0, DS_FILE_HDR_SIZE);

    DS_FileHeader *hdr = (DS_FileHeader*) hdr_buf;
    hdr->firstFree = DS_END_FILE;
    hdr->numPages = 0;

    ofstream file(fileName, ios::out | ios::binary);
    file.write(hdr_buf, DS_FILE_HDR_SIZE);
    file.close();
    sc = DS_SUCCESS;
    return sc;
  }
  else {
    sc = DS_NO_OPEN_FILE;
    return sc;
  }
}

StatusCode DS_Manager::loadFile(const char *fileName,
                                DS_FileH &fileHandle)
{
  StatusCode sc;
  if (fileName == NULL) {
    sc = DS_NULL_PARAM;
    return sc;
  }
  boost::filesystem::path p(fileName);
  if (boost::filesystem::exists(p)) {
    fileHandle.isRemote = false;
    ifstream file(fileName, ios::in | ios::binary);
    file.read((char *)&fileHandle.hdr, DS_FILE_HDR_SIZE);
    file.close();
    fileHandle.rm = rm;
    fileHandle.bm = bm;
    sc = DS_SUCCESS;
  }
  else {
    sc = rm->remoteLoadFile(fileName, fileHandle);
  }
  fileHandle.rm = rm;
  fileHandle.bm = bm;
  return sc;
}

StatusCode DS_RemoteManager::remoteLoadFile(const char *fileName,
                                            DS_FileH &fileHandle)
{
  string file_header;
  getRemoteHeaderFile(fileName, file_header);
  fileHandle.isRemote = true;
  fileHandle.hdr = strdup(file_header.c_str());
  return DS_SUCCESS;
}

StatusCode DS_RemoteManager::getRemoteHeaderFile(const char *fileName,
                                                 string &header_content)
{
  char recv_msg[DS_CHAR_BUF_SIZE], proto_msg[DS_CHAR_BUF_SIZE];
  memset(recv_msg, 0, DS_CHAR_BUF_SIZE);
  
  makeProtocolMsg(DS_PROTO_LOAD_FILE, fileName, proto_msg);
  sendrecvFrom(DS_NAME_SERVER, proto_msg, recv_msg);
  //parse_msg(recv_msg, parse_obj);
  //header_content = parse_obj->value;
  header_content(recv_msg);
  return DS_SUCCESS;
}

StatusCode DS_RemoteManager::sendRecvFrom(int server_code, string send_msg,
                                                           string &recv_msg)
{
  boost::asio::ip::tcp::socket sock;
  pair<char *, char *> p = servers[server_code];
  rawSendRecv(p.first, p.second, send_msg, recv_msg);
  return DS_SUCCESS;
}

StatusCode DS_RemoteManager::rawSendRecv(char *host, char *port, 
                                         char request[], char reply[])
{
  boost::asio::ip::tcp::socket *s = enableConnection(host, port);
  writeRead(s, request, reply);
  return DS_SUCCESS;
}

boost::asio::ip::tcp::socket* DS_RemoteManager::enableConnection(char *host, char *port)
{
  boost::asio::io_service io_service;

  tcp::socket *s = new tcp::socket(io_service);
  tcp::resolver resolver(io_service);
  boost::asio::connect(*s, resolver.resolve({host, port}));
  return s;
}

size_t DS_RemoteManager::writeRead(tcp::socket *s, char request[], char reply[])
{
  size_t request_length = std::strlen(request);
    boost::asio::write(*s, boost::asio::buffer(request, request_length));

  size_t reply_length = boost::asio::read(*s,
        boost::asio::buffer(reply, request_length));

  return reply_length;
}

StatusCode DS_RemoteManager::getRemotePage(char *host, char *port, char *fileName,
                                           int pageNum, char page_content[])
{
  char proto_msg[DS_CHAR_BUF_SIZE], reply_msg[DS_CHAR_BUF_SIZE];
  makeProtocolMsg(DS_PROTO_LOAD_PAGE, (void *)&pageNum, (void *)fileName, NULL, proto_msg);
  rawSendRecv(host, port, proto_msg, reply_msg);
  ProtocolParseObj ppo;
  parseProtocolMsg(reply_msg, ppo);
  memcpy((void *)page_content, (void *)ppo.pageContents, strlen(ppo.pageContents));
  return DS_SUCCESS;
}

StatusCode DS_RemoteManager::makeProtocolMsg(int proto_type, void *value1, 
                                             void *value2, void *value3, char msg[])
{
  // 50|PageNum|FileName
  if (proto_type == DS_PROTO_LOAD_PAGE)
  {
    makeProtoHelper(DS_PROTO_LOAD_PAGE_CODE, value1, value2, msg);
  }
  // 60|PageNum|FileName|PageContents
  else if (proto_type == DS_PROTO_WRITE_PAGE)
  {
    makeProtoHelper(DS_PROTO_WRITE_PAGE_CODE, value1, value2, msg);
    strcat(msg, (char *)value3);
  }
  // 70|FileName
  else if (proto_type == DS_PROTO_NAME_REQ)
  {
    char itoastr[20];
    sprintf(itoastr, "%d", DS_PROTO_NAME_REQ_CODE);
    string itoas(itoastr);
    string fileN((char *)value2);
    string s = itoas + string("|") + fileN;
    memcpy((void *)msg, (void *)s.c_str(), s.size());
    msg[s.size()] = '\0'; 
  }
  return DS_SUCCESS;
}

StatusCode DS_RemoteManager::makeProtoHelper(int status_code, void *value1, 
                                             void *value2, char msg[])
{
  char itoastr[20], itoastr2[20];
  sprintf(itoastr, "%d", status_code);
  string itoas(itoastr);
  sprintf(itoastr2, "%d", *((int *)value1));
  string pageNum(itoastr2);
  string fileN((char *)value2);
  string s = itoas + string("|") + pageNum + string("|") + fileN;
  memcpy((void *)msg, (void *)s.c_str(), s.size());
  msg[s.size()] = '\0'; 
  return DS_SUCCESS;
}


StatusCode DS_RemoteManager::parseProtocolMsg(string msg, ProtocolParseObj &ppo)
{
  // 51|PageNum|FileName|PageContents
  // 61|PageNum|FileName
  // 71|Port|FileName|IP
  vector<string> strs;
  boost::split(strs,msg,boost::is_any_of("|"));

  cout << "* size of the vector: " << strs.size() << endl;    
  for (size_t i = 0; i < strs.size(); i++)
    cout << strs[i] << endl;
  
  ppo.code = std::atoi(strs[0]);
  memcpy((void *)ppo.fileName, (void *)strs[2].c_str(), strs[2].size());
  ppo.fileName[strs[2].size()] = '\0';
  
  if (code == 71) {
    memcpy((void *)ppo.port, (void *)strs[1].c_str(), strs[1].size());
    ppo.port[strs[1].size()] = '\0';
    memcpy((void *)ppo.ipAddr, (void *)strs[3].c_str(), strs[3].size());
    ppo.port[strs[3].size()] = '\0';
  }
  else { 
    ppo.pageNum = std::atoi(strs[1]);
    if (code == 51)
      memcpy((void *)ppo.pageContents, (void *)strs[3].c_str(), strs[3].size());
    else
      ppo.pageContents = NULL;
  }

  return DS_SUCCESS;
}