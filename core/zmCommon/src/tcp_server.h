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
#include "../tcp.h"

#include <map>
#include <asio.hpp>

class TcpSession;

class TcpServer{
public:
  TcpServer(const std::string& addr, int port);

  void start(int innerThreadCnt);
  void stop();
  bool asyncSendData(const std::string& connPnt, const std::string& data, bool isCBackIfError);
  
  ZM_Tcp::ReceiveDataCBack ReceiveDataCB = nullptr;
  ZM_Tcp::SendStatusCBack SendStatusCB = nullptr;   

private:
  void accept();
 
private:
  std::map<std::string, std::shared_ptr<TcpSession>> m_sessions;
  
  asio::io_context m_ioc;
  asio::ip::tcp::acceptor m_acceptor;

  std::vector<bool> m_isThrRun;
  std::vector<std::thread> m_threads;
};