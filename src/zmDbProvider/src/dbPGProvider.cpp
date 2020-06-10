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
#include <numeric>
#include <sstream>  
#include "zmCommon/auxFunc.h"
#include "dbPGProvider.h"


using namespace std;

DbPGProvider::DbPGProvider(const ZM_DB::connectCng& connCng)
  : ZM_DB::DbProvider(connCng){

  _pg = PQconnectdb(connCng.connectStr.c_str());
  if (PQstatus(_pg) != CONNECTION_OK){
    errorMess(PQerrorMessage(_pg));
    return;
  }  
#define RET_VAL  
#define QUERY(req, sts){                \
    auto res = PQexec(_pg, req);        \
    if (PQresultStatus(res) != sts){    \
        errorMess(string("DbPGProvider error: ") + PQerrorMessage(_pg)); \
        PQclear(res);                   \
        return RET_VAL;                 \
    }                                   \
    PQclear(res);                       \
  }
  QUERY("SELECT pg_catalog.set_config('search_path', 'public', false)", PGRES_TUPLES_OK);

  stringstream ss;
  ss << "CREATE TABLE IF NOT EXISTS tblUser("
        "id          SERIAL PRIMARY KEY,"
        "name        TEXT NOT NULL UNIQUE CHECK (name <> ''),"
        "passwHash   TEXT NOT NULL,"
        "description TEXT NOT NULL,"
        "isDelete    INT NOT NULL DEFAULT 0 CHECK (isDelete BETWEEN 0 AND 1));";
  QUERY(ss.str().c_str(), PGRES_COMMAND_OK);

  /////////////////////////////////////////////////////////////////////////////
  /// SYSTEM TABLES
  ss.str("");
  ss << "CREATE TABLE IF NOT EXISTS tblExecutor("
        "id           SERIAL PRIMARY KEY,"
        "kind         TEXT NOT NULL CHECK (kind <> ''));";
  QUERY(ss.str().c_str(), PGRES_COMMAND_OK);

  ss.str("");
  ss << "INSERT INTO tblExecutor VALUES"
        "(0, 'bash'),"
        "(1, 'cmd'),"
        "(2, 'python') ON CONFLICT (id) DO NOTHING;";
  QUERY(ss.str().c_str(), PGRES_COMMAND_OK);

  ss.str("");
  ss << "CREATE TABLE IF NOT EXISTS tblConnectPnt("
        "id           SERIAL PRIMARY KEY,"
        "ipAddr       TEXT NOT NULL CHECK (ipAddr <> ''),"
        "port         INT NOT NULL CHECK (port > 0));";
  QUERY(ss.str().c_str(), PGRES_COMMAND_OK);
  
  ss.str("");
  ss << "CREATE TABLE IF NOT EXISTS tblTask("
        "id           SERIAL PRIMARY KEY,"
        "script       TEXT NOT NULL CHECK (script <> ''),"
        "executor     INT NOT NULL REFERENCES tblExecutor,"
        "averDurationSec INT NOT NULL CHECK (averDurationSec > 0),"
        "maxDurationSec  INT NOT NULL CHECK (maxDurationSec > 0));";
  QUERY(ss.str().c_str(), PGRES_COMMAND_OK);

  ss.str("");
  ss << "CREATE TABLE IF NOT EXISTS tblState("
        "id           SERIAL PRIMARY KEY,"
        "kind         TEXT NOT NULL CHECK (kind <> ''));";
  QUERY(ss.str().c_str(), PGRES_COMMAND_OK);
    
  ss.str("");
  ss << "INSERT INTO tblState VALUES"
        "(-1, 'undefined'),"
        "(0, 'ready'),"
        "(1, 'start'),"
        "(2, 'running'),"
        "(3, 'pause'),"
        "(4, 'stop'),"
        "(5, 'completed'),"
        "(6, 'error'),"
        "(7, 'notResponding') ON CONFLICT (id) DO NOTHING;";
  QUERY(ss.str().c_str(), PGRES_COMMAND_OK);

  ss.str("");
  ss << "CREATE TABLE IF NOT EXISTS tblScheduler("
        "id           SERIAL PRIMARY KEY,"
        "connPnt      INT NOT NULL REFERENCES tblConnectPnt,"
        "state        INT NOT NULL REFERENCES tblState,"
        "capacityTask INT NOT NULL DEFAULT 10000 CHECK (capacityTask > 0),"
        "activeTask   INT NOT NULL DEFAULT 0 CHECK (activeTask >= 0),"
        "isDelete     INT NOT NULL DEFAULT 0 CHECK (isDelete BETWEEN 0 AND 1));";
  QUERY(ss.str().c_str(), PGRES_COMMAND_OK);

  ss.str("");
  ss << "CREATE TABLE IF NOT EXISTS tblWorker("
        "id           SERIAL PRIMARY KEY,"
        "connPnt      INT NOT NULL REFERENCES tblConnectPnt,"
        "schedr       INT NOT NULL REFERENCES tblScheduler,"
        "state        INT NOT NULL REFERENCES tblState,"
        "executor     INT NOT NULL REFERENCES tblExecutor,"
        "capacityTask INT NOT NULL DEFAULT 10 CHECK (capacityTask > 0),"
        "activeTask   INT NOT NULL DEFAULT 0 CHECK (activeTask >= 0),"
        "rating       INT NOT NULL DEFAULT 10 CHECK (rating BETWEEN 1 AND 10),"
        "isDelete     INT NOT NULL DEFAULT 0 CHECK (isDelete BETWEEN 0 AND 1));";
  QUERY(ss.str().c_str(), PGRES_COMMAND_OK);

  ss.str("");
  ss << "CREATE TABLE IF NOT EXISTS tblTaskQueue("
        "id           SERIAL PRIMARY KEY,"
        "task         INT NOT NULL REFERENCES tblTask,"
        "state        INT NOT NULL REFERENCES tblState,"
        "launcher     INT NOT NULL REFERENCES tblUser,"
        "schedr       INT REFERENCES tblScheduler,"
        "worker       INT REFERENCES tblWorker,"
        "percent      INT NOT NULL DEFAULT 0 CHECK (percent BETWEEN 0 AND 100),"
        "priority     INT NOT NULL DEFAULT 1 CHECK (priority BETWEEN 1 AND 3),"
        "createTime   TIMESTAMP NOT NULL DEFAULT current_timestamp,"
        "startTime    TIMESTAMP CHECK (startTime > createTime),"
        "stopTime     TIMESTAMP CHECK (stopTime > startTime),"
        "isDependence INT NOT NULL DEFAULT 0 CHECK (isDependence BETWEEN 0 AND 1));";
  QUERY(ss.str().c_str(), PGRES_COMMAND_OK);

  ss.str("");
  ss << "CREATE TABLE IF NOT EXISTS tblParam("
        "id           SERIAL PRIMARY KEY,"
        "qtask        INT NOT NULL REFERENCES tblTaskQueue,"
        "key          TEXT NOT NULL CHECK (key <> ''),"
        "value        TEXT NOT NULL CHECK (value <> ''));";
  QUERY(ss.str().c_str(), PGRES_COMMAND_OK);

  ss.str("");
  ss << "CREATE TABLE IF NOT EXISTS tblResult("
        "qtask        INT PRIMARY KEY REFERENCES tblTaskQueue,"
        "key          TEXT NOT NULL CHECK (key <> ''),"
        "value        TEXT NOT NULL CHECK (value <> ''));";
  QUERY(ss.str().c_str(), PGRES_COMMAND_OK);

  ss.str("");
  ss << "CREATE TABLE IF NOT EXISTS tblPrevTask("
        "id           SERIAL PRIMARY KEY,"
        "qtask        INT NOT NULL REFERENCES tblTaskQueue,"
        "prevTask     INT NOT NULL REFERENCES tblTaskQueue);";
  QUERY(ss.str().c_str(), PGRES_COMMAND_OK);
  
  ///////////////////////////////////////////////////////////////////////////
  /// USER TABLES
  ss.str("");
  ss << "CREATE TABLE IF NOT EXISTS tblUPipeline("
        "id           SERIAL PRIMARY KEY,"
        "usr          INT NOT NULL REFERENCES tblUser,"
        "name         TEXT NOT NULL CHECK (name <> ''),"
        "description  TEXT NOT NULL,"
        "isDelete     INT NOT NULL DEFAULT 0 CHECK (isDelete BETWEEN 0 AND 1));";
  QUERY(ss.str().c_str(), PGRES_COMMAND_OK);

  ss.str("");
  ss << "CREATE TABLE IF NOT EXISTS tblUTaskTemplate("
        "task         INT PRIMARY KEY REFERENCES tblTask,"
        "parent       INT NOT NULL REFERENCES tblUser,"
        "name         TEXT NOT NULL CHECK (name <> ''),"
        "description  TEXT NOT NULL,"
        "isDelete     INT NOT NULL DEFAULT 0 CHECK (isDelete BETWEEN 0 AND 1));";
  QUERY(ss.str().c_str(), PGRES_COMMAND_OK);

  ss.str("");
  ss << "CREATE TABLE IF NOT EXISTS tblUPipelineTask("
        "id           SERIAL PRIMARY KEY,"
        "pipeline     INT NOT NULL REFERENCES tblUPipeline,"
        "taskTempl    INT NOT NULL REFERENCES tblUTaskTemplate,"
        "qtask        INT REFERENCES tblTaskQueue,"
        "priority     INT NOT NULL DEFAULT 1 CHECK (priority BETWEEN 1 AND 3),"
        "prevTasks    TEXT NOT NULL,"
        "nextTasks    TEXT NOT NULL,"
        "params       TEXT NOT NULL,"
        "screenRect   TEXT NOT NULL,"
        "isDelete     INT NOT NULL DEFAULT 0 CHECK (isDelete BETWEEN 0 AND 1));";
  QUERY(ss.str().c_str(), PGRES_COMMAND_OK);

