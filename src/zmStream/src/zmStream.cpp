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

#define ZM_STM_VERSION "1.0.1"

namespace ZM{
  
    /// create stream    
    /// @return object stream
    zmStream zmCreateStream(ZM::zmStatusCBack cb, ZM::zmUData ud){
                     
        return new Stream(cb, ud);
    }
        
    /// add new frame to stream
    /// @param[in] zmStream - object stream
    /// @param[in] frame - frame       
    /// @return true ok
    bool zmPushFrame(zmStream strm, ZM::zmFrame frame){

        if (!strm) return false;

        return static_cast<Stream*>(strm)->pushFrame(frame);
    }

    /// get frame from stream
    /// @param[in] zmStream - object stream       
    /// @return out frame 
    ZM::zmFrame zmGetFrame(zmStream strm){

        if (!strm) return zmFrame();

        return static_cast<Stream*>(strm)->getFrame();
    }

    /// add stream piece
    /// @param[in] zmStream - object stream
    /// @param[in] zmStreamPiece - stream piece        
    /// @return true ok
    bool zmAddStreamPiece(zmStream strm, zmStreamPiece pc){

        if (!strm) return false;

        return static_cast<Stream*>(strm)->addStreamPiece(pc);
    }

    /// get stream piece
    /// @param[in] zmStream - object stream
    /// @return out stream piece 
    zmStreamPiece zmGetStreamPiece(zmStream strm){

        if (!strm) return zmStreamPiece();

        return static_cast<Stream*>(strm)->zmGetStreamPiece();
    }

    /// free object stream
    /// @param[in] zmStream - object stream
    void zmFreeStream(zmStream strm){

        if (!strm) return;

        delete static_cast<Stream*>(strm);
    }

}