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
  filesJSON =  0,
  SQLite =     1,
  PostgreSQL = 2,
};
std::string dbTypeToStr(dbType);
dbType dbTypeFromStr(const std::string& dbt);

struct connectCng{
  dbType dbSelType;
  std::string connectPnt;
  std::string dbServer;
  std::string dbName;
  std::string dbUser;
  std::string dbPassw;  
};

struct messSchedr{
  ZM_Base::messType type;
  uint64_t workerId;
  uint64_t taskId;
  int workerActiveTaskCnt;  
  int schedrActiveTaskCnt;
  int progress;
  std::string result;
};

typedef std::function<void(const std::string& stsMess)> errCBack;

class DbProvider{  
  friend DbProvider* makeDbProvider(const connectCng&, errCBack);
public:  
  virtual ~DbProvider() = default; 
  DbProvider(const DbProvider& other) = delete;
  DbProvider& operator=(const DbProvider& other) = delete;
  std::string getLastError() const{
    return _err;
  }  
  // common
  virtual bool 
  getManager(const std::string& mnrName,
             const std::string& mnrPassw, ZM_Base::manager& out) = 0;
  
  // for zmManager
  virtual bool addSchedr(const ZM_Base::scheduler& schedl, uint64_t& outSchId) = 0;
  virtual bool schedrState(uint64_t schId, ZM_Base::scheduler& out) = 0;
  virtual std::vector<uint64_t> getAllSchedrs(uint64_t mnrId, ZM_Base::stateType) = 0;
  
  virtual bool addWorker(const ZM_Base::worker& worker, uint64_t& outWkrId) = 0;
  virtual bool workerState(uint64_t wkrId, ZM_Base::worker& out) = 0;
  virtual std::vector<uint64_t> getAllWorkers(uint64_t mnrId, uint64_t schId, ZM_Base::stateType) = 0;

  virtual bool addTask(const ZM_Base::task& task, uint64_t& outTskId) = 0;
  virtual bool getTaskCng(uint64_t tskId, ZM_Base::task& task) = 0;
  virtual std::vector<uint64_t> getAllTasks(uint64_t mnrId) = 0;

  virtual bool pushTaskToQueue(const ZM_Base::queueTask& task, uint64_t& outQId) = 0;
  virtual bool getQueueTaskCng(uint64_t qId, ZM_Base::queueTask& out) = 0;
  virtual bool getQueueTaskState(uint64_t qId, ZM_Base::queueTask& out) = 0;
  virtual std::vector<uint64_t> getAllQueueTasks(uint64_t mnrId, ZM_Base::stateType) = 0;
  
  // for zmSchedr
  virtual bool getSchedr(std::string& connPnt, ZM_Base::scheduler& outSchedl) = 0;
  virtual bool getTasksForSchedr(uint64_t schId, std::vector<std::pair<ZM_Base::task, ZM_Base::queueTask>>& out) = 0;
  virtual bool getWorkersForSchedr(uint64_t schId, std::vector<ZM_Base::worker>& out) = 0;
  virtual bool getNewTasks(int maxTaskCnt, std::vector<std::pair<ZM_Base::task, ZM_Base::queueTask>>& out) = 0;
  virtual bool sendAllMessFromSchedr(uint64_t schId, std::vector<messSchedr>& out) = 0;
protected:  
  DbProvider(const connectCng&, errCBack){};  
  std::string _err;
  errCBack _errCBack = nullptr;
};

DbProvider* makeDbProvider(const connectCng&, errCBack);
}