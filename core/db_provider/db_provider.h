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

#include "base/base.h"

namespace DB{

struct ConnectCng{
  std::string connectStr;
};
struct MessSchedr{
  Base::MessType type = Base::MessType::INTERN_ERROR;
  int workerId = 0;
  int taskId = 0;
  std::string data;
  
  MessSchedr(Base::MessType _type = Base::MessType::INTERN_ERROR, int _workerId = 0, int _taskId = 0, const std::string& _data = "") :
    type(_type),
    workerId(_workerId),
    taskId(_taskId),
    data(_data){}

  static MessSchedr errorMess(int _workerId, const std::string& _err){
    MessSchedr mess;{
      mess.type = Base::MessType::INTERN_ERROR;
      mess.workerId = _workerId;
      mess.data = _err;
    }
    return mess;
  };
};
struct MessError{
  int schedrId;
  int workerId;
  std::string createTime;
  std::string message;
};
struct TaskState{
  uint32_t progress;
  Base::StateType state;
};
struct TaskTime{
  std::string createTime;
  std::string takeInWorkTime;
  std::string startTime;
  std::string stopTime;
};

struct SchedulerState{
  Base::StateType state;
  uint32_t activeTask;
  std::string startTime;
  std::string stopTime;
  std::string pingTime;
};

struct WorkerState{
  Base::StateType state;
  uint32_t activeTask;
  uint32_t load;
  std::string startTime;
  std::string stopTime;
  std::string pingTime;
};

typedef void* UData;
typedef std::function<void(const char* mess, UData)> ErrCBack;
typedef void(*ChangeTaskStateCBack)(int qtId, int progress, Base::StateType prevState, Base::StateType newState, UData);

class DbProvider{  
public: 
  DbProvider(const ConnectCng& cng);
  ~DbProvider(); 
  DbProvider(const DbProvider& other) = delete;
  DbProvider& operator=(const DbProvider& other) = delete;
  std::string getLastError() const{
    return m_err;
  }  
  void setErrorCBack(ErrCBack ecb, UData ud){
    m_errCBack = ecb;
    m_errUData = ud;
  }
  void errorMess(const std::string& mess){
    m_err = mess;
    if (m_errCBack){
      m_errCBack(mess.c_str(), m_errUData);
    } 
  }
  ConnectCng getConnectCng(){
    return m_connCng;
  }
  
  bool createTables();
  
  bool addSchedr(const Base::Scheduler& schedl, int& outSchId);
  bool getSchedr(int sId, Base::Scheduler& outCng);
  bool changeSchedr(int sId, const Base::Scheduler& newCng);
  bool delSchedr(int sId);
  bool schedrState(int sId, SchedulerState& );
  std::vector<int> getAllSchedrs(Base::StateType);

  bool addWorker(const Base::Worker& worker, int& outWkrId);
  bool getWorker(int wId, Base::Worker& outCng);
  bool changeWorker(int wId, const Base::Worker& newCng);
  bool delWorker(int wId);
  bool workerState(const std::vector<int>& wId, std::vector<WorkerState>&);
  std::vector<int> getAllWorkers(int sId, Base::StateType);
  
  bool startTask(int schedPresetId, Base::Task& cng, int& tId);
  bool cancelTask(int tId);
  bool taskState(const std::vector<int>& tId, std::vector<TaskState>&);
  bool taskTime(int tId, TaskTime&);
  
  bool getWorkerByTask(int tId, Base::Worker& wcng);
  bool setChangeTaskStateCBack(int tId, ChangeTaskStateCBack, UData);

  std::vector<MessError> getInternErrors(int sId, int wId, uint32_t mCnt);

  // for schedr
  bool setListenNewTaskNotify(bool on);
  bool getSchedr(const std::string& connPnt, Base::Scheduler& outSchedl);
  bool getTasksById(int sId, const std::vector<int>& tasksId, std::vector<Base::Task>& out);
  bool getTasksOfSchedr(int sId, std::vector<Base::Task>& out);
  bool getTasksOfWorker(int sId, int workerId, std::vector<int>& outTasksId);
  bool getWorkersOfSchedr(int sId, std::vector<Base::Worker>& out);
  bool getNewTasksForSchedr(int sId, int maxTaskCnt, std::vector<Base::Task>& out);
  bool sendAllMessFromSchedr(int sId, std::vector<MessSchedr>& out);

  // for test
  bool delAllTables();  

private:
  std::string m_err;
  ErrCBack m_errCBack = nullptr;
  UData m_errUData = nullptr;
  DB::ConnectCng m_connCng;
  
  class Impl;
  Impl* m_impl = nullptr;
};
}