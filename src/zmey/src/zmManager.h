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
#include <string>
#include <vector>
#include "zmey/zmey.h"
#include "zmBase/structurs.h"

class ZManager{
  std::string _err;
public:
  ZManager(const std::string& localPnt, const std::string& dbServer, const std::string& dbName);
  ~ZManager();
  std::string getLastError();
  bool createDB(const std::string& dbName);
  
  bool addScheduler(const ZM_Base::scheduler&, uint64_t& outSchId);
  bool getScheduler(uint64_t schId, ZM_Base::scheduler& outSchCng);
  std::vector<uint64_t> getAllSchedulers();
  
  bool addWorker(uint64_t schId, const ZM_Base::worker&, uint64_t& outWId);
  bool getWorker(uint64_t wId, ZM_Base::worker&);
  std::vector<uint64_t> getAllWorkers(uint64_t schId);
  
  bool addTask(const ZM_Base::task&, uint64_t& outTId);
  bool getTask(uint64_t tId, ZM_Base::task&);
  std::vector<uint64_t> getAllTasks();
  
  bool pushTaskToQueue(const ZM_Base::task&, uint64_t& outQTId);
  bool getQueueTaskState(uint64_t qtId, ZM_Base::task&);
  std::vector<uint64_t> getAllQueueTasks();
};