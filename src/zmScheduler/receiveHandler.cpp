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
#include "zmBase/structurs.h"
#include "zmCommon/serial.h"
#include "zmCommon/queue.h"
#include "zmDbProvider/dbProvider.h"
#include "zmCommon/tcp.h"
#include "zmCommon/auxFunc.h"
#include "stdafx.h"

using namespace std;

extern ZM_Aux::QueueThrSave<ZM_DB::messSchedr> _messToDB;
extern unordered_map<std::string, ZM_Base::worker> _workers;
extern ZM_Base::scheduler _schedr;

void receiveHandler(const string& cp, const string& data){
 
  auto mess = ZM_Aux::deserialn(data);
  if (mess.empty()){
    statusMess("receiveHandler Error deserialn data from: " + cp);
    return;
  }

#define checkFieldNum(field) \
  if ((mess.find("field") == mess.end()) || !ZM_Aux::isNumber(mess["field"])){  \
    statusMess("receiveHandler Error mess.find(field) == mess.end()) || !ZM_Aux::isNumber(mess[field]) from: " + cp);  \
    return;  \
  } 
#define checkField(field) \
  if (mess.find("field") == mess.end()){  \
    statusMess("receiveHandler Error mess.find(field) == mess.end() from: " + cp);  \
    return;  \
  } 

  checkFieldNum(command);
  ZM_Base::messType mtype = ZM_Base::messType(stoi(mess["command"]));
  // from worker
  if(_workers.find(cp) != _workers.end()){
    switch (mtype){
      case ZM_Base::messType::taskError:
      case ZM_Base::messType::taskCompleted: 
      case ZM_Base::messType::taskRunning:        
      case ZM_Base::messType::taskPause:
      case ZM_Base::messType::taskStart:
      case ZM_Base::messType::taskStop:
        checkFieldNum(taskId);
        checkFieldNum(activeTask);
        checkField(taskResult);
        _workers[cp].activeTask = stoi(mess["activeTask"]); 
        _messToDB.push(ZM_DB::messSchedr{mtype, _workers[cp].id,
                                                stoull(mess["taskId"]),
                                                _workers[cp].activeTask,
                                                _schedr.activeTask, 
                                                0,
                                                mess["taskResult"]});
        break;
      case ZM_Base::messType::justStartWorker:
        _workers[cp].state = ZM_Base::stateType::running;
        _workers[cp].activeTask = 0;
        _messToDB.push(ZM_DB::messSchedr{mtype, _workers[cp].id});
        break;
      case ZM_Base::messType::progress:{
        int tCnt = 0;
        while(mess.find("taskId" + to_string(tCnt)) != mess.end()){
          _messToDB.push(ZM_DB::messSchedr{mtype, _workers[cp].id,
                                           stoull(mess["taskId" + to_string(tCnt)]),
                                           _workers[cp].activeTask,
                                           _schedr.activeTask,
                                           stoi(mess["progress" + to_string(tCnt)])});
          ++tCnt;
        }
        }
        break;
      case ZM_Base::messType::pingWorker:
        _workers[cp].isActive = true;
        break;
      default: statusMess("receiveHandler unknown command: " + mess["command"]);
        break;
    }    
    _workers[cp].isActive = true;
  }
  // from manager
  else{
    switch (mtype){
      case ZM_Base::messType::taskPause:
      case ZM_Base::messType::taskStart:
      case ZM_Base::messType::taskStop:{
        checkFieldNum(taskId);
        checkField(workerConnPnt);
        map<string, string> data{
          make_pair("command", to_string((int)mtype)),
          make_pair("taskId", mess["taskId"]),
          make_pair("managerConnPnt", cp),
        };      
        ZM_Tcp::sendData(mess["workerConnPnt"], ZM_Aux::serialn(data));
        }
        break;
      case ZM_Base::messType::pingSchedr:{
        checkFieldNum(schedId);
        map<string, string> data{
          make_pair("command", to_string((int)mtype)),
          make_pair("schedId", to_string(_schedr.id))
        };      
        ZM_Tcp::sendData(cp, ZM_Aux::serialn(data));
        }
        break;
      case ZM_Base::messType::pauseSchedr:
        _schedr.state = ZM_Base::stateType::pause;
        _messToDB.push(ZM_DB::messSchedr{mtype});
        break;
      case ZM_Base::messType::pauseWorker:
        checkField(workerConnPnt);
        _workers[mess["workerConnPnt"]].state = ZM_Base::stateType::pause;
        _messToDB.push(ZM_DB::messSchedr{mtype, _workers[mess["workerConnPnt"]].id});
        break;
      case ZM_Base::messType::startSchedr:
        _schedr.state = ZM_Base::stateType::running;
        _messToDB.push(ZM_DB::messSchedr{mtype});
        break;
      case ZM_Base::messType::startWorker:
        checkField(workerConnPnt);
        _workers[mess["workerConnPnt"]].state = ZM_Base::stateType::running;
         _messToDB.push(ZM_DB::messSchedr{mtype, _workers[mess["workerConnPnt"]].id});  
        break;
      default: statusMess("receiveHandler unknown command: " + mess["command"]);
        break;
    }
  }
}