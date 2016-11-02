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
  //StatusCode allocatePage(int fd, int pageNum, char **ppBuffer);
  StatusCode markDirty(int fd, int pageNum);
  StatusCode markDirty(char *ipAddr, char *port, char *fileName, int pageNum);
  StatusCode unpinPage(int fd, int pageNum);
  StatusCode unpinPage(char *ipAddr, char *port, char *fileName, int pageNum);
  //StatusCode allocateBlock(char *&buffer);
  //StatusCode disposeBlock(char *buffer);
  //StatusCode flushPages(int fd);
  // Force a page, but do not remove from buffer
  StatusCode forcePage(int fd, int pageNum);
  StatusCode forcePage(char *ipAddr, char *port, char *fileName, int pageNum);
  StatusCode insertAtHead(int slot);
  //StatusCode clearBuffer();f

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
  // StatusCode getSlotNum(int fd, int pageNum, int &slot);
  // StatusCode setSlotNum(int fd, int pageNum, int slot);
  // StatusCode getSlotNum(char *ipAddr, char *port, int pageNum, int &slot);
  // StatusCode setSlotNum(char *ipAddr, char *port, int pageNum, int &slot);
  map<pair<int, int>, int> loc_slot_map; // map<(file_descriptor, pageNumber), slot_number_in_buffer)>
  map<pair<string, string>, int> rem_slot_map;
  DS_BufPageDesc *bufTable;
  DS_RemoteManager *rm;
  int numPages;
  int pageSize;
  int first;
  int last;
  int free;
};

// We load a file
// We need to give them a FileHandle
// isRemote
// getPage(bool isRemote, )



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
  
  makeProtocolMsg(DS_PROTO_NAME_REQ, fileName, proto_msg);
  //sendrecvFrom(DS_NAME_SERVER, proto_msg, recv_msg);
  rawSendRecv("127.0.0.1", DS_NAME_SERVER_PORT, proto_msg, recv_msg);
  ProtocolParseObj ppo;
  parseProtocolMsg(string(recv_msg), ppo);
  //parse_msg(recv_msg, parse_obj);
  //header_content = parse_obj->value;
  memset(recv_msg, 0, DS_CHAR_BUF_SIZE);
  getRemotePage(ppo.ipAddr, ppo.port, fileName, 1, recv_msg);
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

