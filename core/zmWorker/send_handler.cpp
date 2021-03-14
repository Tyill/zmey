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

#include "zmCommon/queue.h" 
#include "zmCommon/serial.h"
#include "zmCommon/aux_func.h"
#include "structurs.h"

using namespace std;

static ZM_Aux::CounterTick m_ctickSH;
extern ZM_Aux::Queue<MessForSchedr> g_listMessForSchedr;

void sendHandler(const string& cp, const string& data, const std::error_code& ec){
  
  auto smess = ZM_Aux::deserialn(data);  
  ZM_Base::MessType messType = (ZM_Base::MessType)stoi(smess["command"]);
  if (ec && (messType != ZM_Base::MessType::PROGRESS) &&
            (messType != ZM_Base::MessType::INTERN_ERROR) &&
            (messType != ZM_Base::MessType::PING_WORKER)){
    MessForSchedr mess;
    mess.MessType = messType;
    mess.taskId = stoull(smess["taskId"]);
    mess.taskResult = smess["taskResult"];
    g_listMessForSchedr.push(move(mess));
    if (m_ctickSH(1000)){
      statusMess("worker::sendHandler error send to schedr: " + ec.message());
    }
  }
}