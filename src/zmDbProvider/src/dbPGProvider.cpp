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
}
DbPGProvider::~DbPGProvider(){
  if (_pg){
    PQfinish(_pg);
  }
}
bool DbPGProvider::createTables(){
  lock_guard<mutex> lk(_mtx);
  #define QUERY(req, sts){              \
    auto res = PQexec(_pg, req);        \
    if (PQresultStatus(res) != sts){    \
        errorMess(string("createTables error: ") + PQerrorMessage(_pg)); \
        PQclear(res);                   \
        return false;                   \
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
        "progress     INT NOT NULL DEFAULT 0 CHECK (progress BETWEEN 0 AND 100),"
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
        "key          TEXT NOT NULL,"
        "sep          TEXT NOT NULL,"
        "value        TEXT NOT NULL);";
  QUERY(ss.str().c_str(), PGRES_COMMAND_OK);

  ss.str("");
  ss << "CREATE TABLE IF NOT EXISTS tblResult("
        "qtask        INT NOT NULL PRIMARY KEY REFERENCES tblTaskQueue,"
        "key          TEXT NOT NULL,"
        "sep          TEXT NOT NULL,"
        "value        TEXT NOT NULL);";
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
        "prevTasks    TEXT NOT NULL,"      // separated by space
        "nextTasks    TEXT NOT NULL,"  
        "params       TEXT[][3] NOT NULL," // [key, sep, val]
        "screenRect   TEXT NOT NULL,"
        "isDelete     INT NOT NULL DEFAULT 0 CHECK (isDelete BETWEEN 0 AND 1));";
  QUERY(ss.str().c_str(), PGRES_COMMAND_OK);
  
  ///////////////////////////////////////////////////////////////////////////
  /// INDEXES


  ///////////////////////////////////////////////////////////////////////////
  /// FUNCTIONS
  ss.str("");
  ss << "CREATE OR REPLACE FUNCTION "
        "funcAddTask(knownTasks int[], task tblUPipelineTask) "
        "RETURNS integer AS $$ "
        "DECLARE "
        "ret int := 0;"
        "t int;"
        "BEGIN"
        "  FOREACH t IN ARRAY knownTasks"
        "  LOOP"
        "    PERFORM id FROM tblUPipelineTask WHERE id = t;"
        "    IF NOT FOUND THEN"
        "      RETURN 0;"
        "    END IF;"
        "  END LOOP;"
        "  INSERT INTO tblUPipelineTask (pipeline, taskTempl, priority,"
        "                              prevTasks, nextTasks, params, screenRect)"
        "    VALUES(task.pipeline,"
        "           task.taskTempl,"
        "           task.priority,"
        "           task.prevTasks,"
        "           task.nextTasks,"
        "           task.params,"
        "           task.screenRect) RETURNING id INTO ret;"
        "  RETURN ret;" 
        "END;"
        "$$ LANGUAGE plpgsql;";
  QUERY(ss.str().c_str(), PGRES_COMMAND_OK);

  return true;
#undef QUERY
}

// for manager
bool DbPGProvider::addUser(const ZM_Base::user& cng, uint64_t& outUserId){  
  lock_guard<mutex> lk(_mtx);
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
  outUserId = stoull(PQgetvalue(res, 0, 0));
  PQclear(res); 
  return true;
}
bool DbPGProvider::getUserId(const std::string& name, const std::string& passw, uint64_t& outUserId){
  lock_guard<mutex> lk(_mtx);
  stringstream ss;
  ss << "SELECT id FROM tblUser "
        "WHERE name = '" << name << "' AND " << "passwHash = MD5('" << passw << "') AND isDelete = 0;";

  auto res = PQexec(_pg, ss.str().c_str());
  if ((PQresultStatus(res) != PGRES_TUPLES_OK) || (PQntuples(res) != 1)){
      errorMess(string("getUser error: ") + PQerrorMessage(_pg));
      PQclear(res);
      return false;
  }
  outUserId = stoull(PQgetvalue(res, 0, 0));
  PQclear(res); 
  return true;
}
bool DbPGProvider::getUserCng(uint64_t userId, ZM_Base::user& cng){
  lock_guard<mutex> lk(_mtx);
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
  lock_guard<mutex> lk(_mtx);
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
  lock_guard<mutex> lk(_mtx);
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
  lock_guard<mutex> lk(_mtx);
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
    ret[i] = stoull(PQgetvalue(res, i, 0));
  }
  PQclear(res);
  return ret;
}

