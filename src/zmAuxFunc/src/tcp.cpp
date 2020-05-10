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

#include "tcpServer.h"
#include "tcpClient.h"
#include "../tcp.h"

ZM_Tcp::dataCBack _dataCB = nullptr;
ZM_Tcp::errSendCBack _errSendCB = nullptr;

namespace ZM_Tcp{

asio::io_context ioc;
TcpServer* _pSrv = nullptr;
std::vector<std::thread> _threads;

bool startServer(const connectPoint& cp, std::string& err){

  if (_pSrv) return true;

  try{
    _pSrv = new TcpServer(ioc, cp.addr, cp.port);
  
    int thrCount = std::max<int>(1, std::thread::hardware_concurrency());
    for (int i = 0; i < thrCount; ++i){
      _threads.push_back(std::thread([&]{ ioc.run(); 
                                          ioc.reset();
      }));
    }
  }catch (std::exception& e){
    err = e.what();
    return false;  
  }   
  return true;  
};

void stopServer(){
  if (_pSrv){   
    for (auto& t : _threads){  
      ioc.stop();    
      t.join();
    }    
  }
};

void sendData(const connectPoint& cp, const std::string& data){
  std::make_shared<TcpClient>(ioc, cp.addr, cp.port)->write(data);
};

void setReceiveCBack(dataCBack cb){
  _dataCB = cb;
};

void setErrorSendCBack(errSendCBack cb){
  _errSendCB = cb;
};
};