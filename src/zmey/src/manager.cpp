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
#include <functional>
#include "manager.h"
#include "zmDbProvider/dbProvider.h"

using namespace std;

Manager::Manager(const ZM_DB::connectCng& connCng){
  
  _db = ZM_DB::makeDbProvider(connCng, bind(&Manager::errorMess, this, placeholders::_1));
}
Manager::~Manager(){
  if (_db){
    delete _db;
  }
}
void Manager::setErrorCBack(zmey::zmErrorCBack ecb, zmey::zmUData ud){
  _errorCBack = ecb;
  _errorUData = ud;
}
void Manager::errorMess(const std::string& mess){
  _err = mess;
  if (_errorCBack){
    _errorCBack(mess.c_str(), _errorUData);
  } 
}
std::string Manager::getLastError(){
  return _err;
}
  
bool Manager::addUser(const ZM_Base::user& newUserCng, uint64_t& outUserId){
  return _db->addUser(newUserCng, outUserId);
}
bool Manager::getUser(const std::string& name, const std::string& passw, uint64_t& outUserId){
  return _db->getUser(name, passw, outUserId);
}
bool Manager::getUser(uint64_t userId, ZM_Base::user& outCng){
  return _db->getUser(userId, outCng);
}
bool Manager::changeUser(uint64_t userId, const ZM_Base::user& newCng){
  return _db->changeUser(userId, newCng);
}
bool Manager::delUser(uint64_t userId){
  return _db->delUser(userId);
}
std::vector<uint64_t> Manager::getAllUsers(){
  return _db->getAllUsers();
}

bool Manager::addScheduler(ZM_Base::scheduler& schedr, uint64_t& outSchId){  
  return _db->addSchedr(schedr, outSchId);
}
bool Manager::schedulerState(uint64_t schId, ZM_Base::scheduler& schedr){
  return _db->schedrState(schId, schedr);
}
std::vector<uint64_t> Manager::getAllSchedulers(ZM_Base::stateType state){
  return _db->getAllSchedrs(state);
}

bool Manager::addWorker(ZM_Base::worker& worker, uint64_t& outWId){
  return _db->addWorker(worker, outWId);
}
bool Manager::workerState(uint64_t wId, ZM_Base::worker& worker){
  return _db->workerState(wId, worker);
}
std::vector<uint64_t> Manager::getAllWorkers(uint64_t schId, ZM_Base::stateType state){
  return _db->getAllWorkers(schId, state);
}

bool Manager::addPipeline(const ZM_Base::pipeline& cng, uint64_t& outPPLId){
  return _db->addPipeline(cng, outPPLId);
}
bool Manager::getPipeline(uint64_t pplId, ZM_Base::pipeline& cng){
  return _db->getPipeline(pplId, cng);
}
bool Manager::changePipeline(uint64_t pplId, const ZM_Base::pipeline& newCng){
  return _db->changePipeline(pplId, newCng);
}
bool Manager::delPipeline(uint64_t pplId){
  return _db->delPipeline(pplId);
}
std::vector<uint64_t> Manager::getAllPipelines(uint64_t userId){
  return _db->getAllPipelines(userId);
}

bool Manager::addTask(ZM_Base::task& task, uint64_t& outTId){
  return _db->addTask(task, outTId);
}
bool Manager::getTaskCng(uint64_t tId, ZM_Base::task& task){
  return _db->getTaskCng(tId, task);
}
std::vector<uint64_t> Manager::getAllTasks(){
  return _db->getAllTasks();
}

bool Manager::pushTaskToQueue(ZM_Base::queueTask& qTask, uint64_t& outQTId){
  return _db->pushTaskToQueue(qTask, outQTId);
}
bool Manager::getQueueTaskCng(uint64_t qtId, ZM_Base::queueTask& qTask){
  return _db->getQueueTaskCng(qtId, qTask);
}
bool Manager::getQueueTaskState(uint64_t qtId, ZM_Base::queueTask& qTask){
  return _db->getQueueTaskState(qtId, qTask);
}
std::vector<uint64_t> Manager::getAllQueueTasks(ZM_Base::stateType state){
  return _db->getAllQueueTasks(state);
}