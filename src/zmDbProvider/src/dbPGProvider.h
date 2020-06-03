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
  DbPGProvider(const ZM_DB::connectCng&, ZM_DB::errCBack);    
  ~DbPGProvider(); 
  DbPGProvider(const DbProvider& other) = delete;
  DbPGProvider& operator=(const DbProvider& other) = delete;
  
  // for zmManager
  bool addSchedr(const ZM_Base::scheduler& schedl, uint64_t& outSchId) override;
  bool schedrState(uint64_t schId, ZM_Base::scheduler& schedl) override;
  std::vector<uint64_t> getAllSchedrs(ZM_Base::stateType) override;

  bool addWorker(const ZM_Base::worker& worker, uint64_t& outWkrId) override;
  bool workerState(uint64_t wkrId, ZM_Base::worker& out) override;
  std::vector<uint64_t> getAllWorkers(uint64_t schId, ZM_Base::stateType) override;

  bool addTask(const ZM_Base::task& task, uint64_t& outTskId) override;
  bool getTaskCng(uint64_t tskId, ZM_Base::task& task) override;
  std::vector<uint64_t> getAllTasks() override;

  bool pushTaskToQueue(const ZM_Base::queueTask& task, uint64_t& outQId) override;
  bool getQueueTaskCng(uint64_t qId, ZM_Base::queueTask& qTask) override;
  bool getQueueTaskState(uint64_t qId, ZM_Base::queueTask& qTask) override;
  std::vector<uint64_t> getAllQueueTasks(ZM_Base::stateType) override;

  // for zmSchedr
  bool getSchedr(std::string& connPnt, ZM_Base::scheduler& outSchedl) override;
  bool getTasksForSchedr(uint64_t schedrId, std::vector<std::pair<ZM_Base::task, ZM_Base::queueTask>>&) override;
  bool getWorkersForSchedr(uint64_t schedrId, std::vector<ZM_Base::worker>&) override;
  bool getNewTasks(int maxTaskCnt, std::vector<std::pair<ZM_Base::task, ZM_Base::queueTask>>&) override;
  bool sendAllMessFromSchedr(uint64_t schedrId, std::vector<ZM_DB::messSchedr>&) override;
private:
  std::string _lastErr;
  ZM_DB::errCBack _errCBack = nullptr;
  bool query(const std::string& query, std::vector<std::vector<std::string>>& results) const;
};