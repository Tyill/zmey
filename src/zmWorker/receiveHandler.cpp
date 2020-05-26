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
#include <string>
#include "zmCommon/serial.h"
#include "zmCommon/auxFunc.h"
#include "zmCommon/queue.h"
#include "zmBase/structurs.h"
#include "stdafx.h"
#include "process.h" 

using namespace std;

extern vector<Process> _procs;
extern ZM_Aux::QueueThrSave<ZM_Base::task> _newTasks;

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
  switch (mtype){
    case ZM_Base::messType::newTask:
      checkFieldNum(taskId);
      checkField(params);
      checkField(script);
      checkFieldNum(averDurationSec);
      checkFieldNum(maxDurationSec);
      _newTasks.push(ZM_Base::task{stoll(mess["taskId"]),
                                   ZM_Base::state::ready,
                                   (ZM_Base::executorType)stoi(mess["exrType"]),
                                   stoi(mess["averDurationSec"]),
                                   stoi(mess["maxDurationSec"]),
                                   mess["params"],
                                   mess["script"]});
      break;
    case ZM_Base::messType::taskPause:
    case ZM_Base::messType::taskStart:
    case ZM_Base::messType::taskStop:
}