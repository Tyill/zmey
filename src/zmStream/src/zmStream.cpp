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

#include <vector>
#include <string>
#include "zmStream/zmStream.h"
#include "stream.h"

#define ZM_STRM_VERSION "1.0.1"

namespace ZM_STREAM{

    /// version lib
    /// @param[out] outVersion The memory is allocated by the user
    void zmVersionLibStrm(char* outVersion /*sz 32*/){

        if (outVersion)
            strcpy(outVersion, ZM_STRM_VERSION);
    }

    /// create stream    
    /// @return object stream
    zmStream zmCreateStream(){
      
        auto strm = new Stream();
               
        return strm;
    }

    /// get last error
    /// @param[in] zmStream - object stream
    /// @param[out] outErr - "" - ok. The memory is allocated by the user
    void zmGetLastErrorStrm(zmStream strm, char* outErr){

        if (!strm || !outErr) return;

        static_cast<Stream*>(strm)->getLastErrorStr(outErr);
    }

    /// add new frame to stream
    /// @param[in] zmStream - object stream
    /// @param[in] frame - frame       
    /// @return true ok
    bool zmPushFrame(zmStream strm, ZM_BASE::zmFrame frame){

        if (!strm) return false;

        return static_cast<Stream*>(strm)->pushFrame(frame);
    }

    /// get stream piece
    /// @param[in] zmStream - object stream
    /// @param[in] outPiece - out stream piece
    /// @return true ok
    bool zmGetStreamPiece(zmStream strm, ZM_BASE::zmStreamPiece* outPiece){

        if (!strm) return false;

        return static_cast<Stream*>(strm)->getStreamPiece(outPiece);
    }

    /// free object stream
    /// @param[in] zmStream - object stream
    void zmFreeStrm(zmStream strm){

        if (!strm) return;

        delete static_cast<Stream*>(strm);
    }

}