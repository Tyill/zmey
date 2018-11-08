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
  #ifdef ZMNWCLNT_DLL_EXPORTS
    #define ZM_NW_CLNT_API extern "C" __declspec(dllexport)
  #else
    #define ZM_NW_CLNT_API extern "C" __declspec(dllimport)
  #endif
#else
  #define ZM_NW_CLNT_API extern "C"
#endif

namespace ZM_CLNT{

    // connect of server
    // clientName - client (leng max 24)
    // ipAddrServ - ip
    ZM_NW_CLNT_API bool zmConnect(const char *clientName, const char *ipAddrServ, int portServ);

    // disconnect of server
    ZM_NW_CLNT_API void zmDisconnect();
       
}