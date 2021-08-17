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

#include "zmBase/structurs.h"

namespace ZM_DB{

struct ConnectCng{
  std::string connectStr;
};
struct MessSchedr{
  ZM_Base::MessType type = ZM_Base::MessType::INTERN_ERROR;
  uint64_t workerId = 0;
  uint64_t taskId = 0;
  std::string data;
  
  MessSchedr(ZM_Base::MessType _type = ZM_Base::MessType::INTERN_ERROR, uint64_t _workerId = 0, uint64_t _taskId = 0, const std::string& _data = "") :
    type(_type),
    workerId(_workerId),
    taskId(_taskId),
    data(_data){}

  static MessSchedr errorMess(uint64_t _workerId, const std::string& _err){
    MessSchedr mess;{
      mess.type = ZM_Base::MessType::INTERN_ERROR;
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
  ZM_Base::StateType state;
};
struct TaskTime{
  std::string createTime;
  std::string takeInWorkTime;
  std::string startTime;
  std::string stopTime;
};

typedef void* UData;
typedef std::function<void(const char* mess, UData)> ErrCBack;
typedef void(*ChangeTaskStateCBack)(uint64_t qtId, ZM_Base::StateType prevState, ZM_Base::StateType newState, UData);

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
  
  bool addSchedr(const ZM_Base::Scheduler& schedl, uint64_t& outSchId);
  bool getSchedr(uint64_t sId, ZM_Base::Scheduler& outCng);
  bool changeSchedr(uint64_t sId, const ZM_Base::Scheduler& newCng);
  bool delSchedr(uint64_t sId);
  bool schedrState(uint64_t sId, ZM_Base::StateType& );
  std::vector<uint64_t> getAllSchedrs(ZM_Base::StateType);

  bool addWorker(const ZM_Base::Worker& worker, uint64_t& outWkrId);
  bool getWorker(uint64_t wId, ZM_Base::Worker& outCng);
  bool changeWorker(uint64_t wId, const ZM_Base::Worker& newCng);
  bool delWorker(uint64_t wId);
  bool workerState(const std::vector<uint64_t>& wId, std::vector<ZM_Base::StateType>&);
  std::vector<uint64_t> getAllWorkers(uint64_t sId, ZM_Base::StateType);
 
  bool addTaskTemplate(const ZM_Base::TaskTemplate& cng, uint64_t& outTId);
  bool getTaskTemplate(uint64_t ttId, ZM_Base::TaskTemplate& outTCng);
  bool changeTaskTemplate(uint64_t ttId, const ZM_Base::TaskTemplate& newTCng);
  bool delTaskTemplate(uint64_t ttId);
  std::vector<uint64_t> getAllTaskTemplates(uint64_t parent);

  bool startTask(uint64_t ttlId, uint32_t priority, const std::string& params, const std::string& prevTasks, uint64_t& tId);
  bool cancelTask(uint64_t tId);
  bool taskState(const std::vector<uint64_t>& tId, std::vector<TaskState>&);
  bool taskResult(uint64_t tId, std::string&);
  bool taskTime(uint64_t tId, TaskTime&);
  
  bool getWorkerByTask(uint64_t tId, ZM_Base::Worker& wcng);
  bool setChangeTaskStateCBack(uint64_t tId, ChangeTaskStateCBack, UData);

  std::vector<MessError> getInternErrors(uint64_t sId, uint64_t wId, uint32_t mCnt);

  // for zmSchedr
  bool getSchedr(const std::string& connPnt, ZM_Base::Scheduler& outSchedl);
  bool getTasksOfSchedr(uint64_t sId, std::vector<ZM_Base::Task>& out);
  bool getWorkersOfSchedr(uint64_t sId, std::vector<ZM_Base::Worker>& out);
  bool getNewTasksForSchedr(uint64_t sId, int maxTaskCnt, std::vector<ZM_Base::Task>& out);
  bool sendAllMessFromSchedr(uint64_t sId, std::vector<MessSchedr>& out);

  // for test
  bool delAllTables();  

private:
  std::string m_err;
  ErrCBack m_errCBack = nullptr;
  UData m_errUData = nullptr;
  ZM_DB::ConnectCng m_connCng;
  
  class Impl;
  Impl* m_impl = nullptr;
};
}