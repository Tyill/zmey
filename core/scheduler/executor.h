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
#include "application.h"

class Loop;

class Executor{
public:  
  Executor(Application&, db::DbProvider& db);

  void setLoop(Loop* l);
  void loopNotify();
  void loopStop();

  void addMessToDB(db::MessSchedr);
  bool appendNewTaskAvailable();
  bool isTasksEmpty();
  bool isMessToDBEmpty();
  bool getSchedrFromDB(const std::string& connPnt, db::DbProvider& db); 
  bool listenNewTask(db::DbProvider& db, bool on);
  
  void receiveHandler(const std::string& cp, const std::string& data);
  void errorNotifyHandler(const std::string& cp, const std::error_code& ec);
  void getNewTaskFromDB(db::DbProvider& db);
  void sendAllMessToDB(db::DbProvider& db);
  bool sendTaskToWorker();
  void checkStatusWorkers(db::DbProvider& db);
  void getPrevTaskFromDB(db::DbProvider& db);
  void getPrevWorkersFromDB(db::DbProvider& db);
  void pingToDB();
  void stopSchedr(db::DbProvider& db);  

  void addTaskForWorker(int wId, const base::Task&);
  void removeTaskForWorker(int wId, const base::Task&);
  std::vector<base::Task> getWorkerTasks(int wId);
  void clearWorkerTasks(int wId);
  
private: 
  void workerNotResponding(db::DbProvider& db, base::Worker*);
  void errorMessage(const std::string& mess, int wId);

  Application& m_app;
  db::DbProvider& m_db;

  std::map<std::string, base::Worker*> m_workers;       // key - worker connectPnt  
  std::map<int, std::vector<base::Task>> m_workerTasks; // key - worker id
  std::map<int, std::mutex*> m_workerLocks;
  misc::Queue<base::Task> m_tasks;
  misc::Queue<db::MessSchedr> m_messToDB;
  base::Scheduler m_schedr;

  misc::CounterTick m_ctickNewTask;
  misc::CounterTick m_ctickMessToDB;

  Loop* m_loop{};
};