#undef QUERY
}
DbPGProvider::~DbPGProvider(){
  if (_pg){
    PQfinish(_pg);
  }
}

// for manager
bool DbPGProvider::addUser(const ZM_Base::user& cng, uint64_t& outUserId){  
  stringstream ss;
  ss << "INSERT INTO tblUser (name, passwHash, description) VALUES("
        "'" << cng.name << "',"
        "MD5('" << cng.passw << "'),"
        "'" << cng.description << "') RETURNING id;";

  auto res = PQexec(_pg, ss.str().c_str());
  if (PQresultStatus(res) != PGRES_TUPLES_OK){
      errorMess(string("addUser error: ") + PQerrorMessage(_pg));
      PQclear(res);
      return false;
  }
  outUserId = atoll(PQgetvalue(res, 0, 0));
  PQclear(res); 
  return true;
}
bool DbPGProvider::getUserId(const std::string& name, const std::string& passw, uint64_t& outUserId){
  stringstream ss;
  ss << "SELECT id FROM tblUser "
        "WHERE name = '" << name << "' AND " << "passwHash = MD5('" << passw << "') AND isDelete = 0;";

  auto res = PQexec(_pg, ss.str().c_str());
  if ((PQresultStatus(res) != PGRES_TUPLES_OK) || (PQntuples(res) != 1)){
      errorMess(string("getUser error: ") + PQerrorMessage(_pg));
      PQclear(res);
      return false;
  }
  outUserId = atoll(PQgetvalue(res, 0, 0));
  PQclear(res); 
  return true;
}
bool DbPGProvider::getUserCng(uint64_t userId, ZM_Base::user& cng){
  stringstream ss;
  ss << "SELECT name, description FROM tblUser "
        "WHERE id = " << userId << " AND isDelete = 0;";

  auto res = PQexec(_pg, ss.str().c_str());
  if ((PQresultStatus(res) != PGRES_TUPLES_OK) || (PQntuples(res) != 1)){
      errorMess(string("getUser error: ") + PQerrorMessage(_pg));
      PQclear(res);
      return false;
  }
  cng.name = PQgetvalue(res, 0, 0);
  cng.description = PQgetvalue(res, 0, 1);  
  PQclear(res); 
  return true;
}
bool DbPGProvider::changeUser(uint64_t userId, const ZM_Base::user& newCng){
  stringstream ss;
  ss << "UPDATE tblUser SET "
        "name = '" << newCng.name << "',"
        "passwHash = MD5('" << newCng.passw << "'),"
        "description = '" << newCng.description << "' "
        "WHERE id = " << userId << ";";

  auto res = PQexec(_pg, ss.str().c_str());
  if (PQresultStatus(res) != PGRES_COMMAND_OK){
      errorMess(string("changeUser error: ") + PQerrorMessage(_pg));
      PQclear(res);
      return false;
  }  
  PQclear(res); 
  return true;
}
bool DbPGProvider::delUser(uint64_t userId){
  stringstream ss;
  ss << "UPDATE tblUser SET "
        "isDelete = 1 "
        "WHERE id = " << userId << ";";

  auto res = PQexec(_pg, ss.str().c_str());
  if (PQresultStatus(res) != PGRES_COMMAND_OK){
      errorMess(string("delUser error: ") + PQerrorMessage(_pg));
      PQclear(res);
      return false;
  }  
  PQclear(res); 
  return true;
}
std::vector<uint64_t> DbPGProvider::getAllUsers(){
  stringstream ss;
  ss << "SELECT id FROM tblUser "
        "WHERE isDelete = 0;";

  auto res = PQexec(_pg, ss.str().c_str());
  if (PQresultStatus(res) != PGRES_TUPLES_OK){
      errorMess(string("getAllUsers error: ") + PQerrorMessage(_pg));
      PQclear(res);
      return std::vector<uint64_t>();
  }  
  int rows = PQntuples(res);
  std::vector<uint64_t> ret(rows);
  for (int i = 0; i < rows; ++i){
    ret[i] = atoll(PQgetvalue(res, i, 0));
  }
  PQclear(res);
  return ret;
}

