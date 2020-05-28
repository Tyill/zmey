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
#include "zmManager.h"

ZManager::ZManager(const std::string& localPnt, const std::string& dbServer, const std::string& dbName){

}
ZManager::~ZManager(){

}
void ZManager::setErrorCBack(zmey::zmErrorCBack ecb, zmey::zmUData ud){
  _errorCBack = ecb;
  _errorUData = ud;
}
void ZManager::errorMess(const std::string& mess){
  _err = mess;
  if (_errorCBack){
    _errorCBack(mess.c_str(), _errorUData);
  } 
}
std::string ZManager::getLastError(){
  return _err;
}
bool ZManager::createDB(const std::string& dbName){
  return false;
}
  
bool ZManager::addScheduler(const ZM_Base::scheduler&, uint64_t& outSchId){
  return false;
}
bool ZManager::schedulerState(uint64_t schId, ZM_Base::scheduler&){
  return false;
}
std::vector<uint64_t> ZManager::getAllSchedulers(){
  return std::vector<uint64_t>();
}

bool ZManager::addWorker(uint64_t schId, const ZM_Base::worker&, uint64_t& outWId){
  return false;
}
bool ZManager::workerState(uint64_t wId, ZM_Base::worker&){
  return false;
}
std::vector<uint64_t> ZManager::getAllWorkers(uint64_t schId){
  return std::vector<uint64_t>();
}

bool ZManager::addTask(const ZM_Base::task&, uint64_t& outTId){
  return false;
}
bool ZManager::getTaskCng(uint64_t tId, ZM_Base::task&){
  return false;
}
std::vector<uint64_t> getAllTasks(){
  return std::vector<uint64_t>();
}

bool ZManager::pushTaskToQueue(const queueTask&, uint64_t& outQTId){
  return false;
}
bool ZManager::getQueueTaskState(uint64_t qtId, queueTask&){
  return false;
}
std::vector<uint64_t> ZManager::getAllQueueTasks(){
  return std::vector<uint64_t>();
}