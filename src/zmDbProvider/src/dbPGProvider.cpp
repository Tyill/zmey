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
#include <libpq-fe.h>
#include "../dbProvider.h"
#include "dbPGProvider.h"



using namespace std;

DbPGProvider::DbPGProvider(const ZM_DB::connectCng& connCng)
  : ZM_DB::DbProvider(connCng){

  // PGconn* _pg = PQconnectdbParams(const char * const *keywords,
  //                         const char * const *values,
  //                         int expand_dbname);


}
DbPGProvider::~DbPGProvider(){
  //disconnect();
}

// for manager
bool DbPGProvider::addUser(const ZM_Base::user& newUserCng, uint64_t& outUserId){
  return true;
}
bool DbPGProvider::getUser(const std::string& name, const std::string& passw, uint64_t& outUserId){
  return true;
}
bool DbPGProvider::getUser(uint64_t userId, ZM_Base::user& cng){
  return true;
}
bool DbPGProvider::changeUser(uint64_t userId, const ZM_Base::user& newCng){
  return true;
}
bool DbPGProvider::delUser(uint64_t userId){
  return true;
}
std::vector<uint64_t> DbPGProvider::getAllUsers(){
  return std::vector<uint64_t>();
}

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

bool DbPGProvider::addPipeline(const ZM_Base::uPipeline& cng, uint64_t& outPPLId){
  return true;
}
bool DbPGProvider::getPipeline(uint64_t pplId, ZM_Base::uPipeline& cng){
  return true;
}
bool DbPGProvider::changePipeline(uint64_t pplId, const ZM_Base::uPipeline& newCng){
  return true;
}
bool DbPGProvider::delPipeline(uint64_t pplId){
  return true;
}
std::vector<uint64_t> DbPGProvider::getAllPipelines(uint64_t userId){
  return std::vector<uint64_t>();
}

bool DbPGProvider::addTaskTemplate(const ZM_Base::uTaskTemplate& cng, uint64_t& outTId){
  return true;
}
bool DbPGProvider::getTaskTemplateCng(uint64_t tId, ZM_Base::uTaskTemplate& outTCng){
  return true;
};
bool DbPGProvider::changeTaskTemplateCng(uint64_t tId, const ZM_Base::uTaskTemplate& newTCng, uint64_t& outTId){
  return true;
}
bool DbPGProvider::delTaskTemplate(uint64_t tId){
  return true;
}
std::vector<uint64_t> DbPGProvider::getAllTaskTemplates(uint64_t parent){
  return std::vector<uint64_t>();
}

bool DbPGProvider::addTask(ZM_Base::uTask&, uint64_t& outTId){
  return true;
}
bool DbPGProvider::getTaskCng(uint64_t tId, ZM_Base::uTask&){
  return true;
}
bool DbPGProvider::changeTaskCng(uint64_t tId, const ZM_Base::uTask& newTCng){
  return true;
}
bool DbPGProvider::delTask(uint64_t tId){
  return true;
}
bool DbPGProvider::startTask(uint64_t tId){
  return true;
}
bool DbPGProvider::getTaskState(uint64_t tId, ZM_Base::queueTask&){
  return true;
}
std::vector<uint64_t> DbPGProvider::getAllTasks(uint64_t pplId, ZM_Base::stateType){
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