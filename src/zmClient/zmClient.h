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

#pragma once

#ifdef _WIN32
  #ifdef ZMCLNT_DLL_EXPORTS
    #define ZM_CLNT_API extern "C" __declspec(dllexport)
  #else
    #define ZM_CLNT_API extern "C" __declspec(dllimport)
  #endif
#else
  #define ZM_CLNT_API extern "C"
#endif

#include "zmFrame/zmFrame.h"

namespace ZM{

    /// object client
    typedef void* zmClient;

    /// create client       
    /// @return object client
    ZM_API zmClient zmCreateClient(const char *clientName,
                                   const char *ipAddrServ,
                                   int portServ,
                                   ZM::zmStatusCBack = nullptr,
                                   ZM::zmUData = nullptr);
    
    // set frame
    ZM_API bool zmPushFrame(zmClient, ZM::zmFrame);
       
}