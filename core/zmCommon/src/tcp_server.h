//
// zmey Project
// Copyright (C) 2018 by Contributors <https://github.com/Tyill/zmey>
//
// This code is licensed under the MIT License.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files(the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
#include <asio.hpp>
#include "tcpSession.h"

class TcpServer{
public:
  TcpServer(asio::io_context& ioc, const std::string& addr, int port)
    : _acceptor(ioc, *tcp::resolver(ioc).resolve(addr, std::to_string(port)).begin()){
  #ifdef __linux__
    ioctl(_acceptor.native_handle(), FIOCLEX); //  FD_CLOEXEC
    int one = 1;
    setsockopt(_acceptor.native_handle(), SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
  #endif 
    accept();
  }
  
private:
  void accept(){
    _acceptor.async_accept(
        [this](std::error_code ec, tcp::socket socket){
          if (!ec){
            auto session = std::make_shared<TcpSession>(std::move(socket));
            if (session->isConnect()) 
              session->read();
          }
          accept();
        });
  }
  tcp::acceptor _acceptor;
};