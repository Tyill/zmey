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
#include "common/misc.h"
#include "base/base.h"
#include "db_provider/db_provider.h"
#include "scheduler/application.h"

class Executor{
public:  
  Executor(Application&, DB::DbProvider& db);

  void loopStandUpNotify(std::function<void()> notify);

  void addMessToDB(DB::MessSchedr);
  bool appendNewTaskAvailable();
  bool isTasksEmpty();
  bool isMessToDBEmpty();
  bool getSchedrFromDB(const std::string& connPnt, DB::DbProvider& db); 
  bool listenNewTask(DB::DbProvider& db, bool on);
  
  void receiveHandler(const std::string& cp, const std::string& data);
  void sendNotifyHandler(const std::string& cp, const std::string& data, const std::error_code& ec);
  void getNewTaskFromDB(DB::DbProvider& db);
  void sendAllMessToDB(DB::DbProvider& db);
  bool sendTaskToWorker();
  void checkStatusWorkers(DB::DbProvider& db);
  void getPrevTaskFromDB(DB::DbProvider& db);
  void getPrevWorkersFromDB(DB::DbProvider& db);
  void pingToDB();
  void stopSchedr(DB::DbProvider& db);  
  
private:
  struct SWorker{
    base::Worker base;
    base::StateType stateMem{};
    std::vector<int> taskList;
    bool isActive{};
  };

  Application& m_app;
  DB::DbProvider& m_db;

  std::map<std::string, SWorker> m_workers;   // key - connectPnt  
  std::vector<base::Worker> m_workersList;
  std::vector<base::Worker*> m_refWorkers;
  misc::Queue<base::Task> m_tasks;
  misc::Queue<DB::MessSchedr> m_messToDB;
  base::Scheduler m_schedr;

  misc::CounterTick m_ctickNewTask;
  misc::CounterTick m_ctickMessToDB;

  std::function<void()> m_loopStandUpNotify{};
};