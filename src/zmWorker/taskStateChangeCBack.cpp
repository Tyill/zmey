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
#include <vector>
#include "zmBase/structurs.h"
#include "zmCommon/queue.h"
#include "zmCommon/tcp.h" 
#include "structurs.h" 

using namespace std;

extern ZM_Aux::QueueThrSave<message> _messToSchedr;

void taskStateChangeCBack(uint64_t taskdId, ZM_Base::stateType newState, const std::string& result){
  
  ZM_Base::messType mType;
  switch (newState){
    case ZM_Base::stateType::start:     mType = ZM_Base::messType::taskStart; break;
    case ZM_Base::stateType::running:   mType = ZM_Base::messType::taskRunning; break;
    case ZM_Base::stateType::stop:      mType = ZM_Base::messType::taskStop; break;
    case ZM_Base::stateType::pause:     mType = ZM_Base::messType::taskPause; break;
    case ZM_Base::stateType::error:     mType = ZM_Base::messType::taskError; break; 
    case ZM_Base::stateType::completed: mType = ZM_Base::messType::taskCompleted; break; 
    default: return;
  }
  _messToSchedr.push(message{taskdId, mType, result});
}