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
#include <list>
#include <mutex>
#include "zmCommon/queue.h"
#include "process.h"
#include "structurs.h"

using namespace std;

extern mutex _mtxPrc;

void updateListTasks(ZM_Aux::QueueThrSave<wTask>& newTasks, list<Process>& procs){
  lock_guard<std::mutex> lock(_mtxPrc);

  wTask tsk;
  while(newTasks.tryPop(tsk)){
    procs.push_back(Process(tsk));
  }
  for (auto ip = procs.begin(); ip != procs.end();){
    ZM_Base::stateType tskState = ip->getTask().state;
    if ((tskState == ZM_Base::stateType::completed) ||
        (tskState == ZM_Base::stateType::error)){
      procs.erase(ip);
      ip = procs.begin();
    }else{
      ++ip;
    }
  }  
}