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
#include "zmCommon/serial.h"

using namespace std;

#define ERROR_MESS(mess, wId)                                                    \
  m_messToDB.push(ZM_DB::MessSchedr{ZM_Base::MessType::INTERN_ERROR, wId, mess}); \
  m_app.statusMess(mess);

#ifdef DEBUG
  #define checkFieldNum(field) \
    if (mess.find(#field) == mess.end()){ \
      ERROR_MESS(string("schedr::sendHandler Error mess.find ") + #field + " from: " + cp, wId); \
      return;  \
    } \
    if (!ZM_Aux::isNumber(mess[#field])){ \
      ERROR_MESS("schedr::sendHandler Error !ZM_Aux::isNumber " + mess[#field] + " from: " + cp, wId); \
      return; \
    }
  #define checkField(field) \
    if (mess.find(#field) == mess.end()){  \
      ERROR_MESS(string("schedr::sendHandler Error mess.find ") + #field + " from: " + cp, wId);  \
      return;  \
    }
#else
  #define checkFieldNum(field)
  #define checkField(field)
#endif

void Executor::sendNotifyHandler(const string& cp, const string& data, const std::error_code& ec)
{
  // error from worker
  auto mess = ZM_Aux::deserialn(data);
  uint64_t wId = 0;
  checkFieldNum(command);
  
  if (ec && (m_workers.find(cp) != m_workers.end())){
    auto& worker = m_workers[cp];
    wId = worker.base.id;
    ZM_Base::MessType mtype = ZM_Base::MessType(stoi(mess["command"]));
    switch (mtype){
      case ZM_Base::MessType::NEW_TASK:{
        checkFieldNum(taskId);
        checkField(params);
        checkField(script);
        checkFieldNum(averDurationSec);
        checkFieldNum(maxDurationSec);
        ZM_Base::Task t;
        t.id = stoull(mess["taskId"]);
        t.params = mess["params"];
        t.script = mess["script"];
        t.averDurationSec = stoi(mess["averDurationSec"]);
        t.maxDurationSec = stoi(mess["maxDurationSec"]);
        m_tasks.push(move(t));
        worker.base.activeTask = std::max(0, worker.base.activeTask - 1);
        }
        break;
      default:
        ERROR_MESS("schedr::sendHandler wrong command mtype: " + mess["command"] + ", cp: " + cp, wId);
        break;
    }
    ERROR_MESS("schedr::sendHandler worker not response, cp: " + cp, wId);
    if (worker.base.rating > 1){
      --worker.base.rating;
      if (worker.base.rating == 1)
        m_messToDB.push(ZM_DB::MessSchedr{ZM_Base::MessType::WORKER_RATING,
                                        worker.base.id,
                                        0,
                                        0,
                                        worker.base.rating});
    }    
  }
  else {
    ERROR_MESS("schedr::sendHandler wrong receiver: " + cp, 0);
  }  
}