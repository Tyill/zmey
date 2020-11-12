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
#pragma once

#include <asio.hpp>
#include "../tcp.h"

extern ZM_Tcp::stsSendCBack _stsSendCBack;

using namespace asio::ip;

class TcpClient : public std::enable_shared_from_this<TcpClient>{
public:
  TcpClient(asio::io_context& ioc, const std::string& connPnt)
  : _ioc(ioc), _connPnt(connPnt), _socket(ioc){
    auto cp = ZM_Aux::split(connPnt, ':');
    asio::connect(_socket, tcp::resolver(_ioc).resolve(cp[0], cp[1]), _ec);
    _isConnect = !_ec;
  }

  bool isConnect(){
    return _isConnect;     
  }

  void write(const std::string& msg, bool isCBackIfError){  
    if (!_isConnect){
      if (_stsSendCBack)
        _stsSendCBack(_connPnt, msg, _ec);
      return;
    } 
    while(!_isSendCBack){
      std::this_thread::yield();
    } 
    auto self(shared_from_this()); 
    asio::async_write(_socket, asio::buffer(msg.data(), msg.size()),
      [this, self, msg, isCBackIfError](std::error_code ec, std::size_t /*length*/){
        if (_stsSendCBack && (ec || !isCBackIfError){
          _ec = ec;
          _stsSendCBack(_connPnt, msg, ec);          
        }
        _isSendCBack = true;
      });
    );   
  }

  std::error_code errorCode(){
    return _ec;
  }
  
private:
  bool _isConnect = false;
  bool _isSendCBack = true;
  asio::io_context& _ioc;
  tcp::socket _socket;
  std::error_code _ec;
  std::string _connPnt;
};
