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
  #define ZM_API 
#endif

#include "zmBase/zmBase.h"
#include "zmStream/zmStream.h"

#if defined(__cplusplus)
extern "C" {
    namespace ZM{
#endif /* __cplusplus */ 

typedef void(*ioDataCBack)(size_t isz, const char* inMess, size_t* osz, char** outMess);

ZM_API bool zmStartAVideoServer(const char* addr,
                                        int port, 
                                    ZM::zmStream);


ZM_API bool zmStartDataServer(const char* addr,
                                      int port,
                                   ioDataCBack);

ZM_API void zmStopServers();


ZM_API void zmSetStatusCBack(ZM::zmStatusCBack, ZM::zmUData = nullptr);


#if defined(__cplusplus)
}}
#endif /* __cplusplus */

#endif /* ZMSRV_API_C_API_H_ */