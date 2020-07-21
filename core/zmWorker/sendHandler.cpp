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
#include <system_error>
#include "zmCommon/queue.h" 
#include "zmCommon/serial.h"
#include "zmCommon/auxFunc.h"
#include "structurs.h"

using namespace std;

ZM_Aux::CounterTick ctickSH;
extern bool _isSendAck;
extern ZM_Aux::QueueThrSave<mess2schedr> _messForSchedr;

void sendHandler(const string& cp, const string& data, const std::error_code& ec){
  if (!ec){
   
    auto smess = ZM_Aux::deserialn(data);

    mess2schedr mess;// = _messForSchedr.front();    
    _messForSchedr.tryPop(mess);    
    _isSendAck = true;

    statusMess("worker error deser mtype " + smess["command"] + " qtask " + smess["taskId"] + 
               " mess mtype " + to_string(int(mess.messType)) + " qtask " + to_string(int(mess.taskId)) + " workerCP " + smess["connectPnt"]);

    // if ((int(mess.messType) == stoi(smess["command"])) && (mess.taskId == stoull(smess["taskId"]))){ 
      
    // }    
    ctickSH.reset();
  }else{
    if (ctickSH(100)){
      statusMess("worker::sendHandler error send to schedr, cp " + cp);
    }
  }
}