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

#include "common/queue.h"
#include "common/aux_func.h"
#include "base/base.h"
#include "db_provider/db_provider.h"
#include "scheduler/application.h"

class Executor{
public:  
  Executor(Application&);

public:
  void addMessToDB(ZM_DB::MessSchedr);
  bool appendNewTaskAvailable();
  bool isTasksEmpty();
  bool isMessToDBEmpty();
  bool getSchedrFromDB(const std::string& connPnt, ZM_DB::DbProvider& db); 
  bool listenNewTask(ZM_DB::DbProvider& db, bool on);
  
  void receiveHandler(const std::string& cp, const std::string& data);
  void sendNotifyHandler(const std::string& cp, const std::string& data, const std::error_code& ec);
  void getNewTaskFromDB(ZM_DB::DbProvider& db);
  void sendAllMessToDB(ZM_DB::DbProvider& db);
  bool sendTaskToWorker();
  void checkStatusWorkers();
  void getPrevTaskFromDB(ZM_DB::DbProvider& db);
  void getPrevWorkersFromDB(ZM_DB::DbProvider& db);
  void pingToDB();
  void stopSchedr(ZM_DB::DbProvider& db);  
  
private:
  struct SWorker{
    ZM_Base::Worker base;
    ZM_Base::StateType stateMem;
    std::vector<uint64_t> taskList;
    bool isActive;
  };

  Application& m_app;

  std::map<std::string, SWorker> m_workers;   // key - connectPnt  
  std::vector<ZM_Base::Worker> m_workersCpy;
  std::vector<ZM_Base::Worker*> m_refWorkers;
  ZM_Aux::Queue<ZM_Base::Task> m_tasks;
  ZM_Aux::Queue<ZM_DB::MessSchedr> m_messToDB;
  ZM_Base::Scheduler m_schedr;

  ZM_Aux::CounterTick m_ctickNewTask;
  ZM_Aux::CounterTick m_ctickMessToDB;
  ZM_Aux::CounterTick m_ctickTaskToWorker;
};