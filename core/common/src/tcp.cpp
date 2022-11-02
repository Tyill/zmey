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
#include "tcp_server.h"
#include "../tcp.h"
#include "../misc.h"

#include <asio.hpp>

namespace Tcp{

TcpServer* pSrv = nullptr;

bool startServer(const std::string& connPnt, ReceiveDataCBack receiveDataCB, SendStatusCBack sendStatusCB,
 int innerThreadCnt, std::string& err)
{
  if (pSrv) return true;

  try{
    auto cp = misc::split(connPnt, ':');
    pSrv = new TcpServer(cp[0], stoi(cp[1]));

    pSrv->ReceiveDataCB = receiveDataCB;
    pSrv->SendStatusCB = sendStatusCB;

    pSrv->start(innerThreadCnt);
  }catch (std::exception& e){
    err = e.what();
    return false;  
  }   
  return true;  
};

void stopServer(){
  if (pSrv)
    pSrv->stop();
};

bool asyncSendData(const std::string& connPnt, const std::string& data, bool isCBackIfError)
{
  return pSrv ? pSrv->asyncSendData(connPnt, data, isCBackIfError) : false;
};

bool syncSendData(const std::string& connPnt, const std::string& data)
{  
  using namespace asio::ip;
  
  asio::io_context io;
  tcp::socket s(io);
  tcp::resolver resolver(io);

  auto cp = misc::split(connPnt, ':');
  asio::error_code ec;
  asio::connect(s, resolver.resolve(cp[0], cp[1]), ec);
  if (!ec)
    asio::write(s, asio::buffer(data.data(), data.size() + 1), ec);
  return !ec;
};

};