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
#include "zmBase/structurs.h"
#include "zmCommon/queue.h"
#include "process.h"

using namespace std;

void taskStateChangeCBack(uint64_t taskdId, ZM_Base::state newState, const std::string& result);

void updateListTasks(ZM_Aux::QueueThrSave<ZM_Base::task>& newTasks, vector<Process>& procs){
  
  ZM_Base::task tsk;
  while(newTasks.tryPop(tsk)){
    procs.push_back(Process(tsk, taskStateChangeCBack));
  }
  
  for (size_t i = 0; i < procs.size(); ++i){
    ZM_Base::state tskState = procs[i].getTask().ste;
    if ((tskState == ZM_Base::state::completed) ||
        (tskState == ZM_Base::state::error) || 
        (tskState == ZM_Base::state::stop)){
      procs.erase(procs.begin() + i);
      --i;
    }
  }  
}