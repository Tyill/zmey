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
#pragma once

#include "zmCommon/queue.h"
#include "zmCommon/aux_func.h"
#include "zmBase/structurs.h"
#include "zmWorker/application.h"
#include "zmWorker/process.h" 

#include <list>

class Executor{
public:  
  Executor(Application&);

public:
  struct MessForSchedr{
    uint64_t taskId;
    ZM_Base::MessType MessType;
    std::string taskResult;
  };

  void addMessForSchedr(MessForSchedr);
  
  void receiveHandler(const std::string& cp, const std::string& data);
  void sendNotifyHandler(const std::string& cp, const std::string& data, const std::error_code& ec);
  void messageToSchedr(const std::string& schedrConnPnt);
  void progressToSchedr(const std::string& schedrConnPnt);
  void pingToSchedr(const std::string& schedrConnPnt);
  void errorToSchedr(const std::string& schedrConnPnt);
  void updateListTasks();
  void waitProcess();
  
private:
    
  struct WTask{
    ZM_Base::Task base;
    ZM_Base::StateType state;
    std::string params; // through ','
  };

  Application& m_app;  
  
  ZM_Base::Worker m_worker;
  ZM_Aux::Queue<MessForSchedr> m_listMessForSchedr;
  ZM_Aux::Queue<WTask> m_newTasks;
  ZM_Aux::Queue<std::string> m_errMess;
  std::list<Process> m_procs;
  std::mutex m_mtxProcess;
  
  ZM_Aux::CounterTick m_ctickSendNotify;
};