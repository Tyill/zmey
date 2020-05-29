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

Manager::Manager(const std::string& localPnt, zmey::zmDbType dbType, const std::string& dbServer, const std::string& dbName){
  
  _db = ZM_DB::makeDbProvider((ZM_DB::dbType)dbType, dbServer, dbName, bind(&Manager::errorMess, this, placeholders::_1));
  if (!_db){
    errorMess("not support dbType " + to_string((int)dbType));
  }
}
Manager::~Manager(){

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
  
bool Manager::addScheduler(const ZM_Base::scheduler& schedr, uint64_t& outSchId){  
  return _db->addSchedr(schedr, outSchId);
}
bool Manager::schedulerState(uint64_t schId, ZM_Base::scheduler&){

  

  return false;
}
std::vector<uint64_t> Manager::getAllSchedulers(){
  return std::vector<uint64_t>();
}

bool Manager::addWorker(uint64_t schId, const ZM_Base::worker&, uint64_t& outWId){
  return false;
}
bool Manager::workerState(uint64_t wId, ZM_Base::worker&){
  return false;
}
std::vector<uint64_t> Manager::getAllWorkers(uint64_t schId){
  return std::vector<uint64_t>();
}

bool Manager::addTask(const ZM_Base::task&, uint64_t& outTId){
  return false;
}
bool Manager::getTaskCng(uint64_t tId, ZM_Base::task&){
  return false;
}
std::vector<uint64_t> getAllTasks(){
  return std::vector<uint64_t>();
}

bool Manager::pushTaskToQueue(const queueTask&, uint64_t& outQTId){
  return false;
}
bool Manager::getQueueTaskState(uint64_t qtId, queueTask&){
  return false;
}
std::vector<uint64_t> Manager::getAllQueueTasks(){
  return std::vector<uint64_t>();
}
