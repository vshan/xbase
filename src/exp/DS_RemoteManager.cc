#include <iostream>
#include <fstream>
#include <stdio.h>
#include <cstdlib>
#include <thread>
#include <utility>
#include <stdlib.h>
#include <fcntl.h>   /* For O_RDWR */
#include <unistd.h>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>                                                                                                                                                
#include <boost/asio.hpp>
#include <string>
#include "DS.h"

using namespace std;

DS_RemoteManager::DS_RemoteManager()
{

}

DS_RemoteManager::~DS_RemoteManager()
{

}

StatusCode DS_RemoteManager::getRemoteHeaderFile(char *fileName,
                                                 char recv_msg[], char ipAddrStr[], char portStr[])
{
  char proto_msg[DS_CHAR_BUF_SIZE]; //, recv_msg[DS_CHAR_BUF_SIZE]
  memset(recv_msg, 0, DS_CHAR_BUF_SIZE);
  int *n = new int;
  *n = 0;
  makeProtocolMsg(DS_PROTO_NAME_REQ, (void *)n, (void *)fileName, NULL, proto_msg);
  //sendrecvFrom(DS_NAME_SERVER, proto_msg, recv_msg);
  rawSendRecv(DS_NAME_SERVER, DS_NAME_SERVER_PORT, proto_msg, recv_msg);
  ProtocolParseObj ppo;
  parseProtocolMsg(string(recv_msg), ppo);
  //parse_msg(recv_msg, parse_obj);
  //header_content = parse_obj->value;
  memset(recv_msg, 0, DS_CHAR_BUF_SIZE);
  strcpy(ipAddrStr, ppo.ipAddr);
  ipAddrStr[strlen(ppo.ipAddr)] = '\0';
  strcpy(portStr, ppo.port);
  portStr[strlen(ppo.port)] = '\0';
  getRemotePage(ppo.ipAddr, ppo.port, fileName, 0, recv_msg);
  return DS_SUCCESS;
}

StatusCode DS_RemoteManager::createRemoteFile(char *fileName, char *ipAddr, char *port)
{
  char proto_msg[DS_CHAR_BUF_SIZE], recv_msg[DS_CHAR_BUF_SIZE];
  memset(recv_msg, 0, DS_CHAR_BUF_SIZE);
  makeProtocolMsg(DS_PROTO_CREAT_FILE, (void *)port, (void *)fileName, (void *)ipAddr, proto_msg);
  rawSendRecv(DS_NAME_SERVER, DS_NAME_SERVER_PORT, proto_msg, recv_msg);
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

  boost::asio::ip::tcp::socket *s = new boost::asio::ip::tcp::socket(io_service);
  boost::asio::ip::tcp::resolver resolver(io_service);
  boost::asio::connect(*s, resolver.resolve({host, port}));
  return s;
}

size_t DS_RemoteManager::writeRead(boost::asio::ip::tcp::socket *s, char request[], char reply[])
{
  boost::system::error_code error;
  size_t request_length = std::strlen(request);
    boost::asio::write(*s, boost::asio::buffer(request, request_length));

  size_t reply_length = s->read_some(boost::asio::buffer(reply, DS_CHAR_BUF_SIZE*10), error);

  return reply_length;
}

