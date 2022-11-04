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
#include "tcp_session.h"
#include "tcp_server.h"

using namespace asio::ip;

TcpSession::TcpSession(TcpServer& server, const std::string& connPnt, tcp::socket socket):
  m_server(server), m_connPnt(connPnt), m_socket(std::move(socket)){}

TcpSession::TcpSession(TcpServer& server, tcp::socket socket) :
  m_server(server), m_socket(std::move(socket))
{
  auto endpoint = m_socket.remote_endpoint(m_ec);
  if (!m_ec){
    m_connPnt = endpoint.address().to_string() + ":" + std::to_string(endpoint.port());
  }
}

void TcpSession::read()
{  
  auto self(shared_from_this());
  m_socket.async_read_some(asio::buffer(m_data, MAX_LENGTH),
      [this, self](std::error_code ec, std::size_t length){
        m_ec = ec;
        if (m_server.ReceiveDataCB && (length > 0)){ 
          size_t cbsz = m_buff.size();
          m_buff.resize(cbsz + length);
          memcpy((void*)(m_buff.data() + cbsz), m_data, length);           
        
          size_t buffSz = cbsz + length,
                  offs = 0; 
          auto pBuff = m_buff.data();
          while((buffSz - offs) > sizeof(int)){
            size_t messSz = *((int*)(pBuff + offs));
            if ((buffSz - offs) >= messSz){
              m_server.ReceiveDataCB(m_connPnt, std::string(pBuff + offs, messSz));  
              offs += messSz;
            }else{
              break;
            }
          }
          if (offs > 0) m_buff.setOffset(offs);
        }
        if (!ec) read();                                
      });
} 
 
void TcpSession::write(const std::string& msg, bool isCBackIfError)
{  
  auto self(shared_from_this());
  if (m_ec){
    if (m_server.SendStatusCB)
      m_server.SendStatusCB(m_connPnt, msg, m_ec);
    return;
  }    
  std::shared_ptr<std::string> pms = std::make_shared<std::string>(msg);
  
  asio::async_write(m_socket, asio::buffer(pms.get()->data(), msg.size()),
      [this, self, pms, isCBackIfError](std::error_code ec, std::size_t /*length*/) {
      m_ec = ec;
      if (m_server.SendStatusCB && (ec || !isCBackIfError)) {
          m_server.SendStatusCB(m_connPnt, *pms, ec);
      }
  });
}

bool TcpSession::isConnect(){
  return !m_ec;     
}