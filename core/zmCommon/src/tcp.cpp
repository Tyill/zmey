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
#include <numeric>
#include <mutex>

#include "tcp_server.h"
#include "../tcp.h"
#include "../aux_func.h"

std::mutex _mtxSession;
ZM_Tcp::receiveDataCBack _receiveDataCBack = nullptr;
ZM_Tcp::sendStatusCBack _sendStatusCBack = nullptr; 

std::map<std::string, std::shared_ptr<TcpSession>> _sessions;

namespace ZM_Tcp{

asio::io_context ioc;
TcpServer* _pSrv = nullptr;
std::vector<bool> _isThrRun;
std::vector<std::thread> _threads;

bool startServer(const std::string& connPnt, std::string& err, int innerThreadCnt){

  if (_pSrv) return true;

  try{
    auto cp = ZM_Aux::split(connPnt, ':');
    _pSrv = new TcpServer(ioc, cp[0], stoi(cp[1]));
  
    int thrCount = std::max<int>(1, std::thread::hardware_concurrency());
    if (innerThreadCnt > 0)
      thrCount = innerThreadCnt;
    _isThrRun.resize(thrCount, true);
    for (int i = 0; i < thrCount; ++i){
      _threads.push_back(std::thread([&, i]{ ioc.run(); 
                                             ioc.reset();
                                             _isThrRun[i] = false;
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
    while(true){        
      if (std::accumulate(_isThrRun.begin(), _isThrRun.end(), 0)){
        ioc.stop();
        std::this_thread::yield();
      }
      else break;
    }
    for (auto& t : _threads){
      if (t.joinable())
        t.join();
    }
  }
};

bool asyncSendData(const std::string& connPnt, const std::string& data, bool isCBackIfError){
  if (_sessions.find(connPnt) != _sessions.end()){
    if (!_sessions[connPnt] || !_sessions[connPnt]->isConnect()){
      tcp::socket socket(ioc);
      std::error_code ec;
      auto cp = ZM_Aux::split(connPnt, ':');
      asio::connect(socket, tcp::resolver(ioc).resolve(cp[0], cp[1]), ec);
      if (!ec){    
        std::lock_guard<std::mutex> lock(_mtxSession);
        _sessions[connPnt] = std::make_shared<TcpSession>(connPnt, std::move(socket));  
      }else{
        if (_sendStatusCBack)
          _sendStatusCBack(connPnt, data, ec);
        return false;
      }
    }
    _sessions[connPnt]->write(data, isCBackIfError);
  }
  else{
    tcp::socket socket(ioc);
    std::error_code ec;
    auto cp = ZM_Aux::split(connPnt, ':');
    asio::connect(socket, tcp::resolver(ioc).resolve(cp[0], cp[1]), ec);
    if (!ec){    
      std::make_shared<TcpSession>(connPnt, std::move(socket))->write(data, isCBackIfError);
    }else{
      if (_sendStatusCBack)
        _sendStatusCBack(connPnt, data, ec);
      return false;
    }
  } 
  return true;
};

bool syncSendData(const std::string& connPnt, const std::string& inData){  
  asio::io_context io;
  tcp::socket s(io);
  tcp::resolver resolver(io);

  auto cp = ZM_Aux::split(connPnt, ':');
  asio::error_code ec;
  asio::connect(s, resolver.resolve(cp[0], cp[1]), ec);
  if (!ec){  
    asio::write(s, asio::buffer(inData.data(), inData.size() + 1), ec);
  }
  return !ec;
};

void addPreConnectPnt(const std::string& connPnt){
  _sessions[connPnt] = nullptr;
}

void setReceiveCBack(receiveDataCBack cb){
  _receiveDataCBack = cb;
};

void setSendStatusCBack(sendStatusCBack cb){
  _sendStatusCBack = cb;
};
};