StatusCode DS_RemoteManager::setRemotePage(char *host, char *port, char *fileName,
                                           int pageNum, char page_content[])
{
  char proto_msg[DS_CHAR_BUF_SIZE], reply_msg[DS_CHAR_BUF_SIZE];
  makeProtocolMsg(DS_PROTO_WRITE_PAGE, (void *)&pageNum, (void *)fileName, (void *)page_content, proto_msg);
  rawSendRecv(host, port, proto_msg, reply_msg);
  ProtocolParseObj ppo;
  parseProtocolMsg(reply_msg, ppo);
  if (ppo.code == 61)
    return DS_SUCCESS;
  else
    return DS_REMOTE_WRITE_ERROR;
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

DS_BufferManager::DS_BufferManager(int numPages, DS_RemoteManager *rem_mgr)
{
  this->numPages = numPages;
  bufTable = new DS_BufPageDesc[numPages];
  for (int i = 0; i < numPages; i++) {
    bufTable[i].pData = new char[pageSize];
    bufTable[i].ipAddr = new char[DS_SMALL_BUF_SIZE];
    bufTable[i].port = new char[DS_SMALL_BUF_SIZE];
    bufTable[i].fileName = new char[DS_SMALL_BUF_SIZE];
    memset((void *)bufTable[i].pData, 0, pageSize);
    bufTable[i].prev = i-1;
    bufTable[i].next = i+1;
  }
  rm = rem_mgr;
  bufTable[0].prev = bufTable[numPages - 1].next = INVALID_SLOT;
  free = 0;
  first = last = INVALID_SLOT;
}

DS_BufferManager::~DS_BufferManager()
{
  // Free up buffer pages and tables
  for (int i = 0; i < this->numPages; i++) {
    delete [] bufTable[i].pData;
  }
  delete [] bufTable;
}

StatusCode DS_BufferManager::getPage(int fd, int pageNum, char **ppBuffer)
{
  StatusCode sc;
  int slot;
  map<pair<int, int>, int>::iterator it;
  pair<int, int> p = make_pair(fd, pageNum);

  it = loc_slot_map.find(p);
  if (it != loc_slot_map.end()) {
    slot = it->second;
    bufTable[slot].pinCount++;
    unlink(slot);
    linkHead(slot);
  }

   // page does not exist in buffer
  else {
    if((sc = internalAlloc(slot)))
        return sc;

    if((sc = readPage(fd, pageNum, bufTable[slot].pData)))
        return sc;

    loc_slot_map.insert(make_pair(p, slot)); // make a new entry in the hashMap
    initPageDesc(fd, pageNum, slot); // initialize page description
  }

  *ppBuffer = bufTable[slot].pData;
  return DS_SUCCESS;
}

StatusCode DS_BufferManager::getPage(char *ipAddr, char *port, char *fileName, int pageNum, char **ppBuffer)
{
  StatusCode sc;
  int slot;
  map<pair<pair<string, string>, pair<string,int>>, int>::iterator it;
  pair<pair<string, string>, pair<string, int>> rp = make_pair(make_pair(string(ipAddr), string(port)), 
                                                               make_pair(string(fileName), pageNum));
  it = rem_slot_map.find(rp);

  if (it != rem_slot_map.end()) {
    slot = it->second;
    bufTable[slot].pinCount++;
    unlink(slot);
    linkHead(slot);
  }
  // page is in network
  else {
    internalAlloc(slot);
    readPage(ipAddr, port, fileName, pageNum, bufTable[slot].pData);
    rem_slot_map.insert(make_pair(rp, slot));
    initPageDesc(ipAddr, port, fileName, pageNum, slot);
  }

  *ppBuffer = bufTable[slot].pData;
  return DS_SUCCESS;
}

StatusCode DS_BufferManager::allocatePage(int fd, int pageNum, char **ppBuffer)
{
  StatusCode sc;
  int slot;

  if((ec = internalAlloc(slot)))
    return ec;

  if((ec = readPage(fd, pageNum, bufTable[slot].pData)))
    return ec;

  slot_map[make_pair(fd, pageNum)] = slot; // make a new entry in the hashMap
  initPageDesc(fd, pageNum, slot); // initialize page description
  *ppBuffer = bufTable[slot].pData;
  return (ec = 0);
}

StatusCode DS_BufferManager::allocatePage(char *ipAddr, char *port, char *fileName, int pageNum, char **ppBuffer)
{
  rm->remoteAllocatePage(ipAddr, port, fileName, pageNum);
  internalAlloc(slot);
  readPage(ipAddr, port, fileName, pageNum, bufTable[slot].pData); 
}

StatusCode DS_BufferManager::markDirty(int fd, int pageNum)
{
  StatusCode sc;
  int slot;

  map<pair<int, int>, int>::iterator it;
  pair<int, int> p = make_pair(fd, pageNum);
  it = loc_slot_map.find(p);
  if (it != loc_slot_map.end()) {
    slot = it->second;
    bufTable[slot].isDirty = TRUE;
    unlink(slot);
    linkHead(slot);
  }
  return (ec = 0);
}

StatusCode DS_BufferManager::markDirty(char *ipAddr, char *port, char *fileName, int pageNum)
{
  StatusCode sc;
  int slot;

  // pair<string, string> p = make_pair(string(ipAddr), string(port));
  map<pair<pair<string, string>, pair<string,int>>, int>::iterator it;
  pair<pair<string, string>, pair<string, int>> rp = make_pair(make_pair(string(ipAddr), string(port)), 
                                                 make_pair(string(fileName), pageNum));
  it = rem_slot_map.find(rp);
  if (it != rem_slot_map.end()) {
    slot = it->second;
    bufTable[slot].isDirty = TRUE;
    unlink(slot);
    linkHead(slot);
  }
  return (ec = 0);
}

StatusCode DS_BufferManager::unpinPage(int fd, int pageNum)
{
  StatusCode sc;
  int slot;

  map<pair<int, int>, int>::iterator it;
  pair<int, int> p = make_pair(fd, pageNum);
  it = loc_slot_map.find(p);

  if (it != loc_slot_map.end()) {
    slot = it->second;

    if (bufTable[slot].pinCount > 0)
      bufTable[slot].pinCount--;

    if (bufTable[slot].pinCount == 0) {
      unlink(slot);
      linkHead(slot);
    }
  }

  return (ec = 0);
}

StatusCode DS_BufferManager::unpinPage(char *ipAddr, char *port, char *fileName, int pageNum)
{
  StatusCode sc;
  int slot;

  map<pair<pair<string, string>, pair<string,int>>, int>::iterator it;
  pair<pair<string, string>, int> rp = make_pair(make_pair(string(ipAddr), string(port)), 
                                                 make_pair(string(fileName), pageNum));
  it = rem_slot_map.find(rp);

  if (it != rem_slot_map.end()) {
    slot = it->second;

    if (bufTable[slot].pinCount > 0)
      bufTable[slot].pinCount--;

    if (bufTable[slot].pinCount == 0) {
      unlink(slot);
      linkHead(slot);
    }
  }

  return (ec = 0);
}

StatusCode DS_BufferManager::forcePage(int fd, int pageNum)
{
  StatusCode sc;
  int slot;
  map<pair<int, int>, int>::iterator it;
  pair<int, int> p = make_pair(fd, pageNum);

  it = loc_slot_map.find(p);

  if (it != loc_slot_map.end()) {
    slot = it->second;

    if (bufTable[slot].isDirty == TRUE) {
      if((ec = writePage(fd, bufTable[slot].pageNum,
        bufTable[slot].pData)))
          return sc;

        bufTable[slot].isDirty = FALSE;
    }
  }

  return (sc = 0);
}

StatusCode DS_BufferManager::forcePage(char *ipAddr, char *port, char *fileName, int pageNum)
{
  StatusCode sc;
  int slot;
  map<pair<pair<string, string>, pair<string,int>>, int>::iterator it;
  pair<pair<string, string>, int> rp = make_pair(make_pair(string(ipAddr), string(port)), 
                                                 make_pair(string(fileName), pageNum));
  
  it = rem_slot_map.find(rp);

  if (it != rem_slot_map.end()) {
    slot = it->second;

    if (bufTable[slot].isDirty == TRUE) {
      if((sc = writePage(ipAddr, port, fileName, pageNum,
        bufTable[slot].pData)))
          return sc;

        bufTable[slot].isDirty = FALSE;
    }
  }

  return (sc = 0);
}

// Insert a slot at the head of the free list
StatusCode DS_BufferManager::insertAtHead(int slot)
{
  bufTable[slot].next = free;
  free = slot;

  return (0);
}

StatusCode DS_BufferManager::linkHead(int slot)
{
  // Set next and prev pointers of slot entry
  bufTable[slot].next = first;
  bufTable[slot].prev = INVALID_SLOT;

   // If list isn't empty, point old first back to slot
  if (first != INVALID_SLOT)
    bufTable[first].prev = slot;

  first = slot; //make slot as the head now

   // if list was empty, set last to slot/first
  if (last == INVALID_SLOT)
    last = first;

   // Return ok
  return (0);
}

/*  unlink the slot from the used list.  Assume that slot is valid.
    Set prev and next pointers to INVALID_SLOT.
    The caller is responsible to either place the unlinked page into
    the free list or the used list.
*/
StatusCode DS_BufferManager::unlink(int slot)
{
   // If slot is at head of list, set first to next element
  if (first == slot)
    first = bufTable[slot].next;

   // If slot is at end of list, set last to previous element
  if (last == slot)
    last = bufTable[slot].prev;

   // If slot not at end of list, point next back to previous
  if (bufTable[slot].next != INVALID_SLOT)
    bufTable[bufTable[slot].next].prev = bufTable[slot].prev;

   // If slot not at head of list, point prev forward to next
  if (bufTable[slot].prev != INVALID_SLOT)
    bufTable[bufTable[slot].prev].next = bufTable[slot].next;

   // Set next and prev pointers of slot entry
  bufTable[slot].prev = bufTable[slot].next = INVALID_SLOT;

   // Return ok
  return (0);
}

/*
    Allocate a buffer slot. The slot is inserted at the
    head of the used list.  Here's how it chooses which slot to use:
    If there is something on the free list, then use it.
    Otherwise, choose a victim to replace.  If a victim cannot be
    chosen (because all the pages are pinned), then return an error.
Output : slot - set to newly-allocated slot
*/
StatusCode DS_BufferManager::internalAlloc(int &slot)
{
  StatusCode sc;
  
  map<pair<int, int>, int>::iterator it;
  pair<int, int> p;
  map<pair<pair<string, string>, pair<string,int>>, int>::iterator rit;
  pair<pair<string, string>, pair<string, int>> rp;
  
  // map<pair<pair<string, string>, int>, int>::iterator rit;
  // pair<pair<string, string>, int> rp;

   // If the free list is not empty, choose a slot from the free list
  if (free != INVALID_SLOT) {
    slot = free;
    free = bufTable[slot].next;
  }
  else
  {
    // Choose the least-recently used page that is unpinned, that is why we start from last
    for (slot = last; slot != INVALID_SLOT; slot = bufTable[slot].prev)
    {
      if (bufTable[slot].pinCount == 0)
        break;
    }
    // return error if all pages are pinned
    if(slot == INVALID_SLOT)
      return DS_NOBUF;

        // write page to disk if it is dirty
    if (bufTable[slot].isDirty == TRUE) {
      if (bufTable[slot].isRemote)
        writePage(bufTable[slot].ipAddr, bufTable[slot].port,
          bufTable[slot].fileName, bufTable[slot].pageNum, bufTable[slot].pData);
      else
        writePage(bufTable[slot].fd, bufTable[slot].pageNum,
            bufTable[slot].pData);

      bufTable[slot].isDirty = FALSE;
    }

    if (isRemote) {
      rp = make_pair(make_pair(string(ipAddr), string(port)), 
                     make_pair(string(fileName), pageNum));
      rit = rem_slot_map.find(p);

      if (rit != rem_slot_map.end()) {
        rem_slot_map.erase(rp);
        unlink(slot);
      }
    }

    else {
      p = make_pair(bufTable[slot].fd, bufTable[slot].pageNum);
      it = loc_slot_map.find(p);

      if (it != loc_slot_map.end()) {
        loc_slot_map.erase(p);
        unlink(slot);
      }
    }
  }
   // link slot at the head of the used list
  linkHead(slot);

  return (sc = 0);
}

StatusCode DS_BufferManager::writePage(int fd, int pageNum, char *source)
{
  StatusCode sc;

    // seek to the appropriate place (cast to long for PC's)
  long offset = pageNum*pageSize; //+ SYSPAGE_FILE_HDR_SIZE;

  if(lseek(fd, offset, L_SET) <0)
    return DS_UNIX;

    // write the data
  int numBytes = write(fd, source, pageSize);
  if(numBytes < 0)
    return DS_UNIX;
  if(numBytes!=pageSize)
    return DS_INCOMPLETEWRITE;

  return (sc = 0);
}

StatusCode DS_BufferManager::writePage(char *ipAddr, char *port, char *fileName, int pageNum, char *source)
{
  StatusCode sc;

  sc = rm->setRemotePage(ipAddr, port, fileName, pageNum, source);

  return (sc);
}


// read a page from disk
StatusCode DS_BufferManager::readPage(int fd, int pageNum, char *dest)
{
  StatusCode sc;
  long offset = pageNum*pageSize;// + SYSPAGE_FILE_HDR_SIZE;

  if(lseek(fd, offset, L_SET) < 0)
    return DS_UNIX;

  // read the data
  int numBytes = read(fd, source, pageSize);
  if(numBytes < 0)
    return DS_UNIX;
  if(numBytes!=pageSize)
    return DS_INCOMPLETEREAD;

  return (sc = 0);
}

StatusCode DS_BufferManager::readPage(char *ipAddr, char *port, char *fileName, int pageNum, char *dest)
{
  StatusCode sc;
  sc = rm->getRemotePage(ipAddr, port, fileName, pageNum, dest);
  return sc;
}

StatusCode DS_BufferManager::initPageDesc(int fd, int pageNum, int slot)
{
  bufTable[slot].fd       = fd;
  bufTable[slot].pageNum  = pageNum;
  bufTable[slot].isDirty   = FALSE;
  bufTable[slot].pinCount = 1;
  bufTable[slot].isRemote = FALSE;
  return 0;
}

StatusCode DS_BufferManager::initPageDesc(char *ipAddr, char *port, char *fileName, int pageNum, int slot)
{
  strcpy(bufTable[slot].ipAddr, ipAddr);
  strcpy(bufTable[slot].port, port);
  strcpy(bufTable[slot].fileName, fileName);
  
  bufTable[slot].pageNum  = pageNum;
  bufTable[slot].isDirty   = FALSE;
  bufTable[slot].pinCount = 1;
  bufTable[slot].isRemote = TRUE;
  return 0;
}