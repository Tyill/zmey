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

enum class dbType{
  undefined = -1,
  PostgreSQL = 0,
};
std::string dbTypeToStr(dbType);
dbType dbTypeFromStr(const std::string& dbt);

struct connectCng{
  dbType selType;
  std::string connectStr;
};
struct messSchedr{
  ZM_Base::messType type;
  uint64_t workerId;
  uint64_t taskId;
  int progress;
  std::string result;
};
struct schedrTask{
  uint64_t qTaskId;
  ZM_Base::task task;
  std::string params;
};
struct taskPrsAState{
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

class DbProvider{  
  friend DbProvider* makeDbProvider(const connectCng&);
public:  
  virtual ~DbProvider() = default; 
  DbProvider(const DbProvider& other) = delete;
  DbProvider& operator=(const DbProvider& other) = delete;
  std::string getLastError() const{
    return _err;
  }  
  virtual bool createTables() = 0;

  // for manager
  virtual bool addUser(const ZM_Base::user& newUserCng, uint64_t& outUserId) = 0;
  virtual bool getUserId(const std::string& name, const std::string& passw, uint64_t& outUserId) = 0;
  virtual bool getUserCng(uint64_t userId, ZM_Base::user& cng) = 0; 
  virtual bool changeUser(uint64_t userId, const ZM_Base::user& newCng) = 0;
  virtual bool delUser(uint64_t userId) = 0;
  virtual std::vector<uint64_t> getAllUsers() = 0;
  
  virtual bool addSchedr(const ZM_Base::scheduler& schedl, uint64_t& outSchId) = 0;
  virtual bool getSchedr(uint64_t sId, ZM_Base::scheduler& outCng) = 0;
  virtual bool changeSchedr(uint64_t sId, const ZM_Base::scheduler& newCng) = 0;
  virtual bool delSchedr(uint64_t sId) = 0;
  virtual bool schedrState(uint64_t sId, ZM_Base::stateType& ) = 0;
  virtual std::vector<uint64_t> getAllSchedrs(ZM_Base::stateType) = 0;
  
  virtual bool addWorker(const ZM_Base::worker& worker, uint64_t& outWkrId) = 0;
  virtual bool getWorker(uint64_t wId, ZM_Base::worker& outCng) = 0;
  virtual bool changeWorker(uint64_t wId, const ZM_Base::worker& newCng) = 0;
  virtual bool delWorker(uint64_t wId) = 0;
  virtual bool workerState(const std::vector<uint64_t>& wId, std::vector<ZM_Base::stateType>&) = 0;
  virtual std::vector<uint64_t> getAllWorkers(uint64_t sId, ZM_Base::stateType) = 0;

  virtual bool addPipeline(const ZM_Base::uPipeline& cng, uint64_t& outPPLId) = 0;
  virtual bool getPipeline(uint64_t pplId, ZM_Base::uPipeline& cng) = 0;
  virtual bool changePipeline(uint64_t pplId, const ZM_Base::uPipeline& newCng) = 0;
  virtual bool delPipeline(uint64_t pplId) = 0;
  virtual std::vector<uint64_t> getAllPipelines(uint64_t userId) = 0;

  virtual bool addTaskTemplate(const ZM_Base::uTaskTemplate& cng, uint64_t& outTId) = 0;
  virtual bool getTaskTemplate(uint64_t tId, ZM_Base::uTaskTemplate& outTCng) = 0;
  virtual bool changeTaskTemplate(uint64_t tId, const ZM_Base::uTaskTemplate& newTCng, uint64_t& outTId) = 0;
  virtual bool delTaskTemplate(uint64_t tId) = 0;
  virtual std::vector<uint64_t> getAllTaskTemplates(uint64_t parent) = 0;

  virtual bool addTask(const ZM_Base::uTask&, uint64_t& outTId) = 0;
  virtual bool getTask(uint64_t tId, ZM_Base::uTask&) = 0;
  virtual bool changeTask(uint64_t tId, const ZM_Base::uTask& newTCng) = 0;
  virtual bool delTask(uint64_t tId) = 0;
  virtual bool startTask(uint64_t tId) = 0;
  virtual bool taskState(const std::vector<uint64_t>& tId, std::vector<taskPrsAState>&) = 0;
  virtual bool taskResult(uint64_t tId, std::string&) = 0;
  virtual bool taskTime(uint64_t tId, taskTime& out) = 0;
  virtual std::vector<uint64_t> getAllTasks(uint64_t pplId, ZM_Base::stateType) = 0;
  
  // for zmSchedr
  virtual bool getSchedr(std::string& connPnt, ZM_Base::scheduler& outSchedl) = 0;
  virtual bool getTasksOfSchedr(uint64_t sId, std::vector<schedrTask>& out) = 0;
  virtual bool getWorkersOfSchedr(uint64_t sId, std::vector<ZM_Base::worker>& out) = 0;
  virtual bool getNewTasksForSchedr(uint64_t sId, int maxTaskCnt, std::vector<schedrTask>& out) = 0;
  virtual bool sendAllMessFromSchedr(uint64_t sId, std::vector<messSchedr>& out) = 0;

  void setErrorCBack(errCBack ecb, udata ud);
  std::string getLastError();  

#ifdef DEBUG
  // for test
  virtual bool delAllUsers() = 0;
  virtual bool delAllSchedrs() = 0;
  virtual bool delAllWorkers() = 0;
  virtual bool delAllPipelines() = 0;
  virtual bool delAllTemplateTask() = 0;
  virtual bool delAllTask() = 0;
#endif

protected:  
  DbProvider(const connectCng&){};    
  void errorMess(const std::string&); 
  std::string _err;
  errCBack _errCBack = nullptr;
  udata _errUData = nullptr;
};

DbProvider* makeDbProvider(const connectCng&);
}