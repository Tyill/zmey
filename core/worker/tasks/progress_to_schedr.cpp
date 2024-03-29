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
#include "worker/executor.h"
#include "common/tcp.h"
#include "common/serial.h"
#include "common/json.h"
#include "base/link.h"

using namespace std;

void Executor::progressToSchedr(const std::string& schedrConnPnt)
{   
  Json::Value rootJs;
  rootJs[ZM_Link::tasks];
  for(auto& p : m_procs){
    Json::Value tJs;
    tJs[ZM_Link::taskId] = p.getTask().id;
    tJs[ZM_Link::progress] = to_string(p.getProgress());    
    rootJs[ZM_Link::tasks].append(tJs);    
  }
 
  Json::FastWriter writerJs;
   
  map<string, string> data{
    {ZM_Link::command, to_string((int)ZM_Base::MessType::TASK_PROGRESS)},
    {ZM_Link::connectPnt, m_worker.connectPnt},
    {ZM_Link::tasks, writerJs.write(rootJs)},
  };  
  
  ZM_Tcp::asyncSendData(schedrConnPnt, ZM_Aux::serialn(data));
}