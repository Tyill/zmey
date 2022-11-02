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

#include "scheduler/executor.h"
#include "common/serial.h"
#include "base/link.h"

using namespace std;

#define ERROR_MESS(mess, wId)                               \
  m_messToDB.push(DB::MessSchedr::errorMess(wId, mess)); \
  m_app.statusMess(mess);

#ifdef DEBUG
  #define checkFieldNum(field) \
    if (mess.find(field) == mess.end()){ \
      ERROR_MESS(string("sendNotifyHandler Error mess.find ") + field + " from: " + cp, wId); \
      return;  \
    } \
    if (!Aux::isNumber(mess[field])){ \
      ERROR_MESS("sendNotifyHandler Error !Aux::isNumber " + mess[field] + " from: " + cp, wId); \
      return; \
    }
  #define checkField(field) \
    if (mess.find(field) == mess.end()){  \
      ERROR_MESS(string("sendNotifyHandler Error mess.find ") + field + " from: " + cp, wId);  \
      return;  \
    }
#else
  #define checkFieldNum(field)
  #define checkField(field)
#endif

void Executor::sendNotifyHandler(const string& cp, const string& data, const std::error_code& ec)
{
  // error from worker
  auto mess = Aux::deserialn(data);
  int wId = 0;
  checkFieldNum(Link::command);
  
  if (ec && (m_workers.find(cp) != m_workers.end())){
    auto& worker = m_workers[cp];
    wId = worker.base.id;
    base::MessType mtype = base::MessType(stoi(mess[Link::command]));
    switch (mtype){
      case base::MessType::NEW_TASK:{
        checkFieldNum(Link::taskId);
        checkField(Link::params);
        checkField(Link::scriptPath);
        checkField(Link::resultPath);
        checkFieldNum(Link::averDurationSec);
        checkFieldNum(Link::maxDurationSec);
        base::Task t;
        t.id = stoull(mess[Link::taskId]);
        t.wId = wId;
        t.params = mess[Link::params];
        t.scriptPath = mess[Link::scriptPath];
        t.resultPath = mess[Link::resultPath];
        t.averDurationSec = stoi(mess[Link::averDurationSec]);
        t.maxDurationSec = stoi(mess[Link::maxDurationSec]);
        m_tasks.push(move(t));
        worker.base.activeTask = std::max(0, worker.base.activeTask - 1);

        for(auto& wt : worker.taskList){
            if (wt == t.id){
              wt = 0;
              break;
            }
          } 
        }
        break;
      default:
        ERROR_MESS("sendHandler wrong command mtype: " + mess[Link::command] + ", cp: " + cp, wId);
        break;
    }
    if (worker.base.rating > 1)
      --worker.base.rating;
    else
      ERROR_MESS("sendHandler worker not response, cp: " + cp, wId);
  }
  else {
    ERROR_MESS("sendHandler wrong receiver: " + cp, 0);
  }  
}