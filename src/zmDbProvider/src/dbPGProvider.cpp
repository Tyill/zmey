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

#include <vector>
#include <sstream>  
#include "../dbProvider.h"
#include "dbPGProvider.h"


using namespace std;

DbPGProvider::DbPGProvider(const ZM_DB::connectCng& connCng, ZM_DB::errCBack ecb)
  : _errCBack(ecb), ZM_DB::DbProvider(connCng, ecb){

  


}
DbPGProvider::~DbPGProvider(){
  //disconnect();
}

// for zmManager
bool DbPGProvider::addSchedr(const ZM_Base::scheduler& schedl, uint64_t& schId){
  return true;
}
bool DbPGProvider::schedrState(uint64_t schId, ZM_Base::scheduler& schedl){
  return true;
}
std::vector<uint64_t> DbPGProvider::getAllSchedrs(ZM_Base::stateType){
  return std::vector<uint64_t>();
}

bool DbPGProvider::addWorker(const ZM_Base::worker& worker, uint64_t& wkrId){
  return true;
}
bool DbPGProvider::workerState(uint64_t wkrId, ZM_Base::worker& worker){
  return true;
}
std::vector<uint64_t> DbPGProvider::getAllWorkers(uint64_t schId, ZM_Base::stateType){
  return std::vector<uint64_t>();
}

bool DbPGProvider::addTask(const ZM_Base::task& task, uint64_t& tskId){
  return true;
}
bool DbPGProvider::getTaskCng(uint64_t tskId, ZM_Base::task& task){
  return true;
}
std::vector<uint64_t> DbPGProvider::getAllTasks(){
  return std::vector<uint64_t>();
}

bool DbPGProvider::pushTaskToQueue(const ZM_Base::queueTask& task, uint64_t& qtskId){
  return true;
}
bool DbPGProvider::getQueueTaskCng(uint64_t qtskId, ZM_Base::queueTask& qTask){
  return true;
}
bool DbPGProvider::getQueueTaskState(uint64_t qtskId, ZM_Base::queueTask& qTask){
  return true;
}
std::vector<uint64_t> DbPGProvider::getAllQueueTasks(ZM_Base::stateType){
  return std::vector<uint64_t>();
}

// for zmSchedr
bool DbPGProvider::getSchedr(std::string& connPnt, ZM_Base::scheduler& outSchedl){
  return true;
}
bool 
DbPGProvider::getTasksForSchedr(uint64_t schedrId, std::vector<std::pair<ZM_Base::task, ZM_Base::queueTask>>&){
  return true;
}
bool DbPGProvider::getWorkersForSchedr(uint64_t schedrId, std::vector<ZM_Base::worker>&){
  return true;
}
bool 
DbPGProvider::getNewTasks(int maxTaskCnt, std::vector<std::pair<ZM_Base::task, ZM_Base::queueTask>>&){
  return true;
}
bool DbPGProvider::sendAllMessFromSchedr(uint64_t schedrId, std::vector<ZM_DB::messSchedr>&){
  return true;
}