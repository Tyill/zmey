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
#include "zmey/zmey.h"
#include "zmBase/structurs.h"
#include "structurs.h"
#include "zmDbProvider/dbProvider.h"

class Manager{
  std::string _err;
  zmey::zmErrorCBack _errorCBack = nullptr;
  zmey::zmUData _errorUData = nullptr;
  ZM_DB::DbProvider* _db = nullptr;
  // std::map<uint64_t, ZM_Base::scheduler> _schedrs;  // key id tblSchedr
  // std::map<uint64_t, ZM_Base::worker> _workers;     // key id tblWorker
  // std::map<uint64_t, ZM_Base::task> _tasks;         // key id tblWorker
public:
  Manager(const ZM_DB::connectCng&);
  ~Manager();
  void setErrorCBack(zmey::zmErrorCBack ecb, zmey::zmUData ud);
  std::string getLastError();
  void errorMess(const std::string&); 
 
  bool addUser(const ZM_Base::user& newUserCng, uint64_t& outUserId);
  bool getUser(const std::string& name, const std::string& passw, uint64_t& outUserId);
  bool getUser(uint64_t userId, ZM_Base::user& cng);
  bool changeUser(uint64_t userIdCng, const ZM_Base::user& newCng);
  bool delUser(uint64_t userId);  
  std::vector<uint64_t> getAllUsers();

  bool addScheduler(ZM_Base::scheduler&, uint64_t& outSchId);
  bool schedulerState(uint64_t schId, ZM_Base::scheduler& outSchCng);
  std::vector<uint64_t> getAllSchedulers(ZM_Base::stateType);
  
  bool addWorker(ZM_Base::worker&, uint64_t& outWId);
  bool workerState(uint64_t wId, ZM_Base::worker&);
  std::vector<uint64_t> getAllWorkers(uint64_t schId, ZM_Base::stateType);
  
  bool addPipeline(const ZM_Base::uPipeline& cng, uint64_t& outPPLId);
  bool getPipeline(uint64_t pplId, ZM_Base::uPipeline& cng);
  bool changePipeline(uint64_t pplId, const ZM_Base::uPipeline& newCng);
  bool delPipeline(uint64_t pplId);
  std::vector<uint64_t> getAllPipelines(uint64_t userId);

  bool addTaskTemplate(const ZM_Base::uTaskTemplate& cng, uint64_t& outTId);
  bool getTaskTemplateCng(uint64_t tId, ZM_Base::uTaskTemplate& outCng);
  bool changeTaskTemplateCng(uint64_t tId, const ZM_Base::uTaskTemplate& newTCng, uint64_t& outTId);
  bool delTaskTemplate(uint64_t tId);
  std::vector<uint64_t> getAllTaskTemplates(uint64_t parent);

  bool addTask(ZM_Base::uTask&, uint64_t& outTId);
  bool getTaskCng(uint64_t tId, ZM_Base::uTask&);
  bool changeTaskCng(uint64_t tId, const ZM_Base::uTask& newTCng);
  bool delTask(uint64_t tId);
  bool startTask(uint64_t tId);
  bool stopTask(uint64_t tId);
  bool pauseTask(uint64_t tId);
  bool getTaskState(uint64_t tId, ZM_Base::queueTask&);
  std::vector<uint64_t> getAllTasks(uint64_t pplId, ZM_Base::stateType);
};