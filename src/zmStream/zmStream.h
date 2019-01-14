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
#define ZM_API __declspec(dllexport)
#else
#define ZM_API __declspec(dllimport)
#endif
#else
#define ZM_API
#endif

#include "zmBase/zmBase.h"

#if defined(__cplusplus)
extern "C" {
    namespace ZM{
#endif /* __cplusplus */

        /// object stream
        typedef void* zmStream;        
       
        /// create stream       
        /// @return object stream
        ZM_API zmStream zmCreateStream(ZM::zmStatusCBack = nullptr, ZM::zmUData = nullptr);
        
        /// add new frame to stream
        /// @param[in] zmStream - object stream
        /// @param[in] frame - frame       
        /// @return true ok
        ZM_API bool zmPushFrame(zmStream, ZM::zmFrame frame);
       
        /// get frame from stream
        /// @param[in] zmStream - object stream       
        /// @return out frame 
        ZM_API ZM::zmFrame zmGetFrame(zmStream);

        /// add stream piece
        /// @param[in] zmStream - object stream
        /// @param[in] zmStreamPiece - stream piece        
        /// @return true ok
        ZM_API bool zmAddStreamPiece(zmStream, zmStreamPiece);

        /// get stream piece
        /// @param[in] zmStream - object stream
        /// @return out stream piece 
        ZM_API zmStreamPiece zmGetStreamPiece(zmStream);
        
        /// free object stream
        /// @param[in] zmStream - object stream
        ZM_API void zmFreeStream(zmStream);

#if defined(__cplusplus)
    }}
#endif /* __cplusplus */

#endif /* ZM_STREAM_API_C_API_H_ */