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

#include <mutex>
#include <libpq-fe.h>
#include "../dbProvider.h"

class DbPGProvider final : ZM_DB::DbProvider{  
public:
  DbPGProvider(const ZM_DB::connectCng&);    
  ~DbPGProvider(); 
  DbPGProvider(const DbProvider& other) = delete;
  DbPGProvider& operator=(const DbProvider& other) = delete;
  bool createTables() override;
  
  // for manager
  bool addUser(const ZM_Base::user& newUserCng, uint64_t& outUserId) override;
  bool getUserId(const std::string& name, const std::string& passw, uint64_t& outUserId) override;
  bool getUserCng(uint64_t userId, ZM_Base::user& cng) override;
  bool changeUser(uint64_t userId, const ZM_Base::user& newCng) override;
  bool delUser(uint64_t userId) override;
  std::vector<uint64_t> getAllUsers() override;

  bool addSchedr(const ZM_Base::scheduler& schedl, uint64_t& outSchId) override;
  bool getSchedr(uint64_t sId, ZM_Base::scheduler& cng) override;
  bool changeSchedr(uint64_t sId, const ZM_Base::scheduler& newCng) override;
  bool delSchedr(uint64_t sId) override;
  bool schedrState(uint64_t sId, ZM_Base::stateType&) override;
  std::vector<uint64_t> getAllSchedrs(ZM_Base::stateType) override;

  bool addWorker(const ZM_Base::worker& worker, uint64_t& outWkrId) override;
  bool getWorker(uint64_t wId, ZM_Base::worker& cng) override;
  bool changeWorker(uint64_t wId, const ZM_Base::worker& newCng) override;
  bool delWorker(uint64_t wId) override;
  bool workerState(const std::vector<uint64_t>& wId, std::vector<ZM_Base::stateType>& out) override;
  std::vector<uint64_t> getAllWorkers(uint64_t sId, ZM_Base::stateType) override;

  bool addPipeline(const ZM_Base::uPipeline& cng, uint64_t& outPPLId) override;
  bool getPipeline(uint64_t pplId, ZM_Base::uPipeline& cng) override;
  bool changePipeline(uint64_t pplId, const ZM_Base::uPipeline& newCng) override;
  bool delPipeline(uint64_t pplId) override;
  std::vector<uint64_t> getAllPipelines(uint64_t userId) override;

  bool addTaskTemplate(const ZM_Base::uTaskTemplate& cng, uint64_t& outTId) override;
  bool getTaskTemplate(uint64_t tId, ZM_Base::uTaskTemplate& outTCng) override;;
  bool changeTaskTemplate(uint64_t tId, const ZM_Base::uTaskTemplate& newTCng, uint64_t& outTId) override;
  bool delTaskTemplate(uint64_t tId) override;
  std::vector<uint64_t> getAllTaskTemplates(uint64_t parent) override;

  bool addTask(const ZM_Base::uTask&, uint64_t& outTId) override;
  bool getTask(uint64_t tId, ZM_Base::uTask&) override;
  bool changeTask(uint64_t tId, const ZM_Base::uTask& newTCng) override;
  bool delTask(uint64_t tId) override;
  bool startTask(uint64_t tId) override;
  bool taskState(const std::vector<uint64_t>& tId, std::vector<ZM_DB::tskState>&) override;
  bool taskResult(uint64_t tId, std::string&) override;
  bool taskTime(uint64_t tId, ZM_DB::taskTime& out) override;
  std::vector<uint64_t> getAllTasks(uint64_t pplId, ZM_Base::stateType) override;
  bool getSchedrAndWorkerByTask(uint64_t tId, uint64_t& qtId, ZM_Base::scheduler& scng, ZM_Base::worker& wcng) override;

  std::vector<ZM_DB::messError> getErrors(uint64_t sId, uint64_t wId, uint32_t mCnt) override;

  // for zmSchedr
  bool getSchedr(std::string& connPnt, ZM_Base::scheduler& outSchedl) override;
  bool getTasksOfSchedr(uint64_t sId, std::vector<ZM_DB::schedrTask>&) override;
  bool getWorkersOfSchedr(uint64_t sId, std::vector<ZM_Base::worker>&) override;
  bool getNewTasksForSchedr(uint64_t sId, int maxTaskCnt, std::vector<ZM_DB::schedrTask>&) override;
  bool sendAllMessFromSchedr(uint64_t schedrId, std::vector<ZM_DB::messSchedr>&) override;

  // for test
  bool delAllUsers() override;
  bool delAllSchedrs() override;
  bool delAllWorkers() override;
  bool delAllPipelines() override;
  bool delAllTemplateTask() override;
  bool delAllTask() override;

private:
  PGconn* _pg = nullptr; 
  std::mutex _mtx;
};