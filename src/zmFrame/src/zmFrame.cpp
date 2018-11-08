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
#include "zmBase/zmBase.h"
#include "zmFrame/zmFrame.h"
#include "frame.h"

#define ZM_FRAME_VERSION "1.0.1"

namespace ZM_FRAME{

    /// version lib
    /// @param[out] outVersion The memory is allocated by the user
    void zmVersionLibFrame(char* outVersion /*sz 32*/){

        if (outVersion)
            strcpy(outVersion, ZM_FRAME_VERSION);
    }

    /// create frame
    /// @return object frame
    zmFrame zmCreateFrame(){
      
        auto strm = new Frame();
               
        return strm;
    }
        
    /// get last error
    /// @param[in] zmFrame - object stream
    /// @param[out] outErr - "" - ok. The memory is allocated by the user
    void zmStrmGetLastErrorStr(zmFrame frm, char* outErr){

        if (!frm || !outErr) return;

        static_cast<Frame*>(frm)->getLastErrorStr(outErr);
    }

    /// add new frame to stream
    /// @param[in] zmStream - object stream
    /// @param[in] frame - frame       
    /// @return true ok
    bool zmPushStream(zmFrame frm, ZM_BASE::zmStreamPiece piece){
        
        if (!frm) return;

        static_cast<Frame*>(frm)->pushStream(piece);
    }

    /// get stream piece
    /// @param[in] zmStream - object stream
    /// @param[in] outPiece - out stream piece
    /// @return true ok
    bool zmGetFrame(zmFrame frm, ZM_BASE::zmFrame* outFrame){

        if (!frm) return;

        static_cast<Frame*>(frm)->getFrame(outFrame);
    }

    /// free object stream
    /// @param[in] zmStream - object stream
    void zmFreeFrame(zmFrame frm){

        if (!frm) return;

        delete static_cast<Frame*>(frm);
    }

   
}