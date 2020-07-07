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
#include <unordered_map>
#include <system_error>
#include "zmCommon/serial.h"
#include "zmCommon/tcp.h"
#include "zmCommon/queue.h"
#include "zmCommon/auxFunc.h"
#include "zmDbProvider/dbProvider.h"
#include "structurs.h"

using namespace std;

extern ZM_Aux::QueueThrSave<sTask> _tasks;
extern ZM_Aux::QueueThrSave<ZM_DB::messSchedr> _messToDB;
extern unordered_map<std::string, sWorker> _workers;
extern ZM_Base::scheduler _schedr;

void sendHandler(const string& cp, const string& data, const std::error_code& ec){
#define ERROR_MESS(mess, wId)                                \
  _messToDB.push(ZM_DB::messSchedr{ZM_Base::messType::error, \
                                   wId,                      \
                                   0,                        \
                                   0,                        \
                                   0,                        \
                                   mess});                   \
  statusMess(mess);

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

  // error from worker
  auto mess = ZM_Aux::deserialn(data);
  uint64_t wId = 0;
  checkFieldNum(command);

  if (ec && (_workers.find(cp) != _workers.end())){
    wId =_workers[cp].base.id;
    ZM_Base::messType mtype = ZM_Base::messType(stoi(mess["command"]));
    switch (mtype){
      case ZM_Base::messType::newTask:{
        checkFieldNum(taskId);
        checkField(params);
        checkField(script);
        checkFieldNum(averDurationSec);
        checkFieldNum(maxDurationSec);
        sTask t;
        t.base.id = stoull(mess["taskId"]);
        t.params = mess["params"];
        t.base.script = mess["script"];
        t.base.averDurationSec = stoi(mess["averDurationSec"]);
        t.base.maxDurationSec = stoi(mess["maxDurationSec"]);
        _tasks.push(move(t));
        }
        break;
      default:
        ERROR_MESS("schedr::sendHandler wrong command mtype: " + mess["command"] + ", cp: " + cp, wId);
        break;
    }
    ERROR_MESS("schedr::sendHandler worker not response, cp: " + cp, wId);
    if (_workers[cp].base.rating > 1){
      --_workers[cp].base.rating;
      _messToDB.push(ZM_DB::messSchedr{ZM_Base::messType::workerRating,
                                       _workers[cp].base.id,
                                       0,
                                       0,
                                       _workers[cp].base.rating});      
    }    
  }
  else {
    ERROR_MESS("schedr::sendHandler wrong receiver: " + cp, 0);
  }  
}