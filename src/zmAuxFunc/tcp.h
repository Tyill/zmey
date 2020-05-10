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

namespace ZM_Tcp{

bool startServer(int port, std::string& err);

void stopServer();

/// send data to receiver 
/// @param[in] addr - IP address of receiver 
/// @param[in] port - port of receiver 
/// @param[in] data - data for send
void sendData(const std::string& addr, int port, const std::string& data);

/// error send data to receiver 
/// @param[in] addr - IP address of receiver 
/// @param[in] port - port of receiver 
/// @param[in] data - data for send
typedef std::function<void(const std::string& addr, int port, const std::string& data)> errSendCBack;
void setErrorSendCBack(errSendCBack);

/// received data from sender
/// @param[in] addr - IP address of sender 
/// @param[in] port - port of sender 
/// @param[in] data - data from sender
typedef std::function<void(const std::string& addr, int port, const std::string& data)> dataCBack;
void setReceiveCBack(dataCBack);

}