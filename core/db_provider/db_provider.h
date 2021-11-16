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
  uint64_t workerId = 0;
  uint64_t taskId = 0;
  std::string data;
  
  MessSchedr(Base::MessType _type = Base::MessType::INTERN_ERROR, uint64_t _workerId = 0, uint64_t _taskId = 0, const std::string& _data = "") :
    type(_type),
    workerId(_workerId),
    taskId(_taskId),
    data(_data){}

  static MessSchedr errorMess(uint64_t _workerId, const std::string& _err){
    MessSchedr mess;{
      mess.type = Base::MessType::INTERN_ERROR;
      mess.workerId = _workerId;
      mess.data = _err;
    }
    return mess;
  };
};
struct MessError{
  uint64_t schedrId;
  uint64_t workerId;
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
typedef void(*ChangeTaskStateCBack)(uint64_t qtId, int progress, Base::StateType prevState, Base::StateType newState, UData);

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
  
  bool addSchedr(const Base::Scheduler& schedl, uint64_t& outSchId);
  bool getSchedr(uint64_t sId, Base::Scheduler& outCng);
  bool changeSchedr(uint64_t sId, const Base::Scheduler& newCng);
  bool delSchedr(uint64_t sId);
  bool schedrState(uint64_t sId, SchedulerState& );
  std::vector<uint64_t> getAllSchedrs(Base::StateType);

  bool addWorker(const Base::Worker& worker, uint64_t& outWkrId);
  bool getWorker(uint64_t wId, Base::Worker& outCng);
  bool changeWorker(uint64_t wId, const Base::Worker& newCng);
  bool delWorker(uint64_t wId);
  bool workerState(const std::vector<uint64_t>& wId, std::vector<WorkerState>&);
  std::vector<uint64_t> getAllWorkers(uint64_t sId, Base::StateType);
  
  bool startTask(uint64_t schedPresetId, Base::Task& cng, uint64_t& tId);
  bool cancelTask(uint64_t tId);
  bool taskState(const std::vector<uint64_t>& tId, std::vector<TaskState>&);
  bool taskTime(uint64_t tId, TaskTime&);
  
  bool getWorkerByTask(uint64_t tId, Base::Worker& wcng);
  bool setChangeTaskStateCBack(uint64_t tId, ChangeTaskStateCBack, UData);

  std::vector<MessError> getInternErrors(uint64_t sId, uint64_t wId, uint32_t mCnt);

  // for schedr
  bool setListenNewTaskNotify(bool on);
  bool getSchedr(const std::string& connPnt, Base::Scheduler& outSchedl);
  bool getTasksById(uint64_t sId, const std::vector<uint64_t>& tasksId, std::vector<Base::Task>& out);
  bool getTasksOfSchedr(uint64_t sId, std::vector<Base::Task>& out);
  bool getTasksOfWorker(uint64_t sId, uint64_t workerId, std::vector<uint64_t>& outTasksId);
  bool getWorkersOfSchedr(uint64_t sId, std::vector<Base::Worker>& out);
  bool getNewTasksForSchedr(uint64_t sId, int maxTaskCnt, std::vector<Base::Task>& out);
  bool sendAllMessFromSchedr(uint64_t sId, std::vector<MessSchedr>& out);

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