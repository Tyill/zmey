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

#ifndef ZMSRV_API_C_API_H_
#define ZMSRV_API_C_API_H_

#ifdef _WIN32
 #ifdef ZMSRV_DLL_EXPORTS
  #define ZM_API __declspec(dllexport)
 #else
  #define ZM_API __declspec(dllimport)
 #endif
#else
  #define ZM_SRV_API 
#endif

#include "zmBase/zmBase.h"
#include "zmStream/zmStream.h"

#if defined(__cplusplus)
extern "C" {
    namespace ZM{
#endif /* __cplusplus */ 
    
// start server
// addr - ip
// port - port
ZM_API bool zmStartServer(const char* addr,
                                int port, 
                                ZM_BASE::zmStatusCBack = nullptr, 
                                ZM_BASE::zmUData = nullptr);

// stop server
ZM_API void zmStopServer();

// set stream
ZM_API void zmSetStream(ZM_STM::zmStream);
       
#if defined(__cplusplus)
}}
#endif /* __cplusplus */

#endif /* ZMSRV_API_C_API_H_ */