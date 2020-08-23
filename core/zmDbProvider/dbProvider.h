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
#include <functional>
#include "zmBase/structurs.h"

namespace ZM_DB{

struct connectCng{
  std::string connectStr;
};
struct messSchedr{
  ZM_Base::messType type;
  uint64_t workerId;
  uint64_t taskId;
  int progress;
  int workerRating;
  int schedrActiveTask;
  int workerActiveTask;
  std::string result; // if type is 'error', then errorMess
};
struct messError{
  uint64_t schedrId;
  uint64_t workerId;
  std::string createTime;
  std::string message;
};
struct schedrTask{
  uint64_t qTaskId;
  ZM_Base::task base;
  std::string params; // through ','
};
struct tskState{
  uint32_t progress;
  ZM_Base::stateType state;
};
struct taskTime{
  std::string createTime;
  std::string takeInWorkTime;
  std::string startTime;
  std::string stopTime;
};

typedef void* udata;
typedef std::function<void(const char* mess, udata)> errCBack;

typedef void(*endTaskCBack)(uint64_t qtId, ZM_Base::stateType tState);

class DbProvider{  
  std::string _err;
  errCBack _errCBack = nullptr;
  udata _errUData = nullptr;
  ZM_DB::connectCng _connCng;
  void* _db = nullptr; 
  std::mutex _mtx, _mtxNotifyTask;
  std::thread _thrEndTask;
  std::map<uint64_t, endTaskCBack> _notifyEndTask;
  volatile bool _fClose = false;
public: 
  DbProvider(const connectCng& cng);
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
  connectCng getConnectCng(){
    return _connCng;
  }
  
  bool createTables();

  // for manager
  bool addUser(const ZM_Base::user& newUserCng, uint64_t& outUserId);
  bool getUserId(const std::string& name, const std::string& passw, uint64_t& outUserId);
  bool getUserCng(uint64_t userId, ZM_Base::user& cng); 
  bool changeUser(uint64_t userId, const ZM_Base::user& newCng);
  bool delUser(uint64_t userId);
  std::vector<uint64_t> getAllUsers();

  bool addSchedr(const ZM_Base::scheduler& schedl, uint64_t& outSchId);
  bool getSchedr(uint64_t sId, ZM_Base::scheduler& outCng);
  bool changeSchedr(uint64_t sId, const ZM_Base::scheduler& newCng);
  bool delSchedr(uint64_t sId);
  bool schedrState(uint64_t sId, ZM_Base::stateType& );
  std::vector<uint64_t> getAllSchedrs(ZM_Base::stateType);

  bool addWorker(const ZM_Base::worker& worker, uint64_t& outWkrId);
  bool getWorker(uint64_t wId, ZM_Base::worker& outCng);
  bool changeWorker(uint64_t wId, const ZM_Base::worker& newCng);
  bool delWorker(uint64_t wId);
  bool workerState(const std::vector<uint64_t>& wId, std::vector<ZM_Base::stateType>&);
  std::vector<uint64_t> getAllWorkers(uint64_t sId, ZM_Base::stateType);

  bool addPipeline(const ZM_Base::uPipeline& cng, uint64_t& outPPLId);
  bool getPipeline(uint64_t pplId, ZM_Base::uPipeline& cng);
  bool changePipeline(uint64_t pplId, const ZM_Base::uPipeline& newCng);
  bool delPipeline(uint64_t pplId);
  std::vector<uint64_t> getAllPipelines(uint64_t userId);

  bool addGroup(const ZM_Base::uGroup& cng, uint64_t& outGId);
  bool getGroup(uint64_t gId, ZM_Base::uGroup& cng);
  bool changeGroup(uint64_t gId, const ZM_Base::uGroup& newCng);
  bool delGroup(uint64_t gId);
  std::vector<uint64_t> getAllGroups(uint64_t pplId);

  bool addTaskTemplate(const ZM_Base::uTaskTemplate& cng, uint64_t& outTId);
  bool getTaskTemplate(uint64_t tId, ZM_Base::uTaskTemplate& outTCng);
  bool changeTaskTemplate(uint64_t tId, const ZM_Base::uTaskTemplate& newTCng);
  bool delTaskTemplate(uint64_t tId);
  std::vector<uint64_t> getAllTaskTemplates(uint64_t parent);

  bool addTask(const ZM_Base::uTask&, uint64_t& outTId);
  bool getTask(uint64_t tId, ZM_Base::uTask&);
  bool changeTask(uint64_t tId, const ZM_Base::uTask& newTCng);
  bool delTask(uint64_t tId);
  bool startTask(uint64_t tId);
  bool cancelTask(uint64_t tId);
  bool taskState(const std::vector<uint64_t>& tId, std::vector<tskState>&);
  bool taskResult(uint64_t tId, std::string&);
  bool taskTime(uint64_t tId, taskTime& out);
  std::vector<uint64_t> getAllTasks(uint64_t pplId, ZM_Base::stateType);
  bool getWorkerByTask(uint64_t tId, uint64_t& qtId, ZM_Base::worker& wcng);
  bool setEndTaskCBack(uint64_t tId, endTaskCBack cback);

  std::vector<messError> getInternErrors(uint64_t sId, uint64_t wId, uint32_t mCnt);

  // for zmSchedr
  bool getSchedr(std::string& connPnt, ZM_Base::scheduler& outSchedl);
  bool getTasksOfSchedr(uint64_t sId, std::vector<schedrTask>& out);
  bool getWorkersOfSchedr(uint64_t sId, std::vector<ZM_Base::worker>& out);
  bool getNewTasksForSchedr(uint64_t sId, int maxTaskCnt, std::vector<schedrTask>& out);
  bool sendAllMessFromSchedr(uint64_t sId, std::vector<messSchedr>& out);

  // for test
  bool delAllTables();  
};
}