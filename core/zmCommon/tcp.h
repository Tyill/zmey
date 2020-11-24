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

#include <string> 
#include <functional>
#include <system_error>

namespace ZM_Tcp{

/// error send data to receiver 
/// [in] connPnt - connection point: IP or DNS ':' port
/// [in] err - error
/// [in] innerThreadCnt - the number of internal threads to run, 0 - std::thread::hardware_concurrency()
/// return true - ok 
bool startServer(const std::string& connPnt, std::string& err, int innerThreadCnt = 0);

void stopServer();

/// asynchronous data sending to receiver whithout answer 
/// [in] connPnt - connection point: IP or DNS ':' port
/// [in] data - data for send
/// [in] isStsCBackIfError - status send call back only if an error
bool asyncSendData(const std::string& connPnt, const std::string& data, bool isStsCBackIfError = true);

/// synchronous data sending once to receiver whithout answer
/// [in] connPnt - connection point of receiver: IP or DNS ':' port
/// [in] data - data for send
/// return true - ok
bool syncSendData(const std::string& connPnt, const std::string& data);

/// add connect point to pool
void addPreConnectPnt(const std::string& connPnt);

/// status send data to receiver 
/// [in] connPnt - connection point: IP or DNS ':' port
/// [in] data - data for send
/// [in] ec - system error code 
typedef std::function<void(const std::string& connPnt,                           
                           const std::string& data,
                           const std::error_code& ec)> sendStatusCBack;
void setSendStatusCBack(sendStatusCBack);

/// received data from sender
/// [in] connPnt - connection point: IP or DNS ':' port
/// [in] data - data from sender
typedef std::function<void(const std::string& connPnt,
                           const std::string& data)> receiveDataCBack;
void setReceiveCBack(receiveDataCBack);

}