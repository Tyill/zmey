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
  ZM_Base::manager _mnr;
  // std::map<uint64_t, ZM_Base::scheduler> _schedrs;  // key id tblSchedr
  // std::map<uint64_t, ZM_Base::worker> _workers;     // key id tblWorker
  // std::map<uint64_t, ZM_Base::task> _tasks;         // key id tblWorker
public:
  Manager(const ZM_DB::connectCng&, const zmey::zmManagerCng&);
  ~Manager();
  void setErrorCBack(zmey::zmErrorCBack ecb, zmey::zmUData ud);
  std::string getLastError();
  void errorMess(const std::string&); 
 
  bool addScheduler(ZM_Base::scheduler&, uint64_t& outSchId);
  bool schedulerState(uint64_t schId, ZM_Base::scheduler& outSchCng);
  std::vector<uint64_t> getAllSchedulers(ZM_Base::stateType);
  
  bool addWorker(ZM_Base::worker&, uint64_t& outWId);
  bool workerState(uint64_t wId, ZM_Base::worker&);
  std::vector<uint64_t> getAllWorkers(uint64_t schId, ZM_Base::stateType);
  
  bool addTask(ZM_Base::task&, uint64_t& outTId);
  bool getTaskCng(uint64_t tId, ZM_Base::task&);
  std::vector<uint64_t> getAllTasks();
  
  bool pushTaskToQueue(ZM_Base::queueTask&, uint64_t& outQTId);
  bool getQueueTaskCng(uint64_t qtId, ZM_Base::queueTask&);
  bool getQueueTaskState(uint64_t qtId, ZM_Base::queueTask&);
  std::vector<uint64_t> getAllQueueTasks(ZM_Base::stateType);
};