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
#include <mutex>
#include <thread>
#include <condition_variable>
#include <functional>

#include "zmBase/structurs.h"

namespace ZM_DB{

struct ConnectCng{
  std::string connectStr;
};
struct MessSchedr{
  ZM_Base::MessType type;
  uint64_t workerId;
  uint64_t taskId;
  int progress;
  int workerRating;
  int workerLoad;
  int schedrActiveTask;
  int workerActiveTask;
  std::string result; // if type is 'error', then errorMess

  MessSchedr(ZM_Base::MessType _type = ZM_Base::MessType::INTERN_ERROR, uint64_t _workerId = 0, uint64_t _taskId = 0, int _progress = 0,
    int _workerRating = 0, int _workerLoad = 0, int _schedrActiveTask = 0, int _workerActiveTask = 0, const std::string& _result = "") :
    type(_type),
    workerId(_workerId),
    taskId(_taskId),
    progress(_progress),
    workerRating(_workerRating),
    workerLoad(_workerLoad),
    schedrActiveTask(_schedrActiveTask),
    workerActiveTask(_workerActiveTask),
    result(_result){}

  MessSchedr(ZM_Base::MessType _type,  uint64_t _workerId, const std::string& _result) :
    type(_type),
    workerId(_workerId),
    taskId(0),
    progress(0),
    workerRating(0),
    workerLoad(0),
    schedrActiveTask(0),
    workerActiveTask(0),
    result(_result){}
};
struct MessError{
  uint64_t schedrId;
  uint64_t workerId;
  std::string createTime;
  std::string message;
};
struct SchedrTask{
  uint64_t qTaskId;
  ZM_Base::Task base;
  std::string params; // through ','
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

typedef void* udata;
typedef std::function<void(const char* mess, udata)> errCBack;

typedef void(*changeTaskStateCBack)(uint64_t qtId, ZM_Base::StateType prevState, ZM_Base::StateType newState);

class DbProvider{  
  std::string _err;
  errCBack _errCBack = nullptr;
  udata _errUData = nullptr;
  ZM_DB::ConnectCng _connCng;
  void* _db = nullptr; 
  std::mutex _mtx, _mtxNotifyTask;
  std::condition_variable _cvNotifyTask;
  std::thread _thrEndTask;
  std::map<uint64_t, std::pair<ZM_Base::StateType, changeTaskStateCBack>> _notifyTaskStateCBack;
  volatile bool _fClose = false;
public: 
  DbProvider(const ConnectCng& cng);
  ~DbProvider(); 
  DbProvider(const DbProvider& other) = delete;
  DbProvider& operator=(const DbProvider& other) = delete;
  std::string getLastError() const{
    return _err;
  }  
  void setErrorCBack(errCBack ecb, udata ud){
    _errCBack = ecb;
    _errUData = ud;
  }
  void errorMess(const std::string& mess){
    _err = mess;
    if (_errCBack){
      _errCBack(mess.c_str(), _errUData);
    } 
  }
  ConnectCng getConnectCng(){
    return _connCng;
  }
  
  bool createTables();

  // for manager
  bool addUser(const ZM_Base::User& newUserCng, uint64_t& outUserId);
  bool getUserId(const std::string& name, const std::string& passw, uint64_t& outUserId);
  bool getUserCng(uint64_t userId, ZM_Base::User& cng); 
  bool changeUser(uint64_t userId, const ZM_Base::User& newCng);
  bool delUser(uint64_t userId);
  std::vector<uint64_t> getAllUsers();

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

  bool addPipeline(const ZM_Base::UPipeline& cng, uint64_t& outPPLId);
  bool getPipeline(uint64_t pplId, ZM_Base::UPipeline& cng);
  bool changePipeline(uint64_t pplId, const ZM_Base::UPipeline& newCng);
  bool delPipeline(uint64_t pplId);
  std::vector<uint64_t> getAllPipelines(uint64_t userId);

  bool addGroup(const ZM_Base::UGroup& cng, uint64_t& outGId);
  bool getGroup(uint64_t gId, ZM_Base::UGroup& cng);
  bool changeGroup(uint64_t gId, const ZM_Base::UGroup& newCng);
  bool delGroup(uint64_t gId);
  std::vector<uint64_t> getAllGroups(uint64_t pplId);

  bool addTaskTemplate(const ZM_Base::UTaskTemplate& cng, uint64_t& outTId);
  bool getTaskTemplate(uint64_t ttId, ZM_Base::UTaskTemplate& outTCng);
  bool changeTaskTemplate(uint64_t ttId, const ZM_Base::UTaskTemplate& newTCng);
  bool delTaskTemplate(uint64_t ttId);
  std::vector<uint64_t> getAllTaskTemplates(uint64_t parent);

  bool addTaskPipeline(const ZM_Base::UTaskPipeline&, uint64_t& outTId);
  bool getTaskPipeline(uint64_t ptId, ZM_Base::UTaskPipeline&);
  bool changeTaskPipeline(uint64_t ptId, const ZM_Base::UTaskPipeline& newTCng);
  bool delTaskPipeline(uint64_t ptId);
  std::vector<uint64_t> getAllTasksPipeline(uint64_t pplId);
  
  bool startTask(uint64_t ptId, const std::string& prevTasks, uint64_t& tId);
  bool cancelTask(uint64_t tId);
  bool taskState(uint64_t tId, TaskState&);
  bool taskResult(uint64_t tId, std::string&);
  bool taskTime(uint64_t tId, TaskTime&);
  
  bool getWorkerByTask(uint64_t tId, ZM_Base::Worker& wcng);
  bool setChangeTaskStateCBack(uint64_t tId, changeTaskStateCBack cback);

  std::vector<MessError> getInternErrors(uint64_t sId, uint64_t wId, uint32_t mCnt);

  // for zmSchedr
  bool getSchedr(std::string& connPnt, ZM_Base::Scheduler& outSchedl);
  bool getTasksOfSchedr(uint64_t sId, std::vector<SchedrTask>& out);
  bool getWorkersOfSchedr(uint64_t sId, std::vector<ZM_Base::Worker>& out);
  bool getNewTasksForSchedr(uint64_t sId, int maxTaskCnt, std::vector<SchedrTask>& out);
  bool sendAllMessFromSchedr(uint64_t sId, std::vector<MessSchedr>& out);

  // for test
  bool delAllTables();  
};
}