bool DbPGProvider::addSchedr(const ZM_Base::scheduler& schedl, uint64_t& outSchId){    
  auto connPnt = ZM_Aux::split(schedl.connectPnt, ":");
  if (connPnt.size() != 2){
    errorMess("addSchedr error: connectPnt not correct");
    return false;
  }
  stringstream ss;
  ss << "WITH ncp AS (INSERT INTO tblConnectPnt (ipAddr, port) VALUES("
        " '" << connPnt[0] << "',"
        " '" << connPnt[1] << "') RETURNING id)"
        "INSERT INTO tblScheduler (connPnt, state, capacityTask) VALUES("
        "(SELECT id FROM ncp),"
        "'" << (int)schedl.state << "',"
        "'" << schedl.capacityTask << "') RETURNING id;";

  auto res = PQexec(_pg, ss.str().c_str());
  if (PQresultStatus(res) != PGRES_TUPLES_OK){
      errorMess(string("addSchedr error: ") + PQerrorMessage(_pg));
      PQclear(res);
      return false;
  }
  outSchId = atoll(PQgetvalue(res, 0, 0));
  PQclear(res); 
  return true;
}
bool DbPGProvider::getSchedr(uint64_t schId, ZM_Base::scheduler& cng){
  stringstream ss;
  ss << "SELECT cp.ipAddr, cp.port, s.state, s.capacityTask FROM tblScheduler s "
        "JOIN tblConnectPnt cp ON cp.id = s.connPnt "
        "WHERE s.id = " << schId << " AND s.isDelete = 0;";

  auto res = PQexec(_pg, ss.str().c_str());
  if ((PQresultStatus(res) != PGRES_TUPLES_OK) || (PQntuples(res) != 1)){
      errorMess(string("getSchedr error: ") + PQerrorMessage(_pg));
      PQclear(res);
      return false;
  }
  cng.connectPnt = PQgetvalue(res, 0, 0) + string(":") + PQgetvalue(res, 0, 1);
  cng.state = (ZM_Base::stateType)atoi(PQgetvalue(res, 0, 2));
  cng.capacityTask = atoi(PQgetvalue(res, 0, 3));
  PQclear(res); 
  return true;
}
bool DbPGProvider::changeSchedr(uint64_t schId, const ZM_Base::scheduler& newCng){  
  auto connPnt = ZM_Aux::split(newCng.connectPnt, ":");
  if (connPnt.size() != 2){
    errorMess("changeSchedr error: connectPnt not correct");
    return false;
  }
  stringstream ss;
  ss << "UPDATE tblScheduler SET "
        "state = '" << (int)newCng.state << "',"
        "capacityTask = '" << newCng.capacityTask << "' "
        "WHERE id = " << schId << "; "

        "UPDATE tblConnectPnt SET "
        "ipAddr = '" << connPnt[0] << "',"
        "port = '" << connPnt[1] << "' "
        "WHERE id = (SELECT connPnt FROM tblScheduler WHERE id = " << schId << ");";

  auto res = PQexec(_pg, ss.str().c_str());
  if (PQresultStatus(res) != PGRES_COMMAND_OK){
      errorMess(string("changeSchedr error: ") + PQerrorMessage(_pg));
      PQclear(res);
      return false;
  }  
  PQclear(res); 
  return true;
}
bool DbPGProvider::delSchedr(uint64_t schId){  
  stringstream ss;
  ss << "UPDATE tblScheduler SET "
        "isDelete = 1 "
        "WHERE id = " << schId << ";";

  auto res = PQexec(_pg, ss.str().c_str());
  if (PQresultStatus(res) != PGRES_COMMAND_OK){
      errorMess(string("delSchedr error: ") + PQerrorMessage(_pg));
      PQclear(res);
      return false;
  }  
  PQclear(res); 
  return true;
}
bool DbPGProvider::schedrState(uint64_t schId, ZM_Base::stateType& state){
  stringstream ss;
  ss << "SELECT state FROM tblScheduler "
        "WHERE id = " << schId << " AND isDelete = 0;";

  auto res = PQexec(_pg, ss.str().c_str());
  if ((PQresultStatus(res) != PGRES_TUPLES_OK) || (PQntuples(res) != 1)){
      errorMess(string("schedrState error: ") + PQerrorMessage(_pg));
      PQclear(res);
      return false;
  }
  state = (ZM_Base::stateType)atoi(PQgetvalue(res, 0, 0));
  PQclear(res); 
  return true;
}
std::vector<uint64_t> DbPGProvider::getAllSchedrs(ZM_Base::stateType state){  
  stringstream ss;
  ss << "SELECT id FROM tblScheduler "
        "WHERE (state = " << (int)state << " OR " << (int)state << " = -1) AND isDelete = 0;";

  auto res = PQexec(_pg, ss.str().c_str());
  if (PQresultStatus(res) != PGRES_TUPLES_OK){
      errorMess(string("getAllSchedrs error: ") + PQerrorMessage(_pg));
      PQclear(res);
      return std::vector<uint64_t>();
  }  
  int rows = PQntuples(res);
  std::vector<uint64_t> ret(rows);
  for (int i = 0; i < rows; ++i){
    ret[i] = atoll(PQgetvalue(res, i, 0));
  }
  PQclear(res);
  return ret;
}

bool DbPGProvider::addWorker(const ZM_Base::worker& worker, uint64_t& outWkrId){
  auto connPnt = ZM_Aux::split(worker.connectPnt, ":");
  if (connPnt.size() != 2){
    errorMess("addWorker error: connectPnt not correct");
    return false;
  }
  stringstream ss;
  ss << "WITH ncp AS (INSERT INTO tblConnectPnt (ipAddr, port) VALUES("
        " '" << connPnt[0] << "',"
        " '" << connPnt[1] << "') RETURNING id)"
        "INSERT INTO tblWorker (connPnt, schedr, executor, state, capacityTask) VALUES("
        "(SELECT id FROM ncp),"
        "'" << (int)worker.sId << "',"
        "'" << (int)worker.exr << "',"
        "'" << (int)worker.state << "',"
        "'" << worker.capacityTask << "') RETURNING id;";

  auto res = PQexec(_pg, ss.str().c_str());
  if (PQresultStatus(res) != PGRES_TUPLES_OK){
      errorMess(string("addWorker error: ") + PQerrorMessage(_pg));
      PQclear(res);
      return false;
  }
  outWkrId = atoll(PQgetvalue(res, 0, 0));
  PQclear(res); 
  return true;
}
bool DbPGProvider::getWorker(uint64_t wkrId, ZM_Base::worker& cng){
  stringstream ss;
  ss << "SELECT cp.ipAddr, cp.port, w.schedr, w.executor, w.state, w.capacityTask FROM tblWorker w "
        "JOIN tblConnectPnt cp ON cp.id = w.connPnt "
        "WHERE w.id = " << wkrId << " AND w.isDelete = 0;";

  auto res = PQexec(_pg, ss.str().c_str());
  if ((PQresultStatus(res) != PGRES_TUPLES_OK) || (PQntuples(res) != 1)){
      errorMess(string("getWorker error: ") + PQerrorMessage(_pg));
      PQclear(res);
      return false;
  }
  cng.connectPnt = PQgetvalue(res, 0, 0) + string(":") + PQgetvalue(res, 0, 1);
  cng.sId = atoll(PQgetvalue(res, 0, 2));
  cng.exr = (ZM_Base::executorType)atoi(PQgetvalue(res, 0, 3));
  cng.state = (ZM_Base::stateType)atoi(PQgetvalue(res, 0, 4));
  cng.capacityTask = atoi(PQgetvalue(res, 0, 5));
  PQclear(res); 
  return true;
}
bool DbPGProvider::changeWorker(uint64_t wkrId, const ZM_Base::worker& newCng){
  auto connPnt = ZM_Aux::split(newCng.connectPnt, ":");
  if (connPnt.size() != 2){
    errorMess("changeWorker error: connectPnt not correct");
    return false;
  }
  stringstream ss;
  ss << "UPDATE tblWorker SET "
        "schedr = '" << (int)newCng.sId << "',"
        "executor = '" << (int)newCng.exr << "',"
        "state = '" << (int)newCng.state << "',"
        "capacityTask = '" << newCng.capacityTask << "' "
        "WHERE id = " << wkrId << "; "

        "UPDATE tblConnectPnt SET "
        "ipAddr = '" << connPnt[0] << "',"
        "port = '" << connPnt[1] << "' "
        "WHERE id = (SELECT connPnt FROM tblWorker WHERE id = " << wkrId << ");";

  auto res = PQexec(_pg, ss.str().c_str());
  if (PQresultStatus(res) != PGRES_COMMAND_OK){
      errorMess(string("changeWorker error: ") + PQerrorMessage(_pg));
      PQclear(res);
      return false;
  }  
  PQclear(res); 
  return true;
}
bool DbPGProvider::delWorker(uint64_t wkrId){
  stringstream ss;
  ss << "UPDATE tblWorker SET "
        "isDelete = 1 "
        "WHERE id = " << wkrId << ";";

  auto res = PQexec(_pg, ss.str().c_str());
  if (PQresultStatus(res) != PGRES_COMMAND_OK){
      errorMess(string("delWorker error: ") + PQerrorMessage(_pg));
      PQclear(res);
      return false;
  }  
  PQclear(res); 
  return true;
}
bool DbPGProvider::workerState(uint64_t wkrId, ZM_Base::stateType& state){
  stringstream ss;
  ss << "SELECT state FROM tblWorker "
        "WHERE id = " << wkrId << " AND isDelete = 0;";

  auto res = PQexec(_pg, ss.str().c_str());
  if ((PQresultStatus(res) != PGRES_TUPLES_OK) || (PQntuples(res) != 1)){
      errorMess(string("workerState error: ") + PQerrorMessage(_pg));
      PQclear(res);
      return false;
  }
  state = (ZM_Base::stateType)atoi(PQgetvalue(res, 0, 0));
  PQclear(res); 
  return true;
}
std::vector<uint64_t> DbPGProvider::getAllWorkers(uint64_t schId, ZM_Base::stateType state){
  stringstream ss;
  ss << "SELECT id FROM tblWorker "
        "WHERE (state = " << (int)state << " OR " << (int)state << " = -1) "
        "AND schedr = " << schId << " "
        "AND isDelete = 0;";

  auto res = PQexec(_pg, ss.str().c_str());
  if (PQresultStatus(res) != PGRES_TUPLES_OK){
      errorMess(string("getAllWorkers error: ") + PQerrorMessage(_pg));
      PQclear(res);
      return std::vector<uint64_t>();
  }  
  int rows = PQntuples(res);
  std::vector<uint64_t> ret(rows);
  for (int i = 0; i < rows; ++i){
    ret[i] = atoll(PQgetvalue(res, i, 0));
  }
  PQclear(res);
  return ret;
}

bool DbPGProvider::addPipeline(const ZM_Base::uPipeline& ppl, uint64_t& outPPLId){
  stringstream ss;
  ss << "INSERT INTO tblUPipeline (usr, name, description) VALUES("
        "'" << ppl.uId << "',"
        "'" << ppl.name << "',"
        "'" << ppl.description << "') RETURNING id;";

  auto res = PQexec(_pg, ss.str().c_str());
  if (PQresultStatus(res) != PGRES_TUPLES_OK){
      errorMess(string("addPipeline error: ") + PQerrorMessage(_pg));
      PQclear(res);
      return false;
  }
  outPPLId = atoll(PQgetvalue(res, 0, 0));
  PQclear(res); 
  return true;
}
bool DbPGProvider::getPipeline(uint64_t pplId, ZM_Base::uPipeline& cng){
  stringstream ss;
  ss << "SELECT usr, name, description FROM tblUPipeline "
        "WHERE id = " << pplId << " AND isDelete = 0;";

  auto res = PQexec(_pg, ss.str().c_str());
  if ((PQresultStatus(res) != PGRES_TUPLES_OK) || (PQntuples(res) != 1)){
      errorMess(string("getPipeline error: ") + PQerrorMessage(_pg));
      PQclear(res);
      return false;
  }
  cng.uId = atoll(PQgetvalue(res, 0, 0));
  cng.name = PQgetvalue(res, 0, 1);
  cng.description = PQgetvalue(res, 0, 2);  
  PQclear(res); 
  return true;
}
bool DbPGProvider::changePipeline(uint64_t pplId, const ZM_Base::uPipeline& newCng){
  stringstream ss;
  ss << "UPDATE tblUPipeline SET "
        "usr = '" << newCng.uId << "',"
        "name = '" << newCng.name << "',"
        "description = '" << newCng.description << "' "
        "WHERE id = " << pplId << ";";

  auto res = PQexec(_pg, ss.str().c_str());
  if (PQresultStatus(res) != PGRES_COMMAND_OK){
      errorMess(string("changePipeline error: ") + PQerrorMessage(_pg));
      PQclear(res);
      return false;
  }  
  PQclear(res); 
  return true;
}
bool DbPGProvider::delPipeline(uint64_t pplId){
  stringstream ss;
  ss << "UPDATE tblUPipeline SET "
        "isDelete = 1 "
        "WHERE id = " << pplId << ";";

  auto res = PQexec(_pg, ss.str().c_str());
  if (PQresultStatus(res) != PGRES_COMMAND_OK){
      errorMess(string("delPipeline error: ") + PQerrorMessage(_pg));
      PQclear(res);
      return false;
  }  
  PQclear(res); 
  return true;
}
std::vector<uint64_t> DbPGProvider::getAllPipelines(uint64_t userId){
  stringstream ss;
  ss << "SELECT id FROM tblUPipeline "
        "WHERE usr = " << userId << " AND isDelete = 0;";

  auto res = PQexec(_pg, ss.str().c_str());
  if (PQresultStatus(res) != PGRES_TUPLES_OK){
      errorMess(string("getAllPipelines error: ") + PQerrorMessage(_pg));
      PQclear(res);
      return std::vector<uint64_t>();
  }  
  int rows = PQntuples(res);
  std::vector<uint64_t> ret(rows);
  for (int i = 0; i < rows; ++i){
    ret[i] = atoll(PQgetvalue(res, i, 0));
  }
  PQclear(res);
  return ret;
}

bool DbPGProvider::addTaskTemplate(const ZM_Base::uTaskTemplate& cng, uint64_t& outTId){
  stringstream ss;
  ss << "WITH ntsk AS (INSERT INTO tblTask (script, executor, averDurationSec, maxDurationSec) VALUES("
        "'" << cng.base.script << "',"
        "'" << (int)cng.base.exr << "',"
        "'" << cng.base.averDurationSec << "',"
        "'" << cng.base.maxDurationSec << "') RETURNING id) "
        "INSERT INTO tblUTaskTemplate (task, parent, name, description) VALUES("
        "(SELECT id FROM ntsk),"
        "'" << cng.uId << "',"
        "'" << cng.name << "',"
        "'" << cng.description << "') RETURNING task;";

  auto res = PQexec(_pg, ss.str().c_str());
  if (PQresultStatus(res) != PGRES_TUPLES_OK){
      errorMess(string("addTaskTemplate error: ") + PQerrorMessage(_pg));
      PQclear(res);
      return false;
  }
  outTId = atoll(PQgetvalue(res, 0, 0));
  PQclear(res); 
  return true;
}
bool DbPGProvider::getTaskTemplate(uint64_t tId, ZM_Base::uTaskTemplate& outTCng){
  stringstream ss;
  ss << "SELECT tt.parent, tt.name, tt.description, t.script, t.executor, t.averDurationSec, t.maxDurationSec "
        "FROM tblUTaskTemplate tt "
        "JOIN tblTask t ON t.id = tt.task "
        "WHERE t.id = " << tId << " AND isDelete = 0;";

  auto res = PQexec(_pg, ss.str().c_str());
  if ((PQresultStatus(res) != PGRES_TUPLES_OK) || (PQntuples(res) != 1)){
      errorMess(string("getTaskTemplate error: ") + PQerrorMessage(_pg));
      PQclear(res);
      return false;
  }
  outTCng.uId = atoll(PQgetvalue(res, 0, 0));
  outTCng.name = PQgetvalue(res, 0, 1);
  outTCng.description = PQgetvalue(res, 0, 2);  
  outTCng.base.script = PQgetvalue(res, 0, 3);  
  outTCng.base.exr = (ZM_Base::executorType)atoi(PQgetvalue(res, 0, 4));
  outTCng.base.averDurationSec = atoi(PQgetvalue(res, 0, 5));
  outTCng.base.maxDurationSec = atoi(PQgetvalue(res, 0, 6));
  PQclear(res); 
  return true;
};
bool DbPGProvider::changeTaskTemplate(uint64_t tId, const ZM_Base::uTaskTemplate& newTCng, uint64_t& outTId){

  return delTaskTemplate(tId) && addTaskTemplate(newTCng, outTId);
}
bool DbPGProvider::delTaskTemplate(uint64_t tId){
  stringstream ss;
  ss << "UPDATE tblUTaskTemplate SET "
        "isDelete = 1 "
        "WHERE task = " << tId << ";";

  auto res = PQexec(_pg, ss.str().c_str());
  if (PQresultStatus(res) != PGRES_COMMAND_OK){
      errorMess(string("delTaskTemplate error: ") + PQerrorMessage(_pg));
      PQclear(res);
      return false;
  }  
  PQclear(res); 
  return true;
}
std::vector<uint64_t> DbPGProvider::getAllTaskTemplates(uint64_t parent){
  stringstream ss;
  ss << "SELECT task FROM tblUTaskTemplate "
        "WHERE parent = " << parent << " AND isDelete = 0;";

  auto res = PQexec(_pg, ss.str().c_str());
  if (PQresultStatus(res) != PGRES_TUPLES_OK){
      errorMess(string("getAllTaskTemplates error: ") + PQerrorMessage(_pg));
      PQclear(res);
      return std::vector<uint64_t>();
  }  
  int rows = PQntuples(res);
  std::vector<uint64_t> ret(rows);
  for (int i = 0; i < rows; ++i){
    ret[i] = atoll(PQgetvalue(res, i, 0));
  }
  PQclear(res);
  return ret;
}

bool DbPGProvider::addTask(const ZM_Base::uTask& cng, uint64_t& outTId){
  stringstream ss;
  for (auto t : cng.prevTasks){ // for check
    ss.str("");       
    ss << "SELECT id FROM tblUPipelineTask WHERE id = " << t << ";"; 
    auto res = PQexec(_pg, ss.str().c_str() );
    if ((PQresultStatus(res) != PGRES_TUPLES_OK) || (PQntuples(res) != 1)){
      errorMess("addTask error: prevTasks not found taskId " + to_string(t));
      PQclear(res);
      return false;
    }
  }
  for (auto t : cng.nextTasks){ // for check
    ss.str("");       
    ss << "SELECT id FROM tblUPipelineTask WHERE id = " << t << ";"; 
    auto res = PQexec(_pg, ss.str().c_str() );
    if ((PQresultStatus(res) != PGRES_TUPLES_OK) || (PQntuples(res) != 1)){
      errorMess("addTask error: nextTasks not found taskId " + to_string(t));
      PQclear(res);
      return false;
    }
  }
  string prevTasks;
  prevTasks = accumulate(cng.prevTasks.begin(), cng.prevTasks.end(), prevTasks,
                [](string& s, uint64_t v){
                  return s.empty() ? to_string(v) : s + " " + to_string(v);
                });  
  string nextTasks;
  nextTasks = accumulate(cng.nextTasks.begin(), cng.nextTasks.end(), nextTasks,
                [](string& s, uint64_t v){
                  return s.empty() ? to_string(v) : s + " " + to_string(v);
                });  
  ss.str("");
  ss << "INSERT INTO tblUPipelineTask (pipeline, taskTempl, priority, "
        "                              prevTasks, nextTasks, params, screenRect) VALUES("
        "'" << cng.pplId << "',"
        "'" << cng.base.tId << "',"
        "'" << cng.base.priority << "',"
        "'" << prevTasks << "',"
        "'" << nextTasks << "',"
        "'" << cng.base.params << "',"
        "'" << cng.rct.toString() << "') RETURNING id;";

  auto res = PQexec(_pg, ss.str().c_str());
  if (PQresultStatus(res) != PGRES_TUPLES_OK){
      errorMess(string("addTask error: ") + PQerrorMessage(_pg));
      PQclear(res);
      return false;
  }
  outTId = atoll(PQgetvalue(res, 0, 0));
  PQclear(res); 
  return true;
}
bool DbPGProvider::getTask(uint64_t tId, ZM_Base::uTask& outTCng){
  stringstream ss;
  ss << "SELECT pipeline, taskTempl, priority, prevTasks, nextTasks, params, screenRect "
        "FROM tblUPipelineTask "
        "WHERE id = " << tId << " AND isDelete = 0;";

  auto res = PQexec(_pg, ss.str().c_str());
  if ((PQresultStatus(res) != PGRES_TUPLES_OK) || (PQntuples(res) != 1)){
      errorMess(string("getTask error: ") + PQerrorMessage(_pg));
      PQclear(res);
      return false;
  }
  outTCng.pplId = atoll(PQgetvalue(res, 0, 0));
  outTCng.base.tId = atoll(PQgetvalue(res, 0, 1));
  outTCng.base.priority = atoi(PQgetvalue(res, 0, 2));  
  auto prevTasks = ZM_Aux::split(PQgetvalue(res, 0, 3), " ");
  for(auto& t : prevTasks){
    outTCng.prevTasks.push_back(stoll(t));
  }
  auto nextTasks = ZM_Aux::split(PQgetvalue(res, 0, 4), " ");
  for(auto& t : nextTasks){
    outTCng.nextTasks.push_back(stoll(t));
  }
  outTCng.base.params = PQgetvalue(res, 0, 5);
  auto rect = ZM_Aux::split(PQgetvalue(res, 0, 6), " ");
  outTCng.rct = {stoi(rect[0]), stoi(rect[1]), stoi(rect[2]), stoi(rect[3])};
  PQclear(res); 
  return true;
}
bool DbPGProvider::changeTask(uint64_t tId, const ZM_Base::uTask& newTCng){
  stringstream ss;
  for (auto t : newTCng.prevTasks){ // for check
    ss.str("");       
    ss << "SELECT id FROM tblUPipelineTask WHERE id = " << t << ";"; 
    auto res = PQexec(_pg, ss.str().c_str() );
    if ((PQresultStatus(res) != PGRES_TUPLES_OK) || (PQntuples(res) != 1)){
      errorMess("changeTask error: prevTasks not found taskId " + to_string(t));
      PQclear(res);
      return false;
    }
  }
  for (auto t : newTCng.nextTasks){ // for check
    ss.str("");       
    ss << "SELECT id FROM tblUPipelineTask WHERE id = " << t << ";"; 
    auto res = PQexec(_pg, ss.str().c_str() );
    if ((PQresultStatus(res) != PGRES_TUPLES_OK) || (PQntuples(res) != 1)){
      errorMess("changeTask error: nextTasks not found taskId " + to_string(t));
      PQclear(res);
      return false;
    }
  }
  string prevTasks;
  prevTasks = accumulate(newTCng.prevTasks.begin(), newTCng.prevTasks.end(), prevTasks,
                [](string& s, uint64_t v){
                  return s.empty() ? to_string(v) : s + " " + to_string(v);
                });  
  string nextTasks;
  nextTasks = accumulate(newTCng.nextTasks.begin(), newTCng.nextTasks.end(), nextTasks,
                [](string& s, uint64_t v){
                  return s.empty() ? to_string(v) : s + " " + to_string(v);
                });  
  ss.str("");
  ss << "UPDATE tblUPipelineTask SET "
        "pipeline = '" << newTCng.pplId << "',"
        "taskTempl = '" << newTCng.base.tId << "',"
        "priority = '" << newTCng.base.priority << "',"
        "prevTasks = '" << prevTasks << "',"
        "nextTasks = '" << nextTasks << "',"
        "params = '" << newTCng.base.params << "',"
        "screenRect = '" << newTCng.rct.toString() << "' "
        "WHERE id = " << tId << ";";

  auto res = PQexec(_pg, ss.str().c_str());
  if (PQresultStatus(res) != PGRES_COMMAND_OK){
      errorMess(string("changeTask error: ") + PQerrorMessage(_pg));
      PQclear(res);
      return false;
  }
  PQclear(res); 
  return true;
}
bool DbPGProvider::delTask(uint64_t tId){
  stringstream ss;
  ss << "UPDATE tblUPipelineTask SET "
        "isDelete = 1 "
        "WHERE id = " << tId << ";";

  auto res = PQexec(_pg, ss.str().c_str());
  if (PQresultStatus(res) != PGRES_COMMAND_OK){
      errorMess(string("delTask error: ") + PQerrorMessage(_pg));
      PQclear(res);
      return false;
  }  
  PQclear(res); 
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
bool DbPGProvider::getTasksForSchedr(uint64_t schedrId, std::vector<std::pair<ZM_Base::task, ZM_Base::queueTask>>&){
  return true;
}
bool DbPGProvider::getWorkersForSchedr(uint64_t schedrId, std::vector<ZM_Base::worker>&){
  return true;
}
bool DbPGProvider::getNewTasks(int maxTaskCnt, std::vector<std::pair<ZM_Base::task, ZM_Base::queueTask>>&){
  return true;
}
bool DbPGProvider::sendAllMessFromSchedr(uint64_t schedrId, std::vector<ZM_DB::messSchedr>&){
  return true;
}

#ifdef DEBUG
// for test
bool DbPGProvider::delAllUsers(){
  stringstream ss;
  ss << "TRUNCATE tblUser CASCADE;";

  auto res = PQexec(_pg, ss.str().c_str());
  if (PQresultStatus(res) != PGRES_COMMAND_OK){
      errorMess(PQerrorMessage(_pg));
      PQclear(res);
      return false;
  }
  return true;
}
bool DbPGProvider::delAllSchedrs(){
  stringstream ss;
  ss << "TRUNCATE tblScheduler CASCADE;";

  auto res = PQexec(_pg, ss.str().c_str());
  if (PQresultStatus(res) != PGRES_COMMAND_OK){
      errorMess(PQerrorMessage(_pg));
      PQclear(res);
      return false;
  }
  return true;
}
bool DbPGProvider::delAllWorkers(){
  stringstream ss;
  ss << "TRUNCATE tblWorker CASCADE;";

  auto res = PQexec(_pg, ss.str().c_str());
  if (PQresultStatus(res) != PGRES_COMMAND_OK){
      errorMess(PQerrorMessage(_pg));
      PQclear(res);
      return false;
  }
  return true;
}
bool DbPGProvider::delAllPipelines(){
  stringstream ss;
  ss << "TRUNCATE tblUPipeline CASCADE;";

  auto res = PQexec(_pg, ss.str().c_str());
  if (PQresultStatus(res) != PGRES_COMMAND_OK){
      errorMess(PQerrorMessage(_pg));
      PQclear(res);
      return false;
  }
  return true;
}
bool DbPGProvider::delAllTemplateTask(){
  stringstream ss;
  ss << "TRUNCATE tblUTaskTemplate CASCADE;";
  ss << "TRUNCATE tblTask CASCADE;";

  auto res = PQexec(_pg, ss.str().c_str());
  if (PQresultStatus(res) != PGRES_COMMAND_OK){
      errorMess(PQerrorMessage(_pg));
      PQclear(res);
      return false;
  }
  return true;
}
bool DbPGProvider::delAllTask(){
  stringstream ss;
  ss << "TRUNCATE tblUPipelineTask CASCADE;";
  
  auto res = PQexec(_pg, ss.str().c_str());
  if (PQresultStatus(res) != PGRES_COMMAND_OK){
      errorMess(PQerrorMessage(_pg));
      PQclear(res);
      return false;
  }
  return true;
}

#endif // DEBUG