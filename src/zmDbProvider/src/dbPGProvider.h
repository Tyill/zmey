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

#include "../dbProvider.h"

class DbPGProvider final : ZM_DB::DbProvider{  
public:
  DbPGProvider(const ZM_DB::connectCng&);    
  ~DbPGProvider(); 
  DbPGProvider(const DbProvider& other) = delete;
  DbPGProvider& operator=(const DbProvider& other) = delete;
  
  // for manager
  bool addUser(const ZM_Base::user& newUserCng, uint64_t& outUserId) override;
  bool getUser(const std::string& name, const std::string& passw, uint64_t& outUserId) override;
  bool getUser(uint64_t userId, ZM_Base::user& cng) override;
  bool changeUser(uint64_t userId, const ZM_Base::user& newCng) override;
  bool delUser(uint64_t userId) override;
  std::vector<uint64_t> getAllUsers() override;

  bool addSchedr(const ZM_Base::scheduler& schedl, uint64_t& outSchId) override;
  bool schedrState(uint64_t schId, ZM_Base::scheduler& schedl) override;
  std::vector<uint64_t> getAllSchedrs(ZM_Base::stateType) override;

  bool addWorker(const ZM_Base::worker& worker, uint64_t& outWkrId) override;
  bool workerState(uint64_t wkrId, ZM_Base::worker& out) override;
  std::vector<uint64_t> getAllWorkers(uint64_t schId, ZM_Base::stateType) override;

  bool addPipeline(const ZM_Base::uPipeline& cng, uint64_t& outPPLId) override;
  bool getPipeline(uint64_t pplId, ZM_Base::uPipeline& cng) override;
  bool changePipeline(uint64_t pplId, const ZM_Base::uPipeline& newCng) override;
  bool delPipeline(uint64_t pplId) override;
  std::vector<uint64_t> getAllPipelines(uint64_t userId) override;

  bool addTaskTemplate(const ZM_Base::uTaskTemplate& cng, uint64_t& outTId) override;
  bool getTaskTemplateCng(uint64_t tId, ZM_Base::uTaskTemplate& outTCng) override;;
  bool changeTaskTemplateCng(uint64_t tId, const ZM_Base::uTaskTemplate& newTCng, uint64_t& outTId) override;
  bool delTaskTemplate(uint64_t tId) override;
  std::vector<uint64_t> getAllTaskTemplates(uint64_t parent) override;

  bool addTask(ZM_Base::uTask&, uint64_t& outTId) override;
  bool getTaskCng(uint64_t tId, ZM_Base::uTask&) override;
  bool changeTaskCng(uint64_t tId, const ZM_Base::uTask& newTCng) override;
  bool delTask(uint64_t tId) override;
  bool startTask(uint64_t tId) override;
  bool getTaskState(uint64_t tId, ZM_Base::queueTask&) override;
  std::vector<uint64_t> getAllTasks(uint64_t pplId, ZM_Base::stateType) override;

  // for zmSchedr
  bool getSchedr(std::string& connPnt, ZM_Base::scheduler& outSchedl) override;
  bool getTasksForSchedr(uint64_t schedrId, std::vector<ZM_Base::schedrTask>&) override;
  bool getWorkersForSchedr(uint64_t schedrId, std::vector<ZM_Base::worker>&) override;
  bool getNewTasks(int maxTaskCnt, std::vector<std::pair<ZM_Base::task, ZM_Base::queueTask>>&) override;
  bool sendAllMessFromSchedr(uint64_t schedrId, std::vector<ZM_DB::messSchedr>&) override;
private:
  bool query(const std::string& query, std::vector<std::vector<std::string>>& results) const;
};