bool DbPGProvider::addSchedr(const ZM_Base::scheduler& schedl, uint64_t& outSchId){    
  lock_guard<mutex> lk(_mtx);
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
  outSchId = stoull(PQgetvalue(res, 0, 0));
  PQclear(res); 
  return true;
}
bool DbPGProvider::getSchedr(uint64_t schId, ZM_Base::scheduler& cng){
  lock_guard<mutex> lk(_mtx);
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
  lock_guard<mutex> lk(_mtx);
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
  lock_guard<mutex> lk(_mtx);
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
  lock_guard<mutex> lk(_mtx);
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
  lock_guard<mutex> lk(_mtx);
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
    ret[i] = stoull(PQgetvalue(res, i, 0));
  }
  PQclear(res);
  return ret;
}

bool DbPGProvider::addWorker(const ZM_Base::worker& worker, uint64_t& outWkrId){
  lock_guard<mutex> lk(_mtx);
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
  outWkrId = stoull(PQgetvalue(res, 0, 0));
  PQclear(res); 
  return true;
}
bool DbPGProvider::getWorker(uint64_t wkrId, ZM_Base::worker& cng){
  lock_guard<mutex> lk(_mtx);
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
  cng.sId = stoull(PQgetvalue(res, 0, 2));
  cng.exr = (ZM_Base::executorType)atoi(PQgetvalue(res, 0, 3));
  cng.state = (ZM_Base::stateType)atoi(PQgetvalue(res, 0, 4));
  cng.capacityTask = atoi(PQgetvalue(res, 0, 5));
  PQclear(res); 
  return true;
}
bool DbPGProvider::changeWorker(uint64_t wkrId, const ZM_Base::worker& newCng){
  lock_guard<mutex> lk(_mtx);
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
  lock_guard<mutex> lk(_mtx);
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
  lock_guard<mutex> lk(_mtx);
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
  lock_guard<mutex> lk(_mtx);
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
    ret[i] = stoull(PQgetvalue(res, i, 0));
  }
  PQclear(res);
  return ret;
}

bool DbPGProvider::addPipeline(const ZM_Base::uPipeline& ppl, uint64_t& outPPLId){
  lock_guard<mutex> lk(_mtx);
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
  outPPLId = stoull(PQgetvalue(res, 0, 0));
  PQclear(res); 
  return true;
}
bool DbPGProvider::getPipeline(uint64_t pplId, ZM_Base::uPipeline& cng){
  lock_guard<mutex> lk(_mtx);
  stringstream ss;
  ss << "SELECT usr, name, description FROM tblUPipeline "
        "WHERE id = " << pplId << " AND isDelete = 0;";

  auto res = PQexec(_pg, ss.str().c_str());
  if ((PQresultStatus(res) != PGRES_TUPLES_OK) || (PQntuples(res) != 1)){
      errorMess(string("getPipeline error: ") + PQerrorMessage(_pg));
      PQclear(res);
      return false;
  }
  cng.uId = stoull(PQgetvalue(res, 0, 0));
  cng.name = PQgetvalue(res, 0, 1);
  cng.description = PQgetvalue(res, 0, 2);  
  PQclear(res); 
  return true;
}
bool DbPGProvider::changePipeline(uint64_t pplId, const ZM_Base::uPipeline& newCng){
  lock_guard<mutex> lk(_mtx);
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
  lock_guard<mutex> lk(_mtx);
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
  lock_guard<mutex> lk(_mtx);
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
    ret[i] = stoull(PQgetvalue(res, i, 0));
  }
  PQclear(res);
  return ret;
}

bool DbPGProvider::addTaskTemplate(const ZM_Base::uTaskTemplate& cng, uint64_t& outTId){
  lock_guard<mutex> lk(_mtx);
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
  outTId = stoull(PQgetvalue(res, 0, 0));
  PQclear(res); 
  return true;
}
bool DbPGProvider::getTaskTemplate(uint64_t tId, ZM_Base::uTaskTemplate& outTCng){
  lock_guard<mutex> lk(_mtx);
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
  outTCng.uId = stoull(PQgetvalue(res, 0, 0));
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
  lock_guard<mutex> lk(_mtx);
  return delTaskTemplate(tId) && addTaskTemplate(newTCng, outTId);
}
bool DbPGProvider::delTaskTemplate(uint64_t tId){
  lock_guard<mutex> lk(_mtx);
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
  lock_guard<mutex> lk(_mtx);
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
    ret[i] = stoull(PQgetvalue(res, i, 0));
  }
  PQclear(res);
  return ret;
}

bool DbPGProvider::addTask(const ZM_Base::uTask& cng, uint64_t& outTId){
  lock_guard<mutex> lk(_mtx);
  stringstream ss;
  ss << "DO $$BEGIN "
         
          "FOR r IN SELECT table_schema, table_name FROM information_schema.tables"
          "         WHERE table_type = 'VIEW' AND table_schema = 'public'"
          "LOOP"
          "    EXECUTE 'GRANT ALL ON ' || quote_ident(r.table_schema) || '.' || quote_ident(r.table_name) || ' TO webuser';"
          "END LOOP;"
        "END$$;";

  // for (auto t : cng.prevTasks){ // for check
  //   ss.str("");       
  //   ss << "SELECT id FROM tblUPipelineTask WHERE id = " << t << ";"; 
  //   auto res = PQexec(_pg, ss.str().c_str() );
  //   if ((PQresultStatus(res) != PGRES_TUPLES_OK) || (PQntuples(res) != 1)){
  //     errorMess("addTask error: prevTasks not found taskId " + to_string(t));
  //     PQclear(res);
  //     return false;
  //   }
  //   PQclear(res);
  // }
  // for (auto t : cng.nextTasks){ // for check
  //   ss.str("");       
  //   ss << "SELECT id FROM tblUPipelineTask WHERE id = " << t << ";"; 
  //   auto res = PQexec(_pg, ss.str().c_str() );
  //   if ((PQresultStatus(res) != PGRES_TUPLES_OK) || (PQntuples(res) != 1)){
  //     errorMess("addTask error: nextTasks not found taskId " + to_string(t));
  //     PQclear(res);
  //     return false;
  //   }
  //   PQclear(res);
  // }
  // string prevTasks;
  // prevTasks = accumulate(cng.prevTasks.begin(), cng.prevTasks.end(), prevTasks,
  //               [](string& s, uint64_t v){
  //                 return s.empty() ? to_string(v) : s + " " + to_string(v);
  //               });  
  // string nextTasks;
  // nextTasks = accumulate(cng.nextTasks.begin(), cng.nextTasks.end(), nextTasks,
  //               [](string& s, uint64_t v){
  //                 return s.empty() ? to_string(v) : s + " " + to_string(v);
  //               });  
  // ss.str("");
  // ss << "INSERT INTO tblUPipelineTask (pipeline, taskTempl, priority, "
  //       "                              prevTasks, nextTasks, params, screenRect) VALUES("
  //       "'" << cng.pplId << "',"
  //       "'" << cng.base.tId << "',"
  //       "'" << cng.base.priority << "',"
  //       "'" << prevTasks << "',"
  //       "'" << nextTasks << "',"
  //       "'" << cng.base.params << "',"
  //       "'" << cng.rct.toString() << "') RETURNING id;";

  // auto res = PQexec(_pg, ss.str().c_str());
  // if (PQresultStatus(res) != PGRES_TUPLES_OK){
  //     errorMess(string("addTask error: ") + PQerrorMessage(_pg));
  //     PQclear(res);
  //     return false;
  // }
  // outTId = stoull(PQgetvalue(res, 0, 0));
  // PQclear(res); 
  return true;
}
bool DbPGProvider::getTask(uint64_t tId, ZM_Base::uTask& outTCng){
  lock_guard<mutex> lk(_mtx);
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
  outTCng.pplId = stoull(PQgetvalue(res, 0, 0));
  outTCng.base.tId = stoull(PQgetvalue(res, 0, 1));
  outTCng.base.priority = atoi(PQgetvalue(res, 0, 2));  
  auto prevTasks = ZM_Aux::split(PQgetvalue(res, 0, 3), " ");
  for(auto& t : prevTasks){
    outTCng.prevTasks.push_back(stoll(t));
  }
  auto nextTasks = ZM_Aux::split(PQgetvalue(res, 0, 4), " ");
  for(auto& t : nextTasks){
    outTCng.nextTasks.push_back(stoll(t));
  }
  //outTCng.base.params = PQgetvalue(res, 0, 5);
  auto rect = ZM_Aux::split(PQgetvalue(res, 0, 6), " ");
  outTCng.rct = {stoi(rect[0]), stoi(rect[1]), stoi(rect[2]), stoi(rect[3])};
  PQclear(res); 
  return true;
}
bool DbPGProvider::changeTask(uint64_t tId, const ZM_Base::uTask& newTCng){
  lock_guard<mutex> lk(_mtx);
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
    PQclear(res);
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
    PQclear(res);
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
     //   "params = '" << newTCng.base.params << "',"
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
  lock_guard<mutex> lk(_mtx);
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
  lock_guard<mutex> lk(_mtx);
  ZM_Base::uTask cng;
  if (!getTask(tId, cng)){
    return false;
  }  
#define ROLLBACK(sts)                                              \
  if (PQresultStatus(res) != sts){                                 \
     errorMess(string("startTask error: ") + PQerrorMessage(_pg)); \
     PQexec(_pg, "ROLLBACK;");                                     \
     PQclear(res);                                                 \
     return false;                                                 \
  }  
  auto res = PQexec(_pg, "BEGIN;");
  ROLLBACK(PGRES_COMMAND_OK);
  PQclear(res);
  /////////////////////////////////////////////////////////////////////////////
  stringstream ss;
  ss << "INSERT INTO tblTaskQueue (task, state, launcher, priority, isDependence) VALUES("
        "'" << cng.base.tId << "',"
        "'" << (int)ZM_Base::stateType::ready << "',"
        "(SELECT usr FROM tblUPipeline WHERE id = " << cng.pplId << "),"
        "'" << cng.base.priority << "',"
        "'" << (cng.prevTasks.empty() ? 0 : 1) << "') RETURNING id;";

  res = PQexec(_pg, ss.str().c_str());
  ROLLBACK(PGRES_TUPLES_OK);
  
  uint64_t tqId = stoull(PQgetvalue(res, 0, 0));
  PQclear(res);   
  /////////////////////////////////////////////////////////////////////////////
  // if (!cng.base.params.empty()){
  //   auto params = ZM_Aux::split(cng.base.params, "-");
  //   map<string, string> keyVal;
  //   for (auto& p : params){
  //     auto kv = ZM_Aux::split(p, "=");
  //     keyVal[kv[0]] = kv[1];
  //   }
  //   ss.str("");
  //   for (auto& kv : keyVal){
  //     ss << "INSERT INTO tblParam (qtask, key, value) VALUES("
  //           "'" << tqId << "',"
  //           "'" << kv.first << "',"
  //           "'" << kv.second << "');";
  //   }    
  //   res = PQexec(_pg, ss.str().c_str());
  //   ROLLBACK(PGRES_COMMAND_OK);
  //   PQclear(res);
  // }
  ///////////////////////////////////////////////////////////////////////////// 
  if (!cng.prevTasks.empty()){
    ss.str("");
    for (auto pt : cng.prevTasks){
      ss << "INSERT INTO tblPrevTask (qtask, prevTask) VALUES("
            "'" << tqId << "',"
            "'" << pt << "');";
    }
    res = PQexec(_pg, ss.str().c_str());
    ROLLBACK(PGRES_COMMAND_OK);
    PQclear(res);
  }
  ///////////////////////////////////////////////////////////////////////////// 
  ss.str("");
  ss << "UPDATE tblUPipelineTask SET "
          "qtask = '" << tqId << "' "
          "WHERE id = " << tId << ";";
  res = PQexec(_pg, ss.str().c_str());
  ROLLBACK(PGRES_COMMAND_OK);
  PQclear(res);
  ///////////////////////////////////////////////////////////////////////////// 
  res = PQexec(_pg, "COMMIT;");
  ROLLBACK(PGRES_COMMAND_OK);
  PQclear(res);
  return true;
#undef ROLLBACK
}
bool DbPGProvider::getTaskState(uint64_t tId, ZM_Base::queueTask& outState){
  lock_guard<mutex> lk(_mtx);
  stringstream ss;
  ss << "SELECT progress, state FROM tblTaskQueue "
        "WHERE id = (SELECT qtask FROM tblUPipelineTask WHERE id = " << tId << " AND isDelete = 0);";

  auto res = PQexec(_pg, ss.str().c_str());
  if ((PQresultStatus(res) != PGRES_TUPLES_OK) || (PQntuples(res) != 1)){      
    if ((PQresultStatus(res) == PGRES_TUPLES_OK) && (PQntuples(res) == 0)){
      outState.progress = 0;
      outState.state = ZM_Base::stateType::undefined;
     // outState.result = "";
      PQclear(res);
      return true;
    }else{
      errorMess(string("getTaskState error: ") + PQerrorMessage(_pg));
      PQclear(res);        
      return false;
    }
  }
  outState.progress = atoi(PQgetvalue(res, 0, 0));
  outState.state = (ZM_Base::stateType)atoi(PQgetvalue(res, 0, 1));
  PQclear(res); 

  if ((outState.state == ZM_Base::stateType::completed) ||
      (outState.state == ZM_Base::stateType::error) ||
      (outState.state == ZM_Base::stateType::stop)){
    ss.str("");
    ss << "SELECT key, value FROM tblResult "
          "WHERE qtask = (SELECT qtask FROM tblUPipelineTask WHERE id = " << tId << ");";

    auto res = PQexec(_pg, ss.str().c_str());
    if ((PQresultStatus(res) != PGRES_TUPLES_OK) || (PQntuples(res) != 1)){
      if ((PQresultStatus(res) == PGRES_TUPLES_OK) && (PQntuples(res) == 0)){
      //  outState.result = "";
        PQclear(res);
        return true;
      }else{
        errorMess(string("getTaskState error: ") + PQerrorMessage(_pg));
        PQclear(res);        
        return false;
      }
    }
   // outState.result = string("-") + PQgetvalue(res, 0, 0) + "=" + PQgetvalue(res, 0, 1);
    PQclear(res); 
  }  
  return true;
}
std::vector<uint64_t> DbPGProvider::getAllTasks(uint64_t pplId, ZM_Base::stateType state){  
  lock_guard<mutex> lk(_mtx);
  stringstream ss;
  ss << "SELECT ut.id FROM tblUPipelineTask ut "
        "LEFT JOIN tblTaskQueue qt ON qt.id = ut.qtask "
        "WHERE (qt.state = " << (int)state << " OR " << (int)state << " = -1) "
          "AND ut.pipeline = " << pplId << " AND ut.isDelete = 0;";

  auto res = PQexec(_pg, ss.str().c_str());
  if (PQresultStatus(res) != PGRES_TUPLES_OK){
      errorMess(string("getAllTasks error: ") + PQerrorMessage(_pg));
      PQclear(res);
      return std::vector<uint64_t>();
  }  
  int rows = PQntuples(res);
  std::vector<uint64_t> ret(rows);
  for (int i = 0; i < rows; ++i){
    ret[i] = stoull(PQgetvalue(res, i, 0));
  }
  PQclear(res);
  return ret;
}

// for zmSchedr
bool DbPGProvider::getSchedr(std::string& connPnt, ZM_Base::scheduler& outCng){
  lock_guard<mutex> lk(_mtx);
  auto cp = ZM_Aux::split(connPnt, ":");
  if (cp.size() != 2){
    errorMess(string("getSchedr error: connPnt not correct"));
    return false;
  }
  stringstream ss;
  ss << "SELECT s.id, s.state, s.capacityTask, s.activeTask FROM tblScheduler s "
        "JOIN tblConnectPnt cp ON cp.id = s.connPnt "
        "WHERE cp.ipAddr = '" << cp[0] << "' AND cp.port = '" << cp[1] << "' AND s.isDelete = 0;";

  auto res = PQexec(_pg, ss.str().c_str());
  if ((PQresultStatus(res) != PGRES_TUPLES_OK) || (PQntuples(res) == 0)){
    errorMess(string("getSchedr error: ") + PQerrorMessage(_pg));
    PQclear(res);
    return false;
  }
  outCng.id = stoull(PQgetvalue(res, 0, 0));
  outCng.connectPnt = connPnt;
  outCng.state = (ZM_Base::stateType)atoi(PQgetvalue(res, 0, 1));
  outCng.capacityTask = atoi(PQgetvalue(res, 0, 2));
  outCng.activeTask = atoi(PQgetvalue(res, 0, 3));
  PQclear(res); 
  return true;
}
bool DbPGProvider::getTasksForSchedr(uint64_t sId, std::vector<ZM_DB::schedrTask>& out){
  lock_guard<mutex> lk(_mtx);
  stringstream ss;
  ss << "SELECT t.id, t.executor, t.averDurationSec, t.maxDurationSec, t.script, tq.id "
        "FROM tblTask t "
        "JOIN tblTaskQueue tq ON tq.task = t.id "
        "WHERE tq.schedr = " << sId << " AND (tq.state BETWEEN 1 AND 3);"; // 1 - start, 2 - running, 3 - pause

  auto resTsk = PQexec(_pg, ss.str().c_str());
  if (PQresultStatus(resTsk) != PGRES_TUPLES_OK){
    errorMess(string("getTasksForSchedr error: ") + PQerrorMessage(_pg));
    PQclear(resTsk);
    return false;
  }
  int tsz = PQntuples(resTsk);
  for (int i = 0; i < tsz; ++i){    
    uint64_t qId = stoull(PQgetvalue(resTsk, i, 5));
    ss.str("");
    ss << "SELECT key, value FROM tblParam "
          "WHERE qtask = " << qId << ";";

    auto resPrm = PQexec(_pg, ss.str().c_str());
    if (PQresultStatus(resPrm) != PGRES_TUPLES_OK){
      errorMess(string("getTasksForSchedr error: ") + PQerrorMessage(_pg));
      PQclear(resPrm);
      PQclear(resTsk);
      return false;
    }
    string prms;
    int psz = PQntuples(resPrm);
    for (int j = 0; j < psz; ++j){    
      prms += string("-") + PQgetvalue(resTsk, j, 0) + "=" + PQgetvalue(resTsk, j, 1);
    }
    PQclear(resPrm);
    out.push_back(make_pair(
      ZM_Base::task{ stoull(PQgetvalue(resTsk, i, 0)),
                     (ZM_Base::executorType)atoi(PQgetvalue(resTsk, i, 1)),
                     atoi(PQgetvalue(resTsk, i, 2)),
                     atoi(PQgetvalue(resTsk, i, 3)),
                     PQgetvalue(resTsk, i, 4)
                   },
                   prms));
  }
  PQclear(resTsk);
  return true;
}
bool DbPGProvider::getWorkersForSchedr(uint64_t sId, std::vector<ZM_Base::worker>& out){
  lock_guard<mutex> lk(_mtx);
  stringstream ss;
  ss << "SELECT w.id, w.state, w.executor, w.capacityTask, w.activeTask, w.rating, cp.ipAddr, cp.port "
        "FROM tblWorker w "
        "JOIN tblConnectPnt cp ON cp.id = w.connPnt "
        "WHERE w.schedr = " << sId << " AND w.isDelete = 0;";

  auto res = PQexec(_pg, ss.str().c_str());
  if (PQresultStatus(res) != PGRES_TUPLES_OK){
    errorMess(string("getWorkersForSchedr error: ") + PQerrorMessage(_pg));
    PQclear(res);
    return false;
  }
  int wsz = PQntuples(res);
  for(int i =0; i < wsz; ++i){
    out.push_back(ZM_Base::worker{ stoull(PQgetvalue(res, i, 0)),
                                   sId,
                                   (ZM_Base::stateType)atoi(PQgetvalue(res, i, 1)),
                                   (ZM_Base::executorType)atoi(PQgetvalue(res, i, 2)),
                                   atoi(PQgetvalue(res, i, 3)),
                                   atoi(PQgetvalue(res, i, 4)),
                                   atoi(PQgetvalue(res, i, 5)),
                                   PQgetvalue(res, i, 6) + string(":") + PQgetvalue(res, i, 7)});
  }
  PQclear(res);
  return true;
}
bool DbPGProvider::getNewTasks(uint64_t sId, int maxTaskCnt, std::vector<ZM_DB::schedrTask>& out){
  lock_guard<mutex> lk(_mtx);
  // #define ROLLBACK(res, sts)                                         \
  // if (PQresultStatus(res) != sts){                                   \
  //    errorMess(string("getNewTasks error: ") + PQerrorMessage(_pg)); \
  //    PQexec(_pg, "ROLLBACK;");                                       \
  //    PQclear(res);                                                   \
  //    return false;                                                   \
  // }  
  // auto resBegin = PQexec(_pg, "BEGIN;");
  // ROLLBACK(resBegin, PGRES_COMMAND_OK);
  // PQclear(resBegin);
  
  // stringstream ss;
  // ss << "SELECT tq.id, tq.isDependence FROM tblTaskQueue tq "
  //       "LEFT JOIN tblPrevTask pt ON pt.qtask = tq.id "
  //       "WHERE tq.state = " << (int)ZM_Base::stateType::ready << " LIMIT " << maxTaskCnt << ";";
  
  // auto resTsk = PQexec(_pg, ss.str().c_str());
  // ROLLBACK(resTsk, PGRES_TUPLES_OK);
 
  // int tsz = PQntuples(resTsk);
  // for (int i = 0; i < tsz; ++i){    
  //   uint64_t qId = stoull(PQgetvalue(resTsk, i, 0));
  //   bool isDependence = atoi(PQgetvalue(resTsk, i, 1)) == 1;
  //   if (isDependence){
  //     ss.str("");
  //     ss << "SELECT pt.id FROM tblPrevTask pt "
  //           "JOIN tblTaskQueue tq ON tq.id = pt.prevTask "
  //           "WHERE pt.qtask = " << qId << " AND tq.state = " << (int)ZM_Base::stateType::completed << ";";

  //     auto resDep = PQexec(_pg, ss.str().c_str());
  //     if (PQresultStatus(resDep) != PGRES_TUPLES_OK){
  //       errorMess(string("getNewTasks error: ") + PQerrorMessage(_pg));
  //       PQclear(resDep);
  //       PQclear(resTsk);
  //       return false;
  //     }
  //   }
  //   string prms;
  //   int psz = PQntuples(resPrm);
  //   for (int j = 0; j < psz; ++j){    
  //     prms += string("-") + PQgetvalue(resTsk, j, 0) + "=" + PQgetvalue(resTsk, j, 1);
  //   }
  //   PQclear(resPrm);
  //   out.push_back(make_pair(
  //     ZM_Base::task{ stoull(PQgetvalue(resTsk, i, 0)),
  //                    (ZM_Base::executorType)atoi(PQgetvalue(resTsk, i, 1)),
  //                    atoi(PQgetvalue(resTsk, i, 2)),
  //                    atoi(PQgetvalue(resTsk, i, 3)),
  //                    PQgetvalue(resTsk, i, 4)
  //                  },
  //                  prms));
  // }
  // PQclear(resTsk);
  return true;
}
bool DbPGProvider::sendAllMessFromSchedr(uint64_t schedrId, std::vector<ZM_DB::messSchedr>&){
  lock_guard<mutex> lk(_mtx);
  return true;
}

#ifdef DEBUG
// for test
bool DbPGProvider::delAllUsers(){
  lock_guard<mutex> lk(_mtx);
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
  lock_guard<mutex> lk(_mtx);
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
  lock_guard<mutex> lk(_mtx);
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
  lock_guard<mutex> lk(_mtx);
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
  lock_guard<mutex> lk(_mtx);
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
  lock_guard<mutex> lk(_mtx);
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