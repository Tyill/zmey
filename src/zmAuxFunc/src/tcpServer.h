
#include <cstdlib>
#include <memory>
#include <utility>
#include <string>
#include <asio.hpp>
#include "../tcp.h"

extern ZM_Tcp::dataCBack _dataCB;

using asio::ip::tcp;

class TcpSession
  : public std::enable_shared_from_this<TcpSession>{
public:
  TcpSession(tcp::socket socket)
    : _socket(std::move(socket)){}
  
  void read(){
    auto self(shared_from_this());
    _socket.async_read_some(asio::buffer(_data, MAX_LENGTH),
        [this, self](std::error_code ec, std::size_t length){
          if (!ec){
            _mess += _data;
            read();
          }
          else{
            if (_dataCB && !_mess.empty()) 
              _dataCB(_mess);
          }
        });
  } 
  tcp::socket _socket;
  enum { MAX_LENGTH = 8192 };
  char _data[MAX_LENGTH];
  std::string _mess;
};

class TcpServer{
public:
  TcpServer(asio::io_context& ioc, short port)
    : _acceptor(ioc, tcp::endpoint(tcp::v4(), port)){
    accept();
  }
  
private:
  void accept(){
    _acceptor.async_accept(
        [this](std::error_code ec, tcp::socket socket){
          if (!ec){
            std::make_shared<TcpSession>(std::move(socket))->read();
          }
          accept();
        });
  }
  tcp::acceptor _acceptor;
};