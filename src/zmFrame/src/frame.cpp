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

#include "frame.h"

void Frame::getLastErrorStr(char* outErr){

    if (outErr){
        outErr = '\0';
        strcpy(outErr, lastError_.c_str());
    }
}

bool Frame::pushStream(ZM_BASE::zmStreamPiece piece){

    strPiece_.push_back(piece);

    return true;
}

bool Frame::getFrame(ZM_BASE::zmFrame* outFrame){

    if (strPiece_.empty()) return false;

    auto piece = strPiece_.back();

    outFrame->data = (char*)realloc(outFrame->data, piece.size);

    memcpy(outFrame->data, piece.data, piece.size);

    outFrame->size = piece.size;

    strPiece_.pop_back();

    return true;
}