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
  char *recv_msg = new char[DS_CHAR_BUF_SIZE];
  memset(recv_msg, 0, DS_CHAR_BUF_SIZE);
  string proto_msg("HeyHo!");
  //makeProtocolMsg(DS_PROTO_LOAD_FILE, fileName, proto_msg);
  sendrecvFrom(DS_NAME_SERVER, proto_msg, recv_msg);
  //parse_msg(recv_msg, parse_obj);
  //header_content = parse_obj->value;
  header_content = recv_msg;
  return DS_SUCCESS;
}

StatusCode DS_RemoteManager::sendrecvFrom(int server_code, string send_msg,
                                                           string &recv_msg)
{
  boost::asio::ip::tcp::socket sock;
  string server_addr = servers[server_code];
  rawSendRecv(server_addr, send_msg, recv_msg);
  return DS_SUCCESS;
}

StatusCode DS_RemoteManager::rawSendRecv(string server_addr,
                                         string send_msg,
                                         string &recv_msg)
{
  establishConn(server_addr, sock);
  send_to(sock, send_msg);
  recv_from(sock, recv_msg);
  close_connection(sock);
  return DS_SUCCESS;
}

StatusCode DS_RemoteManager::establishConn(string server_addr,
                                           boost::asio::ip::tcp::socket &sock)
{
  boost::asio::io_service io_service;
  tcp::resolver resolver(io_service);
  tcp::resolver::query query(server_addr, "daytime");
  tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
  tcp::socket socket(io_service);
  boost::asio::connect(socket, endpoint_iterator);
  sock = socket;
  return DS_SUCCESS;
}

StatusCode DS_RemoteManager::sendTo(boost::asio::ip::tcp::socket &sock,
                                    string send_msg)
{
  boost::system::error_code ignored_error;
  boost::asio::write(socket, boost::asio::buffer(send_msg), ignored_error);
  return DS_SUCCESS;
}

StatusCode DS_RemoteManager::recvFrom(boost::asio::ip::tcp::socket &sock,
                                      string &recv_msg)
{
  boost::array<char, 128> buf;
  boost::system::error_code error;

  size_t len = socket.read_some(boost::asio::buffer(buf), error);

  if (error == boost::asio::error::eof)
    break; // Connection closed cleanly by peer.
  else if (error)
    throw boost::system::system_error(error); // Some other error.

  recv_msg.assign(buf.data());
  return DS_SUCCESS;
}