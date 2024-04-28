//
// zmey Project
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

#include "../front.h"

namespace misc {

Front::Front() : oncSz_(0) {
}
bool Front::posFront(bool en, int id){
  if (oncSz_ <= id) {
    onc_.resize(id + 1, true);
    oncSz_ = id + 1;
  }
  if (!onc_[id] && en) {
    onc_[id] = true;
    return true;
  } 
  else if (!en){
    onc_[id] = false;
  }
  return false;
}
bool Front::negFront(bool en, int id){
  if (oncSz_ <= id){
    onc_.resize(id + 1, true);
    oncSz_ = id + 1;
  }
  if (!onc_[id] && !en) {
    onc_[id] = true;
    return true;
  } 
  else if (en){
    onc_[id] = false;      
  }
  return false;
}
}