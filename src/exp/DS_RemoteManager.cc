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
  // 80|PageNum|FileName
  else if (proto_type == DS_PROTO_ALLOC_PAGE)
  {
    makeProtoHelper(DS_PROTO_ALLOC_PAGE_CODE, value1, value2, msg);
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


void DS_RemoteManager::session(boost::asio::ip::tcp::socket sock)
{
  try
  {
    for (;;)
    {
      char data[max_length];

      boost::system::error_code error;
      size_t length = sock.read_some(boost::asio::buffer(data), error);
      if (error == boost::asio::error::eof)
        break; // Connection closed cleanly by peer.
      else if (error)
        throw boost::system::system_error(error); // Some other error.

      boost::asio::write(sock, boost::asio::buffer(data, length));
    }
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception in thread: " << e.what() << "\n";
  }
}

void DS_RemoteManager::server(boost::asio::io_service& io_service, unsigned short port)
{
  boost::asio::ip::tcp::acceptor a(io_service, 
    boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port));
  for (;;)
  {
    boost::asio::ip::tcp::socket sock(io_service);
    a.accept(sock);
    std::thread(session, std::move(sock)).detach();
  }
}