StatusCode DS_RemoteManager::getRemotePage(char *host, char *port, char *fileName,
                                           int pageNum, char page_content[])
{
  char proto_msg[DS_CHAR_BUF_SIZE*10], reply_msg[DS_CHAR_BUF_SIZE*10];
  memset(proto_msg, 0, DS_CHAR_BUF_SIZE*10);
  memset(reply_msg, 0, DS_CHAR_BUF_SIZE*10);
  makeProtocolMsg(DS_PROTO_LOAD_PAGE, (void *)&pageNum, (void *)fileName, NULL, proto_msg);
  rawSendRecv(host, port, proto_msg, reply_msg);
  // cout << "thoda : " << reply_msg << std::endl;
  // cout << "size thoda : " << strlen(reply_msg) << std::endl;
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
    strcat(msg, "|");
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
  // 80|PageNum|FileName
  else if (proto_type == DS_PROTO_ALLOC_PAGE)
  {
    makeProtoHelper(DS_PROTO_ALLOC_PAGE_CODE, value1, value2, msg);
  }
 
  // 90|FileName|Port|IP
  else if (proto_type == DS_PROTO_CREAT_FILE)
  {
    char itoastr[20];
    sprintf(itoastr, "%d", DS_PROTO_CREAT_FILE_CODE);
    string itoas(itoastr);
    string fileN((char *)value2);
    string s = itoas + string("|") + fileN;
    memcpy((void *)msg, (void *)s.c_str(), s.size());
    msg[s.size()] = '\0';
    strcat(msg, "|");
    strcat(msg, (char *)value1);
    strcat(msg, "|");
    strcat(msg, (char *)value3);
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
  boost::algorithm::split(strs,msg,boost::is_any_of("|"));

  cout << "* size of the vector: " << strs.size() << endl;    
  for (size_t i = 0; i < strs.size(); i++)
    cout << strs[i] << endl;
  
  ppo.code = std::atoi(strs[0].c_str());
  memcpy((void *)ppo.fileName, (void *)strs[2].c_str(), strs[2].size());
  ppo.fileName[strs[2].size()] = '\0';
  int code = ppo.code;
  if (code == 71) {
    memcpy((void *)ppo.port, (void *)strs[1].c_str(), strs[1].size());
    ppo.port[strs[1].size()] = '\0';
    memcpy((void *)ppo.ipAddr, (void *)strs[3].c_str(), strs[3].size());
    ppo.port[strs[3].size()] = '\0';
  }
  else { 
    ppo.pageNum = std::atoi(strs[1].c_str());
    if (code == 51)
      memcpy((void *)ppo.pageContents, (void *)strs[3].c_str(), strs[3].size());
  }

  return DS_SUCCESS;
}

void DS_RemoteManager::session(DS_RemoteManager *window, boost::asio::ip::tcp::socket sock)
{
  try
  {
    for (;;)
    {
      const int max_length = DS_CHAR_BUF_SIZE;
      char data[max_length];
      char reply_msg[max_length];

      boost::system::error_code error;
      size_t length = sock.read_some(boost::asio::buffer(data), error);
      if (error == boost::asio::error::eof)
        break; // Connection closed cleanly by peer.
      else if (error)
        throw boost::system::system_error(error); // Some other error.
      
      ProtocolParseObj ppo;
      window->parseIncomingMsg(data, ppo);
      window->handleProtoReq(ppo, reply_msg);
      boost::asio::write(sock, boost::asio::buffer(reply_msg, strlen(reply_msg)));
    }
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception in thread: " << e.what() << "\n";
  }
}

StatusCode DS_RemoteManager::parseIncomingMsg(char *msg, ProtocolParseObj &ppo)
{
  vector<string> strs;
  boost::algorithm::split(strs,msg, boost::is_any_of("|"));

  cout << "* size of the vector: " << strs.size() << endl;    
  for (size_t i = 0; i < strs.size(); i++)
    cout << strs[i] << endl;

  // 50|PageNum|FileName
  // 60|PageNum|FileName|PageContents
  // 70|FileName
  // 80|PageNum|FileName
  // 100|FileName

  ppo.code = std::atoi(strs[0].c_str());
  int code = ppo.code;
  if (code == 70 || code == 100)
    memcpy((void *)ppo.fileName, (void *)strs[1].c_str(), strs[1].size());
  else {
    ppo.pageNum = std::atoi(strs[1].c_str());
    memcpy((void *)ppo.fileName, (void *)strs[2].c_str(), strs[2].size());
  }

  if (code == 60)
    memcpy((void *)ppo.pageContents, (void *)strs[3].c_str(), strs[3].size());

  return DS_SUCCESS;
}

StatusCode DS_RemoteManager::handleProtoReq(ProtocolParseObj &ppo, char repbuf[])
{
  // 50|PageNum|FileName
  // 60|PageNum|FileName|PageContents
  // 70|FileName
  // 80|PageNum|FileName
  // 100|FileName

  // 51|PageNum|FileName|PageContents
  // 61|PageNum|FileName
  // 71|Port|FileName|IP
  // 81|PageNum|FileName

  char contents[DS_CHAR_BUF_SIZE];
  int fd;

  if (ppo.code == 50) {
    // open file to get fd
    fd = open(ppo.fileName, O_RDONLY);
    getLocalPage(fd, ppo.pageNum, contents);
    makeProtoHelper(DS_SUCC_REM_READ_CODE, (void *)(&ppo.pageNum), ppo.fileName, repbuf);
    strcat(repbuf, "|");
    strcat(repbuf, contents);
    close(fd);
  }

  if (ppo.code == 60) {
    // open file to get fd
    fd = open(ppo.fileName, O_RDWR);
    setLocalPage(fd, ppo.pageNum, ppo.pageContents);
    makeProtoHelper(DS_SUCC_REM_WRI_CODE, (void *)(&ppo.pageNum), (void *)ppo.fileName, repbuf);
    close(fd);
  }

  if (ppo.code == 81) {
    // allocate page..
  }

  if (ppo.code == 100) {
    char *hdr_buf = new char[DS_FILE_HDR_SIZE];
    memset(hdr_buf, 0, DS_FILE_HDR_SIZE);

    DS_FileHeader *hdr = (DS_FileHeader*) hdr_buf;
    hdr->firstFree = DS_END_FILE;
    hdr->numPages = 0;

    ofstream file(ppo.fileName, ios::out | ios::binary);
    file.write(hdr_buf, DS_FILE_HDR_SIZE);
    file.close();
  }

  return DS_SUCCESS;
}

void DS_RemoteManager::server(boost::asio::io_service& io_service, unsigned short port)
{
  boost::asio::ip::tcp::acceptor a(io_service, 
    boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port));
  for (;;)
  {
    boost::asio::ip::tcp::socket sock(io_service);
    a.accept(sock);
    DS_RemoteManager *window = this;
    std::thread(session, window, std::move(sock)).detach();
  }
}

void DS_RemoteManager::spawnServer(unsigned short port, DS_RemoteManager *window)
{
  boost::asio::io_service io_service;
  window->server(io_service, port);
}

StatusCode DS_RemoteManager::getLocalPage(int fd, int pageNum, char *dest)
{
  StatusCode sc;
  int pageSize = DS_PAGE_SIZE;
  long offset = pageNum*pageSize; //+ DS_FILE_HDR_SIZE;

  if(lseek(fd, offset, L_SET) < 0)
    return DS_UNIX;

  // read the data
  int numBytes = read(fd, dest, pageSize);
  if(numBytes < 0)
    return DS_UNIX;
  if(numBytes!=pageSize)
    return DS_INCOMPLETEREAD;

  return (sc = 0);
}

StatusCode DS_RemoteManager::setLocalPage(int fd, int pageNum, char *source)
{
  StatusCode sc;

    // seek to the appropriate place (cast to long for PC's)
  int pageSize = DS_PAGE_SIZE;
  long offset = pageNum*pageSize; //+ sizeof(DS_FileHeader);

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