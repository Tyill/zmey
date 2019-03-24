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

#include "../zmServer.h"
#include "udpServer.h"

using namespace std;
using namespace asio::ip;

namespace ZM{
      
    asio::io_context io_context;
    
    tcp::socket* pTcpSocket = nullptr;

    bool zmStartAVideoServer(const char* addr,
                                     int port,
                                 ZM::zmStream){
        
        if (!pUdpSocket){

            pUdpSocket = new udp::socket(io_context,
                udp::endpoint(address_v4::from_string(addr), port));

        }
        
        //try {
                  
            udpServer server(io_context, asio::ip::address_v4::from_string(addr), port);
            
            

           // asio::thread_pool tPool;
            
           // io_context.run();
        //}
       /* catch (std::exception& e)
        {
            std::cerr << e.what() << std::endl;
        }*/

        return true;
    }


    bool zmStartDataServer(const char* addr,
                                   int port,
                                ioDataCBack){

        return true;
    }

    void zmStopServers(){

    }


    void zmSetStatusCBack(ZM::zmStatusCBack, ZM::zmUData){

    }
}