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

#ifndef ZM_STREAM_API_C_API_H_
#define ZM_STREAM_API_C_API_H_

#ifdef _WIN32
#ifdef ZMSTREAM_DLL_EXPORTS
#define ZM_STREAM_API __declspec(dllexport)
#else
#define ZM_STREAM_API __declspec(dllimport)
#endif
#else
#define ZM_STREAM_API
#endif

#include "zmBase/zmBase.h"

#if defined(__cplusplus)
extern "C" {
    namespace ZM_STREAM{
#endif /* __cplusplus */

        /// object stream
        typedef void* zmStream;        
       
        /// create stream       
        /// @return object stream
        ZM_STREAM_API zmStream zmCreateStream(ZM_BASE::zmStatusCBack = nullptr, ZM_BASE::zmUData = nullptr);
        
        /// add new frame to stream
        /// @param[in] zmStream - object stream
        /// @param[in] frame - frame       
        /// @return true ok
        ZM_STREAM_API bool zmPushFrame(zmStream, ZM_BASE::zmFrame frame);

        /// get stream piece
        /// @param[in] zmStream - object stream
        /// @param[out] outPiece - out stream piece. The memory is allocated by the user
        /// @return >=0 size out piece 
        ZM_STREAM_API size_t zmGetStreamPiece(zmStream, size_t pieceSz, char* outPiece);

        /// free object stream
        /// @param[in] zmStream - object stream
        ZM_STREAM_API void zmFreeStrm(zmStream);

#if defined(__cplusplus)
    }}
#endif /* __cplusplus */

#endif /* ZM_STREAM_API_C_API_H_ */