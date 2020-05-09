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

#include <cstdlib>
#include <iostream>
#include <memory>
#include <utility>
#include <asio.hpp>
#include "../tcp.h"

extern ZM_Tcp::errSendCBack _errSendCB;

using asio::ip::tcp;

class TcpClient
  : public std::enable_shared_from_this<TcpClient>{
public:
  TcpClient(asio::io_context& ioc, const std::string& addr, int port): _socket(ioc){
    tcp::resolver resolver(ioc);
    _endpoints = resolver.resolve(addr, std::to_string(port));
  }

  void write(const std::string& msg){
    auto self(shared_from_this());
    asio::async_connect(_socket, _endpoints,
    [this, self, msg](std::error_code ec, tcp::endpoint ep){
      if (!ec){
        asio::async_write(_socket, asio::buffer(msg.data(), msg.size()),
          [this, self](std::error_code ec, std::size_t /*length*/){
            if (ec && _errSendCB) _errSendCB(""); 
          });
      }else{
        if (_errSendCB) _errSendCB(""); 
      }
    });
  }
  
private:
  tcp::socket _socket;
  tcp::resolver::results_type _endpoints;
};
