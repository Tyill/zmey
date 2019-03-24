//
// Zmey Project
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

#include <iostream>
#include <string>

#include "zmBase/zmBase.h"
#include "zmStream/zmStream.h"
#include "udpServer.h"
#include "asio.hpp"


//
//#define SRVCheck(func, mess){ int fsts = func; \
// if (fsts != 0){  errorMess(std::string(mess) + " " + std::to_string(fsts)); return; }}

using namespace ZM;
using namespace asio::ip;

udpServer::udpServer(asio::io_context& io_context, asio::ip::address_v4 addr, int port){

    pSocket_ = new udp::socket(io_context, udp::endpoint(addr, port));

    startReceive();
}

void udpServer::startReceive(){

    pSocket_->async_receive_from(
        asio::buffer(recvBuffer_), remoteEndPoint_,
        std::bind(&udpServer::handleReceive, this, std::placeholders::_1,
                                                   std::placeholders::_2));
}

void udpServer::handleReceive(const asio::error_code& error, 
                          std::size_t /*bytes_transferred*/){
    if (!error){

       /* boost::shared_ptr<std::string> message(
            new std::string(make_daytime_string()));*/

        pSocket_->async_send_to(asio::buffer(""), remoteEndPoint_,
            std::bind(&udpServer::handleSend, this, "", 
                                 std::placeholders::_1,
                                 std::placeholders::_2));

        startReceive();
    }
}

void udpServer::handleSend(std::string /*message*/,
        const asio::error_code& /*error*/,
        std::size_t /*bytes_transferred*/){


}

 

//#undef SRVCheck
