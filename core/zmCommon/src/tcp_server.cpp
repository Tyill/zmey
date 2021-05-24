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
#include "tcp_session.h"
#include "../aux_func.h"

#include <numeric>

using namespace asio::ip;

TcpServer::TcpServer(const std::string& addr, int port)
  : m_acceptor(m_ioc, *tcp::resolver(m_ioc).resolve(addr, std::to_string(port)).begin())
{
#ifdef __linux__
  ioctl(m_acceptor.native_handle(), FIOCLEX); //  FD_CLOEXEC
  int one = 1;
  setsockopt(m_acceptor.native_handle(), SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
#endif 
  accept();
}
 
void TcpServer::accept(){
  m_acceptor.async_accept(
    [this](std::error_code ec, tcp::socket socket){
      if (!ec){
        auto session = std::make_shared<TcpSession>(*this, std::move(socket));
        if (session->isConnect()) 
          session->read();
      }
      accept();
    });
}

void TcpServer::start(int innerThreadCnt)
{
  int thrCount = (innerThreadCnt > 0) ?
                  innerThreadCnt : std::max<int>(1, std::thread::hardware_concurrency());
  
  m_isThrRun.resize(thrCount, true);
  for (int i = 0; i < thrCount; ++i){
    m_threads.push_back(std::thread([&, i]{ m_ioc.run(); 
                                            m_ioc.reset();
                                            m_isThrRun[i] = false;
    }));
  }
}

void TcpServer::stop()
{
  while(true){        
    if (std::accumulate(m_isThrRun.begin(), m_isThrRun.end(), 0)){
      m_ioc.stop();
      std::this_thread::yield();
    }
    else break;
  }
  for (auto& t : m_threads){
    if (t.joinable())
      t.join();
  }
}

bool TcpServer::asyncSendData(const std::string& connPnt, const std::string& data, bool isCBackIfError)
{
  if (!m_sessions.count(connPnt) || !m_sessions[connPnt]->isConnect()){    
    tcp::socket socket(m_ioc);
    asio::error_code ec;
    auto cp = ZM_Aux::split(connPnt, ':');
    asio::connect(socket, tcp::resolver(m_ioc).resolve(cp[0], cp[1]), ec);
    if (!ec){ 
      m_sessions[connPnt] = std::make_shared<TcpSession>(*this, connPnt, std::move(socket)); 
    }else{
      if (SendStatusCB)
        SendStatusCB(connPnt, data, ec);
      return false;
    }
  }
  m_sessions[connPnt]->write(data, isCBackIfError);  
  return true;
};
