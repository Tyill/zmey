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

#include "zmScheduler/executor.h"

#include <cmath>

using namespace std;

void Executor::checkStatusWorkers()
{
  vector<SWorker*> wkrNotResp;
  for(auto& w : m_workers){
    if (!w.second.isActive){            
      wkrNotResp.push_back(&w.second);
    }else{
      w.second.isActive = false;
    }
  }
  if (wkrNotResp.size() < round(m_workers.size() * 0.75)){ 
    for(auto w : wkrNotResp){
      if (w->base.state != ZM_Base::StateType::NOT_RESPONDING){
        m_messToDB.push(ZM_DB::MessSchedr(ZM_Base::MessType::WORKER_NOT_RESPONDING, w->base.id));
        m_messToDB.push(ZM_DB::MessSchedr::errorMess(w->base.id, "schedr::checkStatusWorkers worker not responding"));          
        w->stateMem = w->base.state;
        w->base.state = ZM_Base::StateType::NOT_RESPONDING;

        for(auto& t : w->taskList)
          t = 0;
      } 
    }
  }else{
    string mess = "schedr::checkStatusWorkers error all workers are not available";
    m_messToDB.push(ZM_DB::MessSchedr::errorMess(0, mess));                                     
    m_app.statusMess(mess);
  }
}