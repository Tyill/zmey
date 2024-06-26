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
        
#include <string>
#include <vector>
#include <map>
#include <functional>
#include <mutex>

#include "base/base.h"
#include "base/messages.h"

namespace db{

struct ConnectCng{
  std::string connectStr;
};
struct MessSchedr{
  mess::MessType type = mess::MessType::INTERN_ERROR;
  int workerId = 0;
  int taskId = 0;
  int taskProgress = 0;
  int activeTaskCount = 0;
  int workerLoad = 0;
  std::string message;
  
  MessSchedr(mess::MessType _type = mess::MessType::INTERN_ERROR, int _workerId = 0, int _taskId = 0) :
    type(_type),
    workerId(_workerId),
    taskId(_taskId){}

  static MessSchedr errorMess(int _workerId, const std::string& _err){
    MessSchedr mess;{
      mess.type = mess::MessType::INTERN_ERROR;
      mess.workerId = _workerId;
      mess.message = _err;
    }
    return mess;
  };
  static MessSchedr taskProgressMess(int _workerId, int _taskId, int _progress){
    MessSchedr mess;{
      mess.type = mess::MessType::TASK_PROGRESS;
      mess.workerId = _workerId;
      mess.taskId = _taskId;
      mess.taskProgress = _progress;
    }
    return mess;
  };
  static MessSchedr pingWorkerMess(int _workerId, int _activeTaskCount, int _workerload){
    MessSchedr mess;{
      mess.type = mess::MessType::PING_WORKER;
      mess.workerId = _workerId;
      mess.activeTaskCount = _activeTaskCount;
      mess.workerLoad = _workerload;
    }
    return mess;
  };
  static MessSchedr pingSchedrMess(int _workerId, int _activeTaskCount){
    MessSchedr mess;{
      mess.type = mess::MessType::PING_SCHEDR;
      mess.activeTaskCount = _activeTaskCount;
    }
    return mess;
  };
};
struct MessError{
  int schedrId{};
  int workerId{};
  std::string createTime;
  std::string message;
};
struct TaskState{
  base::StateType state{};
  int progress{};
};
struct TaskTime{
  std::string createTime;
  std::string takeInWorkTime;
  std::string startTime;
  std::string stopTime;
};

struct SchedulerState{
  base::StateType state{};
  int activeTaskCount{};
  std::string startTime;
  std::string stopTime;
  std::string pingTime;
};

struct WorkerState{
  base::StateType state{};
  int activeTaskCount{};
  int load{};
  std::string startTime;
  std::string stopTime;
  std::string pingTime;
};

typedef void* UData;
typedef std::function<void(const char* mess, UData)> ErrCBack;
typedef void(*ChangeTaskStateCBack)(int qtId, int progress, base::StateType prevState, base::StateType newState, UData);

class DbProvider{  
public: 
  DbProvider(const ConnectCng& cng);
  ~DbProvider(); 
  DbProvider(const DbProvider& other) = delete;
  DbProvider& operator=(const DbProvider& other) = delete;
  
  std::string getLastError();
  void setErrorCBack(ErrCBack ecb, UData ud);
  ConnectCng getConnectCng(){
    return m_connCng;
  }    
  bool addSchedr(const base::Scheduler& schedl, int& outSchId);
  bool getSchedr(int sId, base::Scheduler& outCng);
  bool changeSchedr(int sId, const base::Scheduler& newCng);
  bool delSchedr(int sId);
  bool schedrState(int sId, SchedulerState& );
  std::vector<int> getAllSchedrs(base::StateType);

  bool addWorker(const base::Worker& worker, int& outWkrId);
  bool getWorker(int wId, base::Worker& outCng);
  bool changeWorker(int wId, const base::Worker& newCng);
  bool delWorker(int wId);
  bool workerState(const std::vector<int>& wId, std::vector<WorkerState>&);
  std::vector<int> getAllWorkers(int sId, base::StateType);
  
  bool startTask(int schedPresetId, base::Task& cng, int& tId);
  bool cancelTask(int tId);
  bool taskState(const std::vector<int>& tId, std::vector<TaskState>&);
  bool taskTime(int tId, TaskTime&);
  
  bool getWorkerByTask(int tId, base::Worker& wcng);
  bool setChangeTaskStateCBack(int tId, ChangeTaskStateCBack, UData);

  std::vector<MessError> getInternErrors(int sId, int wId, int mCnt);

  // for schedr
  bool setListenNewTaskNotify(bool on);
  bool getSchedr(const std::string& connPnt, base::Scheduler& outSchedl);
  bool getTasksOfSchedr(int sId, std::vector<base::Task>& out);
  bool getTasksOfWorker(int sId, int workerId, std::vector<base::Task>& out);
  bool getWorkersOfSchedr(int sId, std::vector<base::Worker>& out);
  bool getNewTasksForSchedr(int sId, int maxTaskCnt, std::vector<base::Task>& out);
  bool sendAllMessFromSchedr(int sId, std::vector<MessSchedr>& out);
  
private:
  void errorMess(const std::string& mess);

  std::string m_err;
  ErrCBack m_errCBack = nullptr;
  UData m_errUData = nullptr;
  db::ConnectCng m_connCng;
  
  class Impl;
  Impl* m_impl = nullptr;

  std::mutex m_mtx;
};
}