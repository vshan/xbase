#include <iostream>
#include <string>
#include <boost/asio.hpp>

boost::asio::ip::tcp::socket *establishConn(std::string server_addr)
{
  // boost::asio::io_service io_service;
  // boost::asio::ip::tcp::resolver resolver(io_service);
  // boost::asio::ip::tcp::resolver::query query(server_addr, "daytime");
  // boost::asio::ip::tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
  // boost::asio::ip::tcp::socket *sock = new boost::asio::ip::tcp::socket(io_service);
  // boost::asio::connect(*sock, endpoint_iterator);
  boost::asio::io_service ios;
  boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::address::from_string(server_addr), 1301);
  boost::asio::ip::tcp::socket *sock = new boost::asio::ip::tcp::socket(ios);
  sock->connect(endpoint);
  return sock;
}

int sendTo(boost::asio::ip::tcp::socket &sock,
           std::string send_msg)
{
  boost::array<char, 128> buf;
  std::copy(send_msg.begin(),send_msg.end(),buf.begin());
  boost::system::error_code error;
  sock.write_some(boost::asio::buffer(buf, send_msg.size()), error);
  return 0;
}

int recvFrom(boost::asio::ip::tcp::socket &sock,
             std::string &recv_msg)
{
  boost::array<char, 128> buf;
  boost::system::error_code error;

  size_t len = sock.read_some(boost::asio::buffer(buf), error);

  // if (error == boost::asio::error::eof)
  //   break; // Connection closed cleanly by peer.
  // else if (error)
  //   throw boost::system::system_error(error); // Some other error.

  recv_msg.assign(buf.data());
  return 0;
}

int rawSendRecv(std::string server_addr,
                std::string send_msg,
                std::string &recv_msg)
{
  boost::asio::ip::tcp::socket *sock = establishConn(server_addr);
  sendTo(*sock, send_msg);
  recvFrom(*sock, recv_msg);
  return 0;
}



int main() {
  std::string recv_msg;
  std::string server_addr("localhost");
  std::string send_msg("chela!");
  rawSendRecv(server_addr, send_msg, recv_msg);
  std::cout << "Recevied: " << recv_msg << std::endl;
  return 0;
} 