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
#include "../auxFunc.h"

extern ZM_Tcp::stsSendCBack _stsSendCBack;
extern ZM_Tcp::receiveDataCBack _receiveDataCBack;

using namespace asio::ip;

class TcpSession : public std::enable_shared_from_this<TcpSession>{
public:
  TcpSession(tcp::socket socket)
    : _socket(std::move(socket)){
      auto endpoint = _socket.remote_endpoint(_ec);
      if (!_ec){
        _connPnt = endpoint.address().to_string() + ":" + std::to_string(endpoint.port());
      }
    }
  
  void read(){  
    auto self(shared_from_this());
    _socket.async_read_some(asio::buffer(_data, MAX_LENGTH),
        [this, self](std::error_code ec, std::size_t length){
          _ec = ec;
          if (!ec){
            size_t csz = _mess.size();
            _mess.resize(csz + length);
            memcpy((void*)(_mess.data() + csz), _data, length);
            if (length == MAX_LENGTH){ 
              read();
            }
          }          
          if (ec || (length < MAX_LENGTH)){
            if (_receiveDataCBack && !_mess.empty()){ 
              _receiveDataCBack(_connPnt, _mess, ec);
              _mess.clear();
            }
          }          
        });
  } 

  bool isConnect(){
    return !_ec;     
  }

  void write(const std::string& msg, bool isCBackIfError){  
    if (_ec){
      if (_stsSendCBack)
        _stsSendCBack(_connPnt, msg, _ec);
      return;
    } 
    auto self(shared_from_this()); 
    asio::async_write(_socket, asio::buffer(msg.data(), msg.size()),
      [this, self, msg, isCBackIfError](std::error_code ec, std::size_t /*length*/){
        _ec = ec;
        if (_stsSendCBack && (ec || !isCBackIfError)){
          _stsSendCBack(_connPnt, msg, ec);          
        } 
      });
  }

  std::string connectPnt() const{
    return _connPnt;
  }

  tcp::socket _socket;
  std::string _connPnt;
  enum { MAX_LENGTH = 4096 };
  char _data[MAX_LENGTH];
  std::string _mess;
  std::error_code _ec;
};