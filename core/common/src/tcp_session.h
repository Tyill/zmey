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

class TcpServer;

class TcpSession : public std::enable_shared_from_this<TcpSession>{
public:
  
  TcpSession(TcpServer& server, const std::string& connPnt, asio::ip::tcp::socket socket);
  TcpSession(TcpServer& server, asio::ip::tcp::socket socket);

  void read(); 
  void write(const std::string& msg, bool isCBackIfError);
  bool isConnect();

private:
  TcpServer& m_server;
  asio::ip::tcp::socket m_socket;
  std::string m_connPnt;
  enum { MAX_LENGTH = 4096 };
  char m_data[MAX_LENGTH];
  struct Buffer{
    void setOffset(size_t offset){
      assert(offset <= m_csize);
      if (offset < m_csize){
        memcpy(m_buff.data(), m_buff.data() + offset, m_csize - offset);
      }
      m_csize -= offset;
    }
    size_t size()const{
      return m_csize;
    }
    void resize(size_t sz){
      m_csize = sz;
      if (m_buff.size() < sz) m_buff.resize(sz);
    }
    const char* data()const{
      return m_buff.data();
    }
  private:
    size_t m_csize = 0;
    std::string m_buff;
  };
  Buffer m_buff;
  std::error_code m_ec;
};