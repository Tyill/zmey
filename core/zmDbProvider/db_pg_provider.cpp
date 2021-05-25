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
#include <libpq-fe.h>

#include "zmCommon/aux_func.h"
#include "db_provider.h"

using namespace std;

namespace ZM_DB{

class PGres{
public:
  PGresult* res = nullptr;
  PGres(PGresult* _res):res(_res){}
  ~PGres(){
    if (res){
      PQclear(res);
    }
  }  
  PGres& operator=(PGres& npgr){
    if (res){
      PQclear(res);
    }
    res = npgr.res;
    npgr.res = nullptr;
    return *this;
  }
  PGres& operator=(PGres&& npgr){
    if (res){
      PQclear(res);
    }
    res = npgr.res;
    npgr.res = nullptr;
    return *this;
  }
};

#define _pg static_cast<PGconn*>(_db)

DbProvider::DbProvider(const ZM_DB::ConnectCng& cng)
  : _connCng(cng){ 

  _db = (PGconn*)PQconnectdb(cng.connectStr.c_str());
  if (PQstatus(_pg) != CONNECTION_OK){
    errorMess(PQerrorMessage(_pg));
    return;
  }
}
DbProvider::~DbProvider(){  
  if (_thrEndTask.joinable()){
    _fClose = true;   
    {lock_guard<mutex> lk(_mtxNotifyTask);  
      _cvNotifyTask.notify_one();
    }  
    _thrEndTask.join();
  }
  if (_pg){
    PQfinish(_pg);
  }
}
bool DbProvider::createTables(){
  lock_guard<mutex> lk(_mtx);
  #define QUERY(req, sts){              \
    PGres pgr(PQexec(_pg, req));        \
    if (PQresultStatus(pgr.res) != sts){  \
      errorMess(string("createTables error: ") + PQerrorMessage(_pg)); \
      return false;                     \
    }                                   \
  }
  QUERY("SELECT pg_catalog.set_config('search_path', 'public', false)", PGRES_TUPLES_OK);

  ///////////////////////////////////////////////////////////////////////////
  /// USER TABLES

  stringstream ss;
  ss << "CREATE TABLE IF NOT EXISTS tblUser("
        "id           SERIAL PRIMARY KEY,"
        "isDelete     INT NOT NULL DEFAULT 0 CHECK (isDelete BETWEEN 0 AND 1),"
        "name         TEXT NOT NULL UNIQUE CHECK (name <> ''),"
        "passwHash    TEXT NOT NULL,"
        "description  TEXT NOT NULL);";
  QUERY(ss.str().c_str(), PGRES_COMMAND_OK);
    
  ss.str("");
  ss << "CREATE TABLE IF NOT EXISTS tblUPipeline("
        "id           SERIAL PRIMARY KEY,"
        "usr          INT NOT NULL REFERENCES tblUser,"
        "isDelete     INT NOT NULL DEFAULT 0 CHECK (isDelete BETWEEN 0 AND 1),"
        "name         TEXT NOT NULL CHECK (name <> ''),"
        "description  TEXT NOT NULL);";
  QUERY(ss.str().c_str(), PGRES_COMMAND_OK);

  ss.str("");
  ss << "CREATE TABLE IF NOT EXISTS tblUTaskGroup("
        "id           SERIAL PRIMARY KEY,"
        "pipeline     INT NOT NULL REFERENCES tblUPipeline,"
        "isDelete     INT NOT NULL DEFAULT 0 CHECK (isDelete BETWEEN 0 AND 1),"
        "name         TEXT NOT NULL CHECK (name <> ''),"
        "description  TEXT NOT NULL);";        
  QUERY(ss.str().c_str(), PGRES_COMMAND_OK);

  ss.str("");
  ss << "CREATE TABLE IF NOT EXISTS tblUTaskTemplate("
        "id           SERIAL PRIMARY KEY,"
        "usr          INT NOT NULL REFERENCES tblUser,"
        "name         TEXT NOT NULL CHECK (name <> ''),"
        "description  TEXT NOT NULL,"
        "script       TEXT NOT NULL CHECK (script <> ''),"
        "averDurationSec INT NOT NULL CHECK (averDurationSec > 0),"
        "maxDurationSec  INT NOT NULL CHECK (maxDurationSec > 0),"
        "isDelete     INT NOT NULL DEFAULT 0 CHECK (isDelete BETWEEN 0 AND 1));";
  QUERY(ss.str().c_str(), PGRES_COMMAND_OK);

  ss.str("");
  ss << "CREATE TABLE IF NOT EXISTS tblUPipelineTask("
        "id           SERIAL PRIMARY KEY,"
        "pipeline     INT NOT NULL REFERENCES tblUPipeline,"
        "taskTempl    INT NOT NULL REFERENCES tblUTaskTemplate,"
        "taskGroup    INT REFERENCES tblUTaskGroup,"
        "priority     INT NOT NULL DEFAULT 1 CHECK (priority BETWEEN 1 AND 3),"
        "isDelete     INT NOT NULL DEFAULT 0 CHECK (isDelete BETWEEN 0 AND 1));";
  QUERY(ss.str().c_str(), PGRES_COMMAND_OK);
    
  /////////////////////////////////////////////////////////////////////////////
  /// SYSTEM TABLES
  ss.str("");
  ss << "CREATE TABLE IF NOT EXISTS tblConnectPnt("
        "id           SERIAL PRIMARY KEY,"
        "ipAddr       TEXT NOT NULL CHECK (ipAddr <> ''),"
        "port         INT NOT NULL CHECK (port > 0));";
  QUERY(ss.str().c_str(), PGRES_COMMAND_OK);
    
  ss.str("");
  ss << "CREATE TABLE IF NOT EXISTS tblState("
        "id           SERIAL PRIMARY KEY,"
        "kind         TEXT NOT NULL CHECK (kind <> ''));";
  QUERY(ss.str().c_str(), PGRES_COMMAND_OK);
    
  ss.str("");
  ss << "INSERT INTO tblState VALUES"
        "(" << (int)ZM_Base::StateType::UNDEFINED << ", 'undefined'),"
        "(" << (int)ZM_Base::StateType::READY << ", 'ready'),"
        "(" << (int)ZM_Base::StateType::START << ", 'start'),"
        "(" << (int)ZM_Base::StateType::RUNNING << ", 'running'),"
        "(" << (int)ZM_Base::StateType::PAUSE << ", 'pause'),"
        "(" << (int)ZM_Base::StateType::STOP << ", 'stop'),"
        "(" << (int)ZM_Base::StateType::COMPLETED << ", 'completed'),"
        "(" << (int)ZM_Base::StateType::ERROR << ", 'error'),"
        "(" << (int)ZM_Base::StateType::CANCEL << ", 'cancel'),"
        "(" << (int)ZM_Base::StateType::NOT_RESPONDING << ", 'notResponding') ON CONFLICT (id) DO NOTHING;";
  QUERY(ss.str().c_str(), PGRES_COMMAND_OK);

  ss.str("");
  ss << "CREATE TABLE IF NOT EXISTS tblScheduler("
        "id           SERIAL PRIMARY KEY,"
        "connPnt      INT NOT NULL REFERENCES tblConnectPnt,"
        "state        INT NOT NULL REFERENCES tblState,"
        "capacityTask INT NOT NULL DEFAULT 10000 CHECK (capacityTask > 0),"
        "activeTask   INT NOT NULL DEFAULT 0 CHECK (activeTask >= 0),"
        "isDelete     INT NOT NULL DEFAULT 0 CHECK (isDelete BETWEEN 0 AND 1),"
        "startTime    TIMESTAMP NOT NULL DEFAULT current_timestamp,"
        "stopTime     TIMESTAMP NOT NULL DEFAULT current_timestamp,"
        "internalData TEXT NOT NULL DEFAULT '');";
  QUERY(ss.str().c_str(), PGRES_COMMAND_OK);

  ss.str("");
  ss << "CREATE TABLE IF NOT EXISTS tblWorker("
        "id           SERIAL PRIMARY KEY,"
        "connPnt      INT NOT NULL REFERENCES tblConnectPnt,"
        "schedr       INT NOT NULL REFERENCES tblScheduler,"
        "state        INT NOT NULL REFERENCES tblState,"
        "capacityTask INT NOT NULL DEFAULT 10 CHECK (capacityTask > 0),"
        "activeTask   INT NOT NULL DEFAULT 0 CHECK (activeTask >= 0),"
        "load         INT NOT NULL DEFAULT 0 CHECK (load BETWEEN 0 AND 100),"
        "isDelete     INT NOT NULL DEFAULT 0 CHECK (isDelete BETWEEN 0 AND 1),"
        "startTime    TIMESTAMP NOT NULL DEFAULT current_timestamp,"
        "stopTime     TIMESTAMP NOT NULL DEFAULT current_timestamp);";
  QUERY(ss.str().c_str(), PGRES_COMMAND_OK);

  ss.str("");
  ss << "CREATE TABLE IF NOT EXISTS tblInternError("
        "id           SERIAL PRIMARY KEY,"
        "schedr       INT REFERENCES tblScheduler,"
        "worker       INT REFERENCES tblWorker,"
        "createTime   TIMESTAMP NOT NULL DEFAULT current_timestamp,"
        "message      TEXT NOT NULL);";
  QUERY(ss.str().c_str(), PGRES_COMMAND_OK);

  ss.str("");
  ss << "CREATE TABLE IF NOT EXISTS tblTaskQueue("
        "id           SERIAL PRIMARY KEY,"
        "taskTempl    INT NOT NULL REFERENCES tblUTaskTemplate,"      
        "plTask       INT NOT NULL REFERENCES tblUPipelineTask,"
        "schedr       INT REFERENCES tblScheduler,"
        "worker       INT REFERENCES tblWorker);";
  QUERY(ss.str().c_str(), PGRES_COMMAND_OK);

  ss.str("");
  ss << "CREATE TABLE IF NOT EXISTS tblTaskState("
        "qtask        INT PRIMARY KEY REFERENCES tblTaskQueue,"        
        "state        INT NOT NULL REFERENCES tblState,"
        "progress     INT NOT NULL DEFAULT 0 CHECK (progress BETWEEN 0 AND 100));";
  QUERY(ss.str().c_str(), PGRES_COMMAND_OK);

  ss.str("");
  ss << "CREATE TABLE IF NOT EXISTS tblTaskTime("
        "qtask          INT PRIMARY KEY REFERENCES tblTaskQueue,"        
        "createTime     TIMESTAMP NOT NULL DEFAULT current_timestamp,"
        "takeInWorkTime TIMESTAMP CHECK (takeInWorkTime >= createTime),"
        "startTime      TIMESTAMP CHECK (startTime >= takeInWorkTime),"
        "stopTime       TIMESTAMP CHECK (stopTime >= startTime));";
  QUERY(ss.str().c_str(), PGRES_COMMAND_OK);

  ss.str("");
  ss << "CREATE TABLE IF NOT EXISTS tblTaskParam("
        "qtask        INT PRIMARY KEY REFERENCES tblTaskQueue,"        
        "priority     INT NOT NULL DEFAULT 1 CHECK (priority BETWEEN 1 AND 3),"
        "params       TEXT[] NOT NULL);"; // ['param1', 'param2'..]
  QUERY(ss.str().c_str(), PGRES_COMMAND_OK);

  ss.str("");
  ss << "CREATE TABLE IF NOT EXISTS tblTaskResult("
        "qtask        INT PRIMARY KEY REFERENCES tblTaskQueue,"
        "result       TEXT NOT NULL);";
  QUERY(ss.str().c_str(), PGRES_COMMAND_OK);

  ss.str("");
  ss << "CREATE TABLE IF NOT EXISTS tblPrevTask("
        "qtask        INT PRIMARY KEY REFERENCES tblTaskQueue,"        
        "prevTasks    INT[] NOT NULL);";
  QUERY(ss.str().c_str(), PGRES_COMMAND_OK);
  
  ///////////////////////////////////////////////////////////////////////////
  /// INDEXES
  ss.str(""); 
  ss << "CREATE INDEX IF NOT EXISTS inxTSState ON tblTaskState(state);"
        "CREATE INDEX IF NOT EXISTS inxPPTQTaskTempl ON tblUPipelineTask(taskTempl);"
        "CREATE INDEX IF NOT EXISTS inxTQWorker ON tblTaskQueue(worker);"
        "CREATE INDEX IF NOT EXISTS inxIECreateTime ON tblInternError(createTime);"
        "CREATE INDEX IF NOT EXISTS inxTTCreateTime ON tblTaskTime(createTime);";

  ///////////////////////////////////////////////////////////////////////////
  /// FUNCTIONS
  ss.str("");
  ss << "CREATE OR REPLACE FUNCTION "
        "funcStartTask(ptId int, tParams TEXT[], prvTasks int[]) "
        "RETURNS int AS $$ "
        "DECLARE "
        "  ptask tblUPipelineTask;"
        "  qId int := 0;"
        "BEGIN"
        "  SELECT * INTO ptask "
        "  FROM tblUPipelineTask "
        "  WHERE id = ptId AND isDelete = 0;"
        "  IF NOT FOUND THEN"
        "    RETURN 0;"
        "  END IF;"
        
        "  INSERT INTO tblTaskQueue (taskTempl, plTask) VALUES("
        "    ptask.taskTempl,"
        "    ptId) RETURNING id INTO qId;"
        
        "  INSERT INTO tblTaskTime (qtask) VALUES("
        "    qId);"

        "  INSERT INTO tblTaskParam (qtask, priority, params) VALUES("
        "    qId,"
        "    ptask.priority,"
        "    tParams);"

        "  INSERT INTO tblTaskResult (qtask, result) VALUES("
        "    qId,"
        "    '');"
       
        "  INSERT INTO tblPrevTask (qtask, prevTasks) VALUES("
        "    qId,"
        "    prvTasks);"
       
        "  INSERT INTO tblTaskState (qtask, state) VALUES("
        "    qId,"
        "    0);"  // ready

        "  RETURN qId;"
        "END;"
        "$$ LANGUAGE plpgsql;";
  QUERY(ss.str().c_str(), PGRES_COMMAND_OK);

  ss.str("");
  ss << "CREATE OR REPLACE FUNCTION "
        "funcTasksOfSchedr(sId int) "
        "RETURNS TABLE("
        "  qid int,"
        "  averDurSec int,"
        "  maxDurSec int,"
        "  script text,"
        "  params text[],"
        "  prevTasks int[]) AS $$ "
        "DECLARE "
        "  t int;"
        "BEGIN"
        "  FOR qid, averDurSec, maxDurSec, script, params, prevTasks IN"
        "    SELECT tq.id, tt.averDurationSec, tt.maxDurationSec,"
        "           tt.script, tp.params, pt.prevTasks"
        "    FROM tblUTaskTemplate tt "
        "    JOIN tblTaskQueue tq ON tq.taskTempl = tt.id "
        "    JOIN tblTaskState ts ON ts.qtask = tq.id "
        "    JOIN tblTaskParam tp ON tp.qtask = tq.id "
        "    JOIN tblPrevTask pt ON pt.qtask = tq.id "
        "    WHERE tq.schedr = sId AND (ts.state BETWEEN " << (int)ZM_Base::StateType::START << " AND " << (int)ZM_Base::StateType::PAUSE << ")" 
        "  LOOP"
        "    FOREACH t IN ARRAY prevTasks"
        "      LOOP"
        "        SELECT params || (SELECT result FROM tblTaskResult WHERE qtask = t) INTO params;"
        "      END LOOP;"               
        "    RETURN NEXT;"
        "  END LOOP;"
        "END;"
        "$$ LANGUAGE plpgsql;";
  QUERY(ss.str().c_str(), PGRES_COMMAND_OK);

  ss.str("");
  ss << "CREATE OR REPLACE FUNCTION "
        "funcNewTasksForSchedr(sId int, maxTaskCnt int) "
        "RETURNS TABLE("
        "  qid int,"
        "  averDurSec int,"
        "  maxDurSec int,"
        "  script text,"
        "  params text[],"
        "  prevTasks int[]) AS $$ "
        "DECLARE "
        "  t int;"
        "BEGIN"
        "  <<mBegin>> "
        "  FOR qid, averDurSec, maxDurSec, script, params, prevTasks IN"
        "    SELECT tq.id, tt.averDurationSec, tt.maxDurationSec,"
        "           tt.script, tp.params, pt.prevTasks"
        "    FROM tblUTaskTemplate tt "
        "    JOIN tblTaskQueue tq ON tq.taskTempl = tt.id "
        "    JOIN tblTaskState ts ON ts.qtask = tq.id "
        "    JOIN tblTaskParam tp ON tp.qtask = tq.id "
        "    JOIN tblTaskTime tm ON tm.qtask = tq.id "
        "    JOIN tblPrevTask pt ON pt.qtask = tq.id "
        "    WHERE ts.state = " << int(ZM_Base::StateType::READY) << ""
        "      AND tq.schedr IS NULL AND tm.takeInWorkTime IS NULL "
        "    ORDER BY tp.priority LIMIT maxTaskCnt "
        "    FOR UPDATE OF tq SKIP LOCKED"
        "  LOOP"
        "    FOREACH t IN ARRAY prevTasks"
        "      LOOP"
        "        PERFORM * FROM tblTaskState"
        "        WHERE qtask = t AND state = " << int(ZM_Base::StateType::COMPLETED) << ";"
        "        CONTINUE mBegin WHEN NOT FOUND;"

        "        SELECT params || (SELECT result FROM tblTaskResult WHERE qtask = t) INTO params;"
        "      END LOOP;"
        
        "    UPDATE tblTaskQueue SET"
        "      schedr = sId"
        "    WHERE id = qid;"
        
        "    UPDATE tblTaskState SET"
        "      state = " << int(ZM_Base::StateType::START) << ""
        "    WHERE qtask = qid;"
        
        "    UPDATE tblTaskTime SET"
        "      createTime = current_timestamp"
        "    WHERE qtask = qid AND createTime > current_timestamp;"  

        "    UPDATE tblTaskTime SET"
        "      takeInWorkTime = current_timestamp"
        "    WHERE qtask = qid;"       
        
        "    RETURN NEXT;"
        "  END LOOP;"
        "END;"
        "$$ LANGUAGE plpgsql;";
  QUERY(ss.str().c_str(), PGRES_COMMAND_OK);

  return true;
#undef QUERY
}

// for manager
bool DbProvider::addUser(const ZM_Base::User& cng, uint64_t& outUserId){  
  lock_guard<mutex> lk(_mtx);
  stringstream ss;
  ss << "INSERT INTO tblUser (name, passwHash, description) VALUES("
        "'" << cng.name << "',"
        "MD5('" << cng.passw << "'),"
        "'" << cng.description << "') RETURNING id;";

  PGres pgr(PQexec(_pg, ss.str().c_str()));
  if (PQresultStatus(pgr.res) != PGRES_TUPLES_OK){
    errorMess(string("addUser error: ") + PQerrorMessage(_pg));
    return false;
  }
  outUserId = stoull(PQgetvalue(pgr.res, 0, 0));
  return true;
}
bool DbProvider::getUserId(const std::string& name, const std::string& passw, uint64_t& outUserId){
  lock_guard<mutex> lk(_mtx);
  stringstream ss;
  ss << "SELECT id FROM tblUser "
        "WHERE name = '" << name << "' AND " << "passwHash = MD5('" << passw << "') AND isDelete = 0;";

  PGres pgr(PQexec(_pg, ss.str().c_str()));
  if (PQresultStatus(pgr.res) != PGRES_TUPLES_OK){
    errorMess(string("getUser error: ") + PQerrorMessage(_pg));
    return false;
  }
  if (PQntuples(pgr.res) != 1){
    errorMess(string("getUser error: such user does not exist"));
    
    return false;
  }
  outUserId = stoull(PQgetvalue(pgr.res, 0, 0));
  return true;
}
bool DbProvider::getUserCng(uint64_t userId, ZM_Base::User& cng){
  lock_guard<mutex> lk(_mtx);
  stringstream ss;
  ss << "SELECT name, description FROM tblUser "
        "WHERE id = " << userId << " AND isDelete = 0;";

  PGres pgr(PQexec(_pg, ss.str().c_str()));
  if (PQresultStatus(pgr.res) != PGRES_TUPLES_OK){
    errorMess(string("getUser error: ") + PQerrorMessage(_pg));
    return false;
  }
  if (PQntuples(pgr.res) != 1){
    errorMess(string("getUser error: such user does not exist"));
    return false;
  }
  cng.name = PQgetvalue(pgr.res, 0, 0);
  cng.description = PQgetvalue(pgr.res, 0, 1);  
  return true;
}
bool DbProvider::changeUser(uint64_t userId, const ZM_Base::User& newCng){
  lock_guard<mutex> lk(_mtx);
  stringstream ss;
  ss << "UPDATE tblUser SET "
        "name = '" << newCng.name << "',"
        "passwHash = MD5('" << newCng.passw << "'),"
        "description = '" << newCng.description << "' "
        "WHERE id = " << userId << " AND isDelete = 0;";

  PGres pgr(PQexec(_pg, ss.str().c_str()));
  if (PQresultStatus(pgr.res) != PGRES_COMMAND_OK){
    errorMess(string("changeUser error: ") + PQerrorMessage(_pg));
    return false;
  }  
  return true;
}
bool DbProvider::delUser(uint64_t userId){
  lock_guard<mutex> lk(_mtx);
  stringstream ss;
  ss << "UPDATE tblUser SET "
        "isDelete = 1 "
        "WHERE id = " << userId << ";";

  PGres pgr(PQexec(_pg, ss.str().c_str()));
  if (PQresultStatus(pgr.res) != PGRES_COMMAND_OK){
    errorMess(string("delUser error: ") + PQerrorMessage(_pg));
    return false;
  }  
  return true;
}
std::vector<uint64_t> DbProvider::getAllUsers(){
  lock_guard<mutex> lk(_mtx);
  stringstream ss;
  ss << "SELECT id FROM tblUser "
        "WHERE isDelete = 0;";

  PGres pgr(PQexec(_pg, ss.str().c_str()));
  if (PQresultStatus(pgr.res) != PGRES_TUPLES_OK){
    errorMess(string("getAllUsers error: ") + PQerrorMessage(_pg));
    return std::vector<uint64_t>();
  }  
  int rows = PQntuples(pgr.res);
  std::vector<uint64_t> ret(rows);
  for (int i = 0; i < rows; ++i){
    ret[i] = stoull(PQgetvalue(pgr.res, i, 0));
  }
  return ret;
}

bool DbProvider::addSchedr(const ZM_Base::Scheduler& schedl, uint64_t& outSchId){    
  lock_guard<mutex> lk(_mtx);
  auto connPnt = ZM_Aux::split(schedl.connectPnt, ':');
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

  PGres pgr(PQexec(_pg, ss.str().c_str()));
  if (PQresultStatus(pgr.res) != PGRES_TUPLES_OK){
    errorMess(string("addSchedr error: ") + PQerrorMessage(_pg));
    return false;
  }
  outSchId = stoull(PQgetvalue(pgr.res, 0, 0));
  return true;
}
bool DbProvider::getSchedr(uint64_t sId, ZM_Base::Scheduler& cng){
  lock_guard<mutex> lk(_mtx);
  stringstream ss;
  ss << "SELECT cp.ipAddr, cp.port, s.state, s.capacityTask FROM tblScheduler s "
        "JOIN tblConnectPnt cp ON cp.id = s.connPnt "
        "WHERE s.id = " << sId << " AND s.isDelete = 0;";

  PGres pgr(PQexec(_pg, ss.str().c_str()));
  if (PQresultStatus(pgr.res) != PGRES_TUPLES_OK){
    errorMess(string("getSchedr error: ") + PQerrorMessage(_pg));
    return false;
  }
  if (PQntuples(pgr.res) != 1){
    errorMess(string("getSchedr error: such schedr does not exist"));
    return false;
  }
  cng.connectPnt = PQgetvalue(pgr.res, 0, 0) + string(":") + PQgetvalue(pgr.res, 0, 1);
  cng.state = (ZM_Base::StateType)atoi(PQgetvalue(pgr.res, 0, 2));
  cng.capacityTask = atoi(PQgetvalue(pgr.res, 0, 3));
  return true;
}
bool DbProvider::changeSchedr(uint64_t sId, const ZM_Base::Scheduler& newCng){  
  lock_guard<mutex> lk(_mtx);
  auto connPnt = ZM_Aux::split(newCng.connectPnt, ':');
  if (connPnt.size() != 2){
    errorMess("changeSchedr error: connectPnt not correct");
    return false;
  }
  stringstream ss;
  ss << "UPDATE tblScheduler SET "
        "capacityTask = '" << newCng.capacityTask << "' "
        "WHERE id = " << sId << " AND isDelete = 0; "

        "UPDATE tblConnectPnt SET "
        "ipAddr = '" << connPnt[0] << "',"
        "port = '" << connPnt[1] << "' "
        "WHERE id = (SELECT connPnt FROM tblScheduler WHERE id = " << sId << " AND isDelete = 0);";

  PGres pgr(PQexec(_pg, ss.str().c_str()));
  if (PQresultStatus(pgr.res) != PGRES_COMMAND_OK){
    errorMess(string("changeSchedr error: ") + PQerrorMessage(_pg));
    return false;
  }  
  return true;
}
bool DbProvider::delSchedr(uint64_t sId){  
  lock_guard<mutex> lk(_mtx);
  stringstream ss;
  ss << "UPDATE tblScheduler SET "
        "isDelete = 1 "
        "WHERE id = " << sId << ";";

  PGres pgr(PQexec(_pg, ss.str().c_str()));
  if (PQresultStatus(pgr.res) != PGRES_COMMAND_OK){
    errorMess(string("delSchedr error: ") + PQerrorMessage(_pg));
    return false;
  }  
  return true;
}
bool DbProvider::schedrState(uint64_t sId, ZM_Base::StateType& state){
  lock_guard<mutex> lk(_mtx);
  stringstream ss;
  ss << "SELECT state FROM tblScheduler "
        "WHERE id = " << sId << " AND isDelete = 0;";

  PGres pgr(PQexec(_pg, ss.str().c_str()));
  if ((PQresultStatus(pgr.res) != PGRES_TUPLES_OK) || (PQntuples(pgr.res) != 1)){
    errorMess(string("schedrState error: ") + PQerrorMessage(_pg));
    return false;
  }
  state = (ZM_Base::StateType)atoi(PQgetvalue(pgr.res, 0, 0));
  return true;
}
std::vector<uint64_t> DbProvider::getAllSchedrs(ZM_Base::StateType state){  
  lock_guard<mutex> lk(_mtx);
  stringstream ss;
  ss << "SELECT id FROM tblScheduler "
        "WHERE (state = " << (int)state << " OR " << (int)state << " = -1) AND isDelete = 0;";

  PGres pgr(PQexec(_pg, ss.str().c_str()));
  if (PQresultStatus(pgr.res) != PGRES_TUPLES_OK){
    errorMess(string("getAllSchedrs error: ") + PQerrorMessage(_pg));
    return std::vector<uint64_t>();
  }  
  int rows = PQntuples(pgr.res);
  std::vector<uint64_t> ret(rows);
  for (int i = 0; i < rows; ++i){
    ret[i] = stoull(PQgetvalue(pgr.res, i, 0));
  }
  return ret;
}

bool DbProvider::addWorker(const ZM_Base::Worker& worker, uint64_t& outWkrId){
  lock_guard<mutex> lk(_mtx);
  auto connPnt = ZM_Aux::split(worker.connectPnt, ':');
  if (connPnt.size() != 2){
    errorMess("addWorker error: connectPnt not correct");
    return false;
  }
  stringstream ss;
  ss << "WITH ncp AS (INSERT INTO tblConnectPnt (ipAddr, port) VALUES("
        " '" << connPnt[0] << "',"
        " '" << connPnt[1] << "') RETURNING id)"
        "INSERT INTO tblWorker (connPnt, schedr, state, capacityTask) VALUES("
        "(SELECT id FROM ncp),"
        "'" << (int)worker.sId << "',"
        "'" << (int)worker.state << "',"
        "'" << worker.capacityTask << "') RETURNING id;";

  PGres pgr(PQexec(_pg, ss.str().c_str()));
  if (PQresultStatus(pgr.res) != PGRES_TUPLES_OK){
      errorMess(string("addWorker error: ") + PQerrorMessage(_pg));
      return false;
  }
  outWkrId = stoull(PQgetvalue(pgr.res, 0, 0));
  return true;
}
bool DbProvider::getWorker(uint64_t wId, ZM_Base::Worker& cng){
  lock_guard<mutex> lk(_mtx);
  stringstream ss;
  ss << "SELECT cp.ipAddr, cp.port, w.schedr, w.state, w.capacityTask FROM tblWorker w "
        "JOIN tblConnectPnt cp ON cp.id = w.connPnt "
        "WHERE w.id = " << wId << " AND w.isDelete = 0;";

  PGres pgr(PQexec(_pg, ss.str().c_str()));
  if (PQresultStatus(pgr.res) != PGRES_TUPLES_OK){
    errorMess(string("getWorker error: ") + PQerrorMessage(_pg));
    return false;
  }
  if (PQntuples(pgr.res) != 1){
    errorMess(string("getWorker error: such worker does not exist"));
    return false;
  }
  cng.connectPnt = PQgetvalue(pgr.res, 0, 0) + string(":") + PQgetvalue(pgr.res, 0, 1);
  cng.sId = stoull(PQgetvalue(pgr.res, 0, 2));
  cng.state = (ZM_Base::StateType)atoi(PQgetvalue(pgr.res, 0, 3));
  cng.capacityTask = atoi(PQgetvalue(pgr.res, 0, 4));
  return true;
}
bool DbProvider::changeWorker(uint64_t wId, const ZM_Base::Worker& newCng){
  lock_guard<mutex> lk(_mtx);
  auto connPnt = ZM_Aux::split(newCng.connectPnt, ':');
  if (connPnt.size() != 2){
    errorMess("changeWorker error: connectPnt not correct");
    return false;
  }
  stringstream ss;
  ss << "UPDATE tblWorker SET "
        "schedr = '" << (int)newCng.sId << "',"
        "capacityTask = '" << newCng.capacityTask << "' "
        "WHERE id = " << wId << " AND isDelete = 0; "

        "UPDATE tblConnectPnt SET "
        "ipAddr = '" << connPnt[0] << "',"
        "port = '" << connPnt[1] << "' "
        "WHERE id = (SELECT connPnt FROM tblWorker WHERE id = " << wId << " AND isDelete = 0);";

  PGres pgr(PQexec(_pg, ss.str().c_str()));
  if (PQresultStatus(pgr.res) != PGRES_COMMAND_OK){
    errorMess(string("changeWorker error: ") + PQerrorMessage(_pg));
    return false;
  }  
  return true;
}
bool DbProvider::delWorker(uint64_t wId){
  lock_guard<mutex> lk(_mtx);
  stringstream ss;
  ss << "UPDATE tblWorker SET "
        "isDelete = 1 "
        "WHERE id = " << wId << ";";

  PGres pgr(PQexec(_pg, ss.str().c_str()));
  if (PQresultStatus(pgr.res) != PGRES_COMMAND_OK){
    errorMess(string("delWorker error: ") + PQerrorMessage(_pg));
    return false;
  }  
  return true;
}
bool DbProvider::workerState(const std::vector<uint64_t>& wId, std::vector<ZM_Base::StateType>& state){
  lock_guard<mutex> lk(_mtx);
  string swId;
  swId = accumulate(wId.begin(), wId.end(), swId,
                [](string& s, uint64_t v){
                  return s.empty() ? to_string(v) : s + "," + to_string(v);
                }); 
  stringstream ss;
  ss << "SELECT state FROM tblWorker "
        "WHERE id IN (" << swId << ")  AND isDelete = 0 ORDER BY id;";

  PGres pgr(PQexec(_pg, ss.str().c_str()));
  if (PQresultStatus(pgr.res) != PGRES_TUPLES_OK){
    errorMess(string("workerState error: ") + PQerrorMessage(_pg));
    return false;
  }
  size_t wsz = wId.size();
  if (PQntuples(pgr.res) != wsz){
    errorMess("workerState error: PQntuples(pgr.res) != wsz");
    return false;
  }
  state.resize(wsz);
  for (size_t i = 0; i < wsz; ++i){
    state[i] = (ZM_Base::StateType)atoi(PQgetvalue(pgr.res, (int)i, 0));
  }
  return true;
}
std::vector<uint64_t> DbProvider::getAllWorkers(uint64_t sId, ZM_Base::StateType state){
  lock_guard<mutex> lk(_mtx);
  stringstream ss;
  ss << "SELECT id FROM tblWorker "
        "WHERE (state = " << (int)state << " OR " << (int)state << " = -1) "
        "AND schedr = " << sId << " "
        "AND isDelete = 0;";

  PGres pgr(PQexec(_pg, ss.str().c_str()));
  if (PQresultStatus(pgr.res) != PGRES_TUPLES_OK){
    errorMess(string("getAllWorkers error: ") + PQerrorMessage(_pg));
    return std::vector<uint64_t>();
  }  
  int rows = PQntuples(pgr.res);
  std::vector<uint64_t> ret(rows);
  for (int i = 0; i < rows; ++i){
    ret[i] = stoull(PQgetvalue(pgr.res, i, 0));
  }
  return ret;
}

bool DbProvider::addPipeline(const ZM_Base::UPipeline& ppl, uint64_t& outPPLId){
  lock_guard<mutex> lk(_mtx);
  stringstream ss;
  ss << "INSERT INTO tblUPipeline (usr, name, description) VALUES("
        "'" << ppl.uId << "',"
        "'" << ppl.name << "',"
        "'" << ppl.description << "') RETURNING id;";

  PGres pgr(PQexec(_pg, ss.str().c_str()));
  if (PQresultStatus(pgr.res) != PGRES_TUPLES_OK){
    errorMess(string("addPipeline error: ") + PQerrorMessage(_pg));
    return false;
  }
  outPPLId = stoull(PQgetvalue(pgr.res, 0, 0));
  return true;
}
bool DbProvider::getPipeline(uint64_t pplId, ZM_Base::UPipeline& cng){
  lock_guard<mutex> lk(_mtx);
  stringstream ss;
  ss << "SELECT usr, name, description FROM tblUPipeline "
        "WHERE id = " << pplId << " AND isDelete = 0;";

  PGres pgr(PQexec(_pg, ss.str().c_str()));
  if (PQresultStatus(pgr.res) != PGRES_TUPLES_OK){
    errorMess(string("getPipeline error: ") + PQerrorMessage(_pg));
    return false;
  }
  if (PQntuples(pgr.res) != 1){
    errorMess(string("getPipeline error: such pipeline does not exist"));
    return false;
  }
  cng.uId = stoull(PQgetvalue(pgr.res, 0, 0));
  cng.name = PQgetvalue(pgr.res, 0, 1);
  cng.description = PQgetvalue(pgr.res, 0, 2);
  return true;
}
bool DbProvider::changePipeline(uint64_t pplId, const ZM_Base::UPipeline& newCng){
  lock_guard<mutex> lk(_mtx);
  stringstream ss;
  ss << "UPDATE tblUPipeline SET "
        "usr = '" << newCng.uId << "',"
        "name = '" << newCng.name << "',"
        "description = '" << newCng.description << "' "
        "WHERE id = " << pplId << " AND isDelete = 0;";

  PGres pgr(PQexec(_pg, ss.str().c_str()));
  if (PQresultStatus(pgr.res) != PGRES_COMMAND_OK){
    errorMess(string("changePipeline error: ") + PQerrorMessage(_pg));
    return false;
  }  
  return true;
}
bool DbProvider::delPipeline(uint64_t pplId){
  lock_guard<mutex> lk(_mtx);
  stringstream ss;
  ss << "UPDATE tblUPipeline SET "  
        "isDelete = 1 "
        "WHERE id = " << pplId << ";";
    
  PGres pgr(PQexec(_pg, ss.str().c_str()));
  if (PQresultStatus(pgr.res) != PGRES_COMMAND_OK){
    errorMess(string("delPipeline error: ") + PQerrorMessage(_pg));
    return false;
  }  
  return true;
}
std::vector<uint64_t> DbProvider::getAllPipelines(uint64_t userId){
  lock_guard<mutex> lk(_mtx);
  stringstream ss;
  ss << "SELECT id FROM tblUPipeline "
        "WHERE usr = " << userId << " AND isDelete = 0;";

  PGres pgr(PQexec(_pg, ss.str().c_str()));
  if (PQresultStatus(pgr.res) != PGRES_TUPLES_OK){
    errorMess(string("getAllPipelines error: ") + PQerrorMessage(_pg));
    return std::vector<uint64_t>();
  }  
  int rows = PQntuples(pgr.res);
  std::vector<uint64_t> ret(rows);
  for (int i = 0; i < rows; ++i){
    ret[i] = stoull(PQgetvalue(pgr.res, i, 0));
  }
  return ret;
}

bool DbProvider::addGroup(const ZM_Base::UGroup& cng, uint64_t& outGId){
  lock_guard<mutex> lk(_mtx);
  stringstream ss;
  ss << "INSERT INTO tblUTaskGroup (pipeline, name, description) VALUES("
        "'" << cng.pplId << "',"
        "'" << cng.name << "',"
        "'" << cng.description << "') RETURNING id;";

  PGres pgr(PQexec(_pg, ss.str().c_str()));
  if (PQresultStatus(pgr.res) != PGRES_TUPLES_OK){
    errorMess(string("addGroup error: ") + PQerrorMessage(_pg));
    return false;
  }
  outGId = stoull(PQgetvalue(pgr.res, 0, 0));
  return true;
}
bool DbProvider::getGroup(uint64_t gId, ZM_Base::UGroup& cng){
  lock_guard<mutex> lk(_mtx);
  stringstream ss;
  ss << "SELECT pipeline, name, description FROM tblUTaskGroup "
        "WHERE id = " << gId << " AND isDelete = 0;";

  PGres pgr(PQexec(_pg, ss.str().c_str()));
  if (PQresultStatus(pgr.res) != PGRES_TUPLES_OK){
    errorMess(string("getGroup error: ") + PQerrorMessage(_pg));
    return false;
  }
  if (PQntuples(pgr.res) != 1){
    errorMess(string("getGroup error: such group does not exist"));
    return false;
  }
  cng.pplId = stoull(PQgetvalue(pgr.res, 0, 0));
  cng.name = PQgetvalue(pgr.res, 0, 1);
  cng.description = PQgetvalue(pgr.res, 0, 2);
  return true;
}
bool DbProvider::changeGroup(uint64_t gId, const ZM_Base::UGroup& newCng){
  lock_guard<mutex> lk(_mtx);
  stringstream ss;
  ss << "UPDATE tblUTaskGroup SET "
        "pipeline = '" << newCng.pplId << "',"
        "name = '" << newCng.name << "',"
        "description = '" << newCng.description << "' "
        "WHERE id = " << gId << " AND isDelete = 0;";

  PGres pgr(PQexec(_pg, ss.str().c_str()));
  if (PQresultStatus(pgr.res) != PGRES_COMMAND_OK){
    errorMess(string("changeGroup error: ") + PQerrorMessage(_pg));
    return false;
  }  
  return true;
}
bool DbProvider::delGroup(uint64_t gId){
  lock_guard<mutex> lk(_mtx);
  stringstream ss;
  ss << "UPDATE tblUTaskGroup SET "  
        "isDelete = 1 "
        "WHERE id = " << gId << ";";

  PGres pgr(PQexec(_pg, ss.str().c_str()));
  if (PQresultStatus(pgr.res) != PGRES_COMMAND_OK){
    errorMess(string("delGroup error: ") + PQerrorMessage(_pg));
    return false;
  }  
  return true;
}
std::vector<uint64_t> DbProvider::getAllGroups(uint64_t pplId){
  lock_guard<mutex> lk(_mtx);
  stringstream ss;
  ss << "SELECT id FROM tblUTaskGroup "
        "WHERE pipeline = " << pplId << " AND isDelete = 0;";

  PGres pgr(PQexec(_pg, ss.str().c_str()));
  if (PQresultStatus(pgr.res) != PGRES_TUPLES_OK){
    errorMess(string("getAllGroups error: ") + PQerrorMessage(_pg));
    return std::vector<uint64_t>();
  }  
  int rows = PQntuples(pgr.res);
  std::vector<uint64_t> ret(rows);
  for (int i = 0; i < rows; ++i){
    ret[i] = stoull(PQgetvalue(pgr.res, i, 0));
  }
  return ret;
}

bool DbProvider::addTaskTemplate(const ZM_Base::UTaskTemplate& cng, uint64_t& outTId){
  lock_guard<mutex> lk(_mtx);
  stringstream ss;
  ss << "INSERT INTO tblUTaskTemplate (usr, name, description, script, averDurationSec, maxDurationSec) VALUES("
        "'" << cng.uId << "',"
        "'" << cng.name << "',"
        "'" << cng.description << "',"
        "'" << cng.script << "',"
        "'" << cng.averDurationSec << "',"
        "'" << cng.maxDurationSec << "') RETURNING id;";

  PGres pgr(PQexec(_pg, ss.str().c_str()));
  if (PQresultStatus(pgr.res) != PGRES_TUPLES_OK){
    errorMess(string("addTaskTemplate error: ") + PQerrorMessage(_pg));
    return false;
  }
  outTId = stoull(PQgetvalue(pgr.res, 0, 0));
  return true;
}
bool DbProvider::getTaskTemplate(uint64_t tId, ZM_Base::UTaskTemplate& outTCng){
  lock_guard<mutex> lk(_mtx);
  stringstream ss;
  ss << "SELECT tt.usr, tt.name, tt.description, tt.script, tt.averDurationSec, tt.maxDurationSec "
        "FROM tblUTaskTemplate tt "
        "WHERE tt.id = " << tId << " AND tt.isDelete = 0;";

  PGres pgr(PQexec(_pg, ss.str().c_str()));
  if (PQresultStatus(pgr.res) != PGRES_TUPLES_OK){
    errorMess(string("getTaskTemplate error: ") + PQerrorMessage(_pg));
    return false;
  }
  if (PQntuples(pgr.res) != 1){
    errorMess(string("getTaskTemplate error: such taskTemplate does not exist"));
    return false;
  }
  outTCng.uId = stoull(PQgetvalue(pgr.res, 0, 0));
  outTCng.name = PQgetvalue(pgr.res, 0, 1);
  outTCng.description = PQgetvalue(pgr.res, 0, 2);
  outTCng.script = PQgetvalue(pgr.res, 0, 3);  
  outTCng.averDurationSec = atoi(PQgetvalue(pgr.res, 0, 4));
  outTCng.maxDurationSec = atoi(PQgetvalue(pgr.res, 0, 5));
  return true;
};
bool DbProvider::changeTaskTemplate(uint64_t tId, const ZM_Base::UTaskTemplate& newTCng){
  lock_guard<mutex> lk(_mtx);
  stringstream ss;
  ss << "UPDATE tblUTaskTemplate SET "
        "usr = '" << newTCng.uId << "',"
        "name = '" << newTCng.name << "',"
        "description = '" << newTCng.description << "', "
        "script = '" << newTCng.script << "',"
        "averDurationSec = '" << newTCng.averDurationSec << "',"
        "maxDurationSec = '" << newTCng.maxDurationSec << "' "
        "WHERE id = " << tId << ";";
          
  PGres pgr(PQexec(_pg, ss.str().c_str()));
  if (PQresultStatus(pgr.res) != PGRES_COMMAND_OK){
    errorMess(string("changeTaskTemplate error: ") + PQerrorMessage(_pg));
    return false;
  } 
  return true;
}
bool DbProvider::delTaskTemplate(uint64_t tId){
  lock_guard<mutex> lk(_mtx);
  stringstream ss;
  ss << "UPDATE tblUTaskTemplate SET "
        "isDelete = 1 "
        "WHERE id = " << tId << ";";

  PGres pgr(PQexec(_pg, ss.str().c_str()));
  if (PQresultStatus(pgr.res) != PGRES_COMMAND_OK){
    errorMess(string("delTaskTemplate error: ") + PQerrorMessage(_pg));
    return false;
  }  
  return true;
}
std::vector<uint64_t> DbProvider::getAllTaskTemplates(uint64_t usr){
  lock_guard<mutex> lk(_mtx);
  stringstream ss;
  ss << "SELECT id FROM tblUTaskTemplate "
        "WHERE usr = " << usr << " AND isDelete = 0;";

  PGres pgr(PQexec(_pg, ss.str().c_str()));
  if (PQresultStatus(pgr.res) != PGRES_TUPLES_OK){
    errorMess(string("getAllTaskTemplates error: ") + PQerrorMessage(_pg));
    return std::vector<uint64_t>();
  }  
  int rows = PQntuples(pgr.res);
  std::vector<uint64_t> ret(rows);
  for (int i = 0; i < rows; ++i){
    ret[i] = stoull(PQgetvalue(pgr.res, i, 0));
  }
  return ret;
}

bool DbProvider::addTaskPipeline(const ZM_Base::UTaskPipeline& cng, uint64_t& outTId){
  lock_guard<mutex> lk(_mtx);
  
  stringstream ss;
  ss << "INSERT INTO tblUPipelineTask (pipeline, taskTempl, taskGroup, priority) VALUES("
        "'" << cng.pplId << "',"
        "'" << cng.ttId << "',"
        "NULLIF(" << cng.gId << ", 0),"
        "'" << cng.priority << "') RETURNING id;";

  PGres pgr(PQexec(_pg, ss.str().c_str()));
  if (PQresultStatus(pgr.res) != PGRES_TUPLES_OK){
    errorMess(string("addTaskPipeline error: ") + PQerrorMessage(_pg));
    return false;
  }
  outTId = stoull(PQgetvalue(pgr.res, 0, 0));
  return true;
}
bool DbProvider::getTaskPipeline(uint64_t tId, ZM_Base::UTaskPipeline& outTCng){
  lock_guard<mutex> lk(_mtx);
  stringstream ss;
  ss << "SELECT pipeline, COALESCE(taskGroup, 0), taskTempl, priority "
        "FROM tblUPipelineTask "
        "WHERE id = " << tId << " AND isDelete = 0;";

  PGres pgr(PQexec(_pg, ss.str().c_str()));
  if (PQresultStatus(pgr.res) != PGRES_TUPLES_OK){
    errorMess(string("getTaskPipeline error: ") + PQerrorMessage(_pg));
    return false;
  }
  if (PQntuples(pgr.res) != 1){
    errorMess(string("getTaskPipeline error: such task does not exist"));
    return false;
  }
  outTCng.pplId = stoull(PQgetvalue(pgr.res, 0, 0));
  outTCng.gId = stoull(PQgetvalue(pgr.res, 0, 1));
  outTCng.ttId = stoull(PQgetvalue(pgr.res, 0, 2));
  outTCng.priority = atoi(PQgetvalue(pgr.res, 0, 3));  
  return true;
}
bool DbProvider::changeTaskPipeline(uint64_t tId, const ZM_Base::UTaskPipeline& newCng){
  lock_guard<mutex> lk(_mtx);
 
  stringstream ss;
  ss << "UPDATE tblUPipelineTask SET "
        "pipeline = '" << newCng.pplId << "',"
        "taskTempl = '" << newCng.ttId << "',"
        "taskGroup = NULLIF(" << newCng.gId << ", 0),"
        "priority = '" << newCng.priority << "' "        
        "WHERE id = " << tId << " AND isDelete = 0;";
          
  PGres pgr(PQexec(_pg, ss.str().c_str()));
  if (PQresultStatus(pgr.res) != PGRES_COMMAND_OK){
    errorMess(string("changeTaskPipeline error: ") + PQerrorMessage(_pg));
    return false;
  }  
  return true;
}
bool DbProvider::delTaskPipeline(uint64_t tId){
  lock_guard<mutex> lk(_mtx);
  stringstream ss;
  ss << "UPDATE tblUPipelineTask SET "
        "isDelete = 1 "
        "WHERE id = " << tId << ";";

  PGres pgr(PQexec(_pg, ss.str().c_str()));
  if (PQresultStatus(pgr.res) != PGRES_COMMAND_OK){
    errorMess(string("delTaskPipeline error: ") + PQerrorMessage(_pg));
    return false;
  }  
  return true;
}
std::vector<uint64_t> DbProvider::getAllTasksPipeline(uint64_t pplId){  
  lock_guard<mutex> lk(_mtx);
  stringstream ss;
  ss << "SELECT ut.id FROM tblUPipelineTask ut "
        "WHERE ut.isDelete = 0 AND ut.pipeline = " << pplId << ";";

  PGres pgr(PQexec(_pg, ss.str().c_str()));
  if (PQresultStatus(pgr.res) != PGRES_TUPLES_OK){
    errorMess(string("getAllTasksPipeline error: ") + PQerrorMessage(_pg));
    return std::vector<uint64_t>();
  }  
  int rows = PQntuples(pgr.res);
  std::vector<uint64_t> ret(rows);
  for (int i = 0; i < rows; ++i){
    ret[i] = stoull(PQgetvalue(pgr.res, i, 0));
  }
  return ret;
}

bool DbProvider::startTask(uint64_t ptId, const std::string& inparams, const std::string& prevTasks, uint64_t& tId){
  lock_guard<mutex> lk(_mtx);  

  string params = "['" + inparams + "']";
  ZM_Aux::replace(params, ",", "','");

  stringstream ss;
  ss << "SELECT * FROM funcStartTask(" << ptId << ", ARRAY" << params << "::TEXT[], ARRAY[" << prevTasks << "]::INT[]);";

  PGres pgr(PQexec(_pg, ss.str().c_str()));
  if (PQresultStatus(pgr.res) != PGRES_TUPLES_OK){
    errorMess(string("startTask error: ") + PQerrorMessage(_pg));
    return false;
  }
  tId = stoull(PQgetvalue(pgr.res, 0, 0));
  if (tId == 0){
    errorMess("startTask error: such task does not exist");
    return false;
  }  
  return true;
}
bool DbProvider::cancelTask(uint64_t tId){
  lock_guard<mutex> lk(_mtx);
  stringstream ss;
  ss << "UPDATE tblTaskState ts SET "
        "state = " << int(ZM_Base::StateType::CANCEL) << " "
        "FROM tblTaskQueue tq "
        "WHERE tq.id = " << tId << " AND "
        "      ts.qtask = tq.id AND "
        "      ts.state = " << int(ZM_Base::StateType::READY) << " "
        "RETURNING ts.qtask;";
        
  PGres pgr(PQexec(_pg, ss.str().c_str()));
  if (PQresultStatus(pgr.res) != PGRES_TUPLES_OK){
    errorMess(string("cancelTask error: ") + PQerrorMessage(_pg));
    return false;
  }  
  if (PQntuples(pgr.res) != 1){
    errorMess(string("cancelTask error: task already take in work") + PQerrorMessage(_pg));
    return false;
  }
  return true;
}
bool DbProvider::taskState(const std::vector<uint64_t>& tId, std::vector<ZM_DB::TaskState>& outState){
  lock_guard<mutex> lk(_mtx);
  string stId;
  stId = accumulate(tId.begin(), tId.end(), stId,
                [](string& s, uint64_t v){
                  return s.empty() ? to_string(v) : s + "," + to_string(v);
                }); 
  stringstream ss;
  ss << "SELECT ts.state, ts.progress "
        "FROM tblTaskState ts "
        "WHERE ts.qtask IN (" << stId << ") ORDER BY ts.qtask;";

  PGres pgr(PQexec(_pg, ss.str().c_str()));
  if (PQresultStatus(pgr.res) != PGRES_TUPLES_OK){
    errorMess(string("taskState error: ") + PQerrorMessage(_pg));
    return false;
  }
  size_t tsz = tId.size();
  if (PQntuples(pgr.res) != tsz){      
    errorMess("taskState error: PQntuples(pgr.res) != tsz");
    return false;
  }
  outState.resize(tsz);
  for (size_t i = 0; i < tsz; ++i){
    outState[i].state = (ZM_Base::StateType)atoi(PQgetvalue(pgr.res, (int)i, 0));
    outState[i].progress = atoi(PQgetvalue(pgr.res, (int)i, 1));
  }
  return true;
}
bool DbProvider::taskResult(uint64_t tId, std::string& out){
  lock_guard<mutex> lk(_mtx);
  stringstream ss;
  ss << "SELECT result FROM tblTaskResult "
        "WHERE qtask = " << tId << ";";

  PGres pgr(PQexec(_pg, ss.str().c_str()));
  if (PQresultStatus(pgr.res) != PGRES_TUPLES_OK){      
    errorMess(string("taskResult error: ") + PQerrorMessage(_pg));
    return false;
  } 
  if (PQntuples(pgr.res) != 1){
    errorMess("taskResult error: task delete OR not taken to work");
    return false;
  } 
  out = PQgetvalue(pgr.res, 0, 0);
  return true;
}
bool DbProvider::taskTime(uint64_t tId, ZM_DB::TaskTime& out){
  lock_guard<mutex> lk(_mtx);
  stringstream ss;
  ss << "SELECT createTime, takeInWorkTime, startTime, stopTime "
        "FROM tblTaskTime "
        "WHERE qtask = " << tId << ";";

  PGres pgr(PQexec(_pg, ss.str().c_str()));
  if (PQresultStatus(pgr.res) != PGRES_TUPLES_OK){      
    errorMess(string("taskTime error: ") + PQerrorMessage(_pg));
    return false;
  } 
  if (PQntuples(pgr.res) != 1){
    errorMess("taskTime error: task delete OR not taken to work");
    return false;
  } 
  out.createTime = PQgetvalue(pgr.res, 0, 0);
  out.takeInWorkTime = PQgetvalue(pgr.res, 0, 1);
  out.startTime = PQgetvalue(pgr.res, 0, 2);
  out.stopTime = PQgetvalue(pgr.res, 0, 3);
  return true;
}

bool DbProvider::getWorkerByTask(uint64_t tId, ZM_Base::Worker& wcng){
  lock_guard<mutex> lk(_mtx);
  stringstream ss;
  ss << "SELECT wkr.id, wkr.connPnt "
        "FROM tblTaskQueue tq "
        "JOIN tblWorker wkr ON wkr.id = tq.worker "
        "WHERE tq.id = " << tId << ";";

  PGres pgr(PQexec(_pg, ss.str().c_str()));
  if (PQresultStatus(pgr.res) != PGRES_TUPLES_OK){
    errorMess(string("getWorkerByTask error: ") + PQerrorMessage(_pg));
    return false;
  }
  if (PQntuples(pgr.res) != 1){
    errorMess(string("getWorkerByTask error: task delete OR not taken to work"));
    return false;
  }
  wcng.id = stoull(PQgetvalue(pgr.res, 0, 0));
  wcng.connectPnt = PQgetvalue(pgr.res, 0, 1);

  return true;
}
bool DbProvider::setChangeTaskStateCBack(uint64_t tId, changeTaskStateCBack cback){
  if (!cback){
    return false;
  }
  {
    lock_guard<mutex> lk(_mtxNotifyTask);  
    _notifyTaskStateCBack[tId] = {ZM_Base::StateType::UNDEFINED, cback};
    if (_notifyTaskStateCBack.size() == 1)
      _cvNotifyTask.notify_one();
  }  
  if (!_thrEndTask.joinable()){
    _thrEndTask = thread([this](){
      while (!_fClose){
        if (_notifyTaskStateCBack.empty()){
          std::unique_lock<std::mutex> lk(_mtxNotifyTask);
          _cvNotifyTask.wait(lk); 
        }
        std::map<uint64_t, pair<ZM_Base::StateType, changeTaskStateCBack>> notifyTask;
        {
          lock_guard<mutex> lk(_mtxNotifyTask); 
          notifyTask = _notifyTaskStateCBack;
        } 
        string stId;
        stId = accumulate(notifyTask.begin(), notifyTask.end(), stId,
                  [](string& s, pair<uint64_t, pair<ZM_Base::StateType, changeTaskStateCBack>> v){
                    return s.empty() ? to_string(v.first) : s + "," + to_string(v.first);
                  });       
        stringstream ss;
        ss << "SELECT qt.id, ts.state "
              "FROM tblTaskState ts "
              "JOIN tblTaskQueue qt ON qt.id = ts.qtask "
              "WHERE qt.id IN (" << stId << ");"; 
        
        vector<pair<uint64_t, ZM_Base::StateType>> notifyRes;
        { 
          lock_guard<mutex> lk(_mtx);
          PGres pgr(PQexec(_pg, ss.str().c_str()));
          if (PQresultStatus(pgr.res) == PGRES_TUPLES_OK){
            size_t tsz = PQntuples(pgr.res);
            for (size_t i = 0; i < tsz; ++i){
              uint64_t tId = stoull(PQgetvalue(pgr.res, (int)i, 0));
              ZM_Base::StateType state = (ZM_Base::StateType)atoi(PQgetvalue(pgr.res, (int)i, 1));
              if (state != notifyTask[tId].first){
                notifyRes.push_back(make_pair(tId, state));
              }
            }
          }else{
            errorMess(string("endTaskCBack error: ") + PQerrorMessage(_pg));
          } 
        }
        if (!notifyRes.empty()){
          for (auto& t : notifyRes){
            uint64_t tId = t.first;
            ZM_Base::StateType prevState = notifyTask[tId].first,
                               newState = t.second;
            notifyTask[tId].second(tId, prevState, newState);
          }
          { 
            lock_guard<mutex> lk(_mtxNotifyTask);  
            for (auto& t : notifyRes){
              uint64_t tId = t.first;
              ZM_Base::StateType newState = t.second;
              if ((newState == ZM_Base::StateType::COMPLETED) || (newState == ZM_Base::StateType::ERROR) || (newState == ZM_Base::StateType::CANCEL)){
                _notifyTaskStateCBack.erase(tId);
              }else{
                _notifyTaskStateCBack[tId].first = newState;
              }
            }    
          }
        }
      }      
    });
  }
  return true;
}

vector<ZM_DB::MessError> DbProvider::getInternErrors(uint64_t sId, uint64_t wId, uint32_t mCnt){
  lock_guard<mutex> lk(_mtx);
  if (mCnt == 0){
    mCnt = INT32_MAX;
  }
  stringstream ss;
  ss << "SELECT schedr, worker, message, createTime "
        "FROM tblInternError "
        "WHERE (schedr = " << sId << " OR " << sId << " = 0)" << " AND "
        "      (worker = " << wId << " OR " << wId << " = 0) ORDER BY createTime LIMIT " << mCnt << ";";

  PGres pgr(PQexec(_pg, ss.str().c_str()));
  if (PQresultStatus(pgr.res) != PGRES_TUPLES_OK){
    errorMess(string("getInternErrors error: ") + PQerrorMessage(_pg));
    return vector<ZM_DB::MessError>();
  }  
  int rows = PQntuples(pgr.res);
  std::vector<ZM_DB::MessError> ret(rows);
  for (int i = 0; i < rows; ++i){
    ret[i].schedrId = stoull(PQgetvalue(pgr.res, i, 0));
    ret[i].workerId = stoull(PQgetvalue(pgr.res, i, 1));
    ret[i].message = PQgetvalue(pgr.res, i, 2);
    ret[i].createTime = PQgetvalue(pgr.res, i, 3);
  }
  return ret;
}

// for zmSchedr
bool DbProvider::getSchedr(const std::string& connPnt, ZM_Base::Scheduler& outCng){
  lock_guard<mutex> lk(_mtx);
  auto cp = ZM_Aux::split(connPnt, ':');
  if (cp.size() != 2){
    errorMess(string("getSchedr error: connPnt not correct"));
    return false;
  }
  stringstream ss;
  ss << "SELECT s.id, s.state, s.capacityTask, s.activeTask, s.internalData FROM tblScheduler s "
        "JOIN tblConnectPnt cp ON cp.id = s.connPnt "
        "WHERE cp.ipAddr = '" << cp[0] << "' AND cp.port = '" << cp[1] << "' AND s.isDelete = 0;";

  PGres pgr(PQexec(_pg, ss.str().c_str()));
  if (PQresultStatus(pgr.res) != PGRES_TUPLES_OK){
    errorMess(string("getSchedr error: ") + PQerrorMessage(_pg));
    return false;
  }
  if (PQntuples(pgr.res) == 0){
    errorMess("getSchedr error: such schedr does not exist");
    return false;
  }
  outCng.id = stoull(PQgetvalue(pgr.res, 0, 0));
  outCng.connectPnt = connPnt;
  outCng.state = (ZM_Base::StateType)atoi(PQgetvalue(pgr.res, 0, 1));
  outCng.capacityTask = atoi(PQgetvalue(pgr.res, 0, 2));
  outCng.activeTask = atoi(PQgetvalue(pgr.res, 0, 3));
  outCng.internalData = PQgetvalue(pgr.res, 0, 4);
  return true;
}
bool DbProvider::getTasksOfSchedr(uint64_t sId, std::vector<ZM_Base::Task>& out){
  lock_guard<mutex> lk(_mtx);
  stringstream ss;
  ss << "SELECT * FROM funcTasksOfSchedr(" << sId << ");";
     
  PGres pgr(PQexec(_pg, ss.str().c_str()));
  if (PQresultStatus(pgr.res) != PGRES_TUPLES_OK){
    errorMess(string("getTasksOfSchedr error: ") + PQerrorMessage(_pg));
    return false;
  }
  int tsz = PQntuples(pgr.res);
  for (int i = 0; i < tsz; ++i){
    string params = PQgetvalue(pgr.res, i, 4);
    ZM_Aux::replace(params, "\"", "");
    params = params.substr(1, params.size() - 2); // remove { and }
    out.push_back(ZM_Base::Task{stoull(PQgetvalue(pgr.res, i, 0)),
                                atoi(PQgetvalue(pgr.res, i, 1)),
                                atoi(PQgetvalue(pgr.res, i, 2)),
                                PQgetvalue(pgr.res, i, 3),
                                params});
  }
  return true;
}
bool DbProvider::getWorkersOfSchedr(uint64_t sId, std::vector<ZM_Base::Worker>& out){
  lock_guard<mutex> lk(_mtx);
  stringstream ss;
  ss << "SELECT w.id, w.state, w.capacityTask, w.activeTask, cp.ipAddr, cp.port "
        "FROM tblWorker w "
        "JOIN tblConnectPnt cp ON cp.id = w.connPnt "
        "WHERE w.schedr = " << sId << " AND w.isDelete = 0;";

  PGres pgr(PQexec(_pg, ss.str().c_str()));
  if (PQresultStatus(pgr.res) != PGRES_TUPLES_OK){
    errorMess(string("getWorkersOfSchedr error: ") + PQerrorMessage(_pg));
    return false;
  }
  int wsz = PQntuples(pgr.res);
  for(int i =0; i < wsz; ++i){
    out.push_back(ZM_Base::Worker{ stoull(PQgetvalue(pgr.res, i, 0)),
                                   sId,
                                   (ZM_Base::StateType)atoi(PQgetvalue(pgr.res, i, 1)),
                                   atoi(PQgetvalue(pgr.res, i, 2)),
                                   atoi(PQgetvalue(pgr.res, i, 3)),
                                   ZM_Base::Worker::RATING_MAX,
                                   0,
                                   PQgetvalue(pgr.res, i, 4) + string(":") + PQgetvalue(pgr.res, i, 5)});
  }
  return true;
}
bool DbProvider::getNewTasksForSchedr(uint64_t sId, int maxTaskCnt, std::vector<ZM_Base::Task>& out){
  lock_guard<mutex> lk(_mtx);  
  stringstream ss;
  ss << "SELECT * FROM funcNewTasksForSchedr(" << sId << "," << maxTaskCnt << ");";

  PGres pgr(PQexec(_pg, ss.str().c_str()));
  if (PQresultStatus(pgr.res) != PGRES_TUPLES_OK){
    errorMess(string("getNewTasksForSchedr error: ") + PQerrorMessage(_pg));
    return false;
  }
  int tsz = PQntuples(pgr.res);
  for (int i = 0; i < tsz; ++i){
    string params = PQgetvalue(pgr.res, i, 4);
    ZM_Aux::replace(params, "\"", "");
    params = params.substr(1, params.size() - 2); // remove { and }
    out.push_back(ZM_Base::Task{stoull(PQgetvalue(pgr.res, i, 0)),
                                atoi(PQgetvalue(pgr.res, i, 1)),
                                atoi(PQgetvalue(pgr.res, i, 2)),
                                PQgetvalue(pgr.res, i, 3),
                                params});
  }
  return true;
}
bool DbProvider::sendAllMessFromSchedr(uint64_t sId, std::vector<ZM_DB::MessSchedr>& mess){
  lock_guard<mutex> lk(_mtx);
  
  if(mess.empty()){
    return true;
  }
  stringstream ss;
  for (const auto& m : mess){
    switch (m.type){      
      case ZM_Base::MessType::TASK_ERROR:
        ss << "UPDATE tblTaskTime SET "
              "stopTime = current_timestamp "
              "WHERE qtask = " << m.taskId << ";"
              
              "UPDATE tblTaskResult SET "
              "result = '" << m.result << "' "
              "WHERE qtask = " << m.taskId << ";"

              "UPDATE tblTaskState SET "
              "state = " << (int)ZM_Base::StateType::ERROR << " "
              "WHERE qtask = " << m.taskId << ";";
        break; 
      case ZM_Base::MessType::TASK_COMPLETED: 
        ss << "UPDATE tblTaskTime SET "
              "stopTime = current_timestamp "
              "WHERE qtask = " << m.taskId << ";"

              "UPDATE tblTaskResult SET "
              "result = '" << m.result << "' "
              "WHERE qtask = " << m.taskId << ";"
              
              "UPDATE tblTaskState SET "
              "state = " << (int)ZM_Base::StateType::COMPLETED << " "
              "WHERE qtask = " << m.taskId << ";";
        break;  
      case ZM_Base::MessType::TASK_START:   // schedr talk, when send task to worker 
        ss << "UPDATE tblTaskState SET "
              "state = " << (int)ZM_Base::StateType::RUNNING << " "
              "WHERE qtask = " << m.taskId << ";"

              "UPDATE tblTaskQueue SET "
              "worker = " << m.workerId << " "
              "WHERE id = " << m.taskId << ";";              
        break;         
      case ZM_Base::MessType::TASK_RUNNING: // worker talk, when first run task
        ss << "UPDATE tblTaskTime SET "
              "startTime = current_timestamp "
              "WHERE qtask = " << m.taskId << ";";
        break;         
      case ZM_Base::MessType::TASK_PAUSE:
        ss << "UPDATE tblTaskState SET "
              "state = " << (int)ZM_Base::StateType::PAUSE << " "
              "WHERE qtask = " << m.taskId << ";"; 
        break;     
      case ZM_Base::MessType::TASK_CONTINUE: // worker talk, when run task
        ss << "UPDATE tblTaskState SET "
              "state = " << (int)ZM_Base::StateType::RUNNING << " "
              "WHERE qtask = " << m.taskId << ";";                        
        break;    
      case ZM_Base::MessType::TASK_STOP:        
        ss << "UPDATE tblTaskTime SET "
              "stopTime = current_timestamp "
              "WHERE qtask = " << m.taskId << ";"
              
              "UPDATE tblTaskState SET "
              "state = " << (int)ZM_Base::StateType::STOP << " "
              "WHERE qtask = " << m.taskId << ";"; 
        break;
      case ZM_Base::MessType::JUST_START_WORKER:
        ss << "UPDATE tblTaskTime SET "
              "takeInWorkTime = NULL, "
              "startTime = NULL, "
              "stopTime = NULL "
              "FROM tblTaskState ts, tblTaskQueue tq "
              "WHERE tq.id = ts.qtask AND tq.worker = " << m.workerId << " AND (ts.state BETWEEN " << (int)ZM_Base::StateType::RUNNING << " AND " << (int)ZM_Base::StateType::PAUSE << ");" 
              
              "WITH taskUpd AS ("
              "UPDATE tblTaskQueue tq SET "
              "schedr = NULL,"
              "worker = NULL " 
              "FROM tblTaskState ts "
              "WHERE tq.id = ts.qtask AND tq.worker = " << m.workerId << " AND (ts.state BETWEEN " << (int)ZM_Base::StateType::RUNNING << " AND " << (int)ZM_Base::StateType::PAUSE << ") " 
              "RETURNING tq.id) "
                         
              "UPDATE tblTaskState SET "
              "state = " << (int)ZM_Base::StateType::READY << " "
              "WHERE qtask IN (SELECT id FROM taskUpd);"
        
              "UPDATE tblWorker SET "
              "startTime = current_timestamp "
              "WHERE id = " << m.workerId << ";";
        break;
      case ZM_Base::MessType::PROGRESS:
        ss << "UPDATE tblTaskState SET "
              "progress = " << m.progress << " "
              "WHERE qtask = " << m.taskId << ";";
        break;
      case ZM_Base::MessType::PAUSE_SCHEDR:
        ss << "UPDATE tblScheduler SET "
              "state = " << (int)ZM_Base::StateType::PAUSE << " "
              "WHERE id = " << sId << ";";
        break;
      case ZM_Base::MessType::START_SCHEDR:
        ss << "UPDATE tblScheduler SET "
              "state = " << (int)ZM_Base::StateType::RUNNING << ", "
              "startTime = current_timestamp "
              "WHERE id = " << sId << ";";
        break;
      case ZM_Base::MessType::STOP_SCHEDR:
        ss << "UPDATE tblScheduler SET "
              "state = " << (int)ZM_Base::StateType::STOP << ", "
              "stopTime = current_timestamp, "
              "internalData = '" << m.internalData << "' "
              "WHERE id = " << sId << ";";
        break;
      case ZM_Base::MessType::START_AFTER_PAUSE_SCHEDR:
        ss << "UPDATE tblScheduler SET "
              "state = " << (int)ZM_Base::StateType::RUNNING << " "
              "WHERE id = " << sId << ";";
        break;
      case ZM_Base::MessType::PAUSE_WORKER:
        ss << "UPDATE tblWorker SET "
              "state = " << (int)ZM_Base::StateType::PAUSE << " "
              "WHERE id = " << m.workerId << ";";
        break;
      case ZM_Base::MessType::START_WORKER:
        ss << "UPDATE tblWorker SET "
              "state = " << (int)ZM_Base::StateType::RUNNING << ", "
              "startTime = current_timestamp "
              "WHERE id = " << m.workerId << ";";
        break;
      case ZM_Base::MessType::STOP_WORKER:
        ss << "UPDATE tblWorker SET "
              "state = " << (int)ZM_Base::StateType::STOP << ", "
              "stopTime = current_timestamp "
              "WHERE id = " << m.workerId << ";";
        break;
      case ZM_Base::MessType::START_AFTER_PAUSE_WORKER:
        ss << "UPDATE tblWorker SET "
              "state = " << (int)ZM_Base::StateType::RUNNING << " "
              "WHERE id = " << m.workerId << ";";
        break;
      case ZM_Base::MessType::WORKER_NOT_RESPONDING:
        ss << "UPDATE tblTaskTime SET "
              "takeInWorkTime = NULL, "
              "startTime = NULL, "
              "stopTime = NULL "
              "FROM tblTaskState ts, tblTaskQueue tq "
              "WHERE tq.id = ts.qtask AND tq.worker = " << m.workerId << " AND (ts.state BETWEEN " << (int)ZM_Base::StateType::RUNNING << " AND " << (int)ZM_Base::StateType::PAUSE << ");"
              
              "WITH taskUpd AS ("
              "UPDATE tblTaskQueue tq SET "
              "schedr = NULL,"
              "worker = NULL " 
              "FROM tblTaskState ts "
              "WHERE tq.id = ts.qtask AND tq.worker = " << m.workerId << " AND (ts.state BETWEEN " << (int)ZM_Base::StateType::RUNNING << " AND " << (int)ZM_Base::StateType::PAUSE << ") "
              "RETURNING tq.id) "
                         
              "UPDATE tblTaskState SET "
              "state = " << (int)ZM_Base::StateType::READY << " "
              "WHERE qtask IN (SELECT id FROM taskUpd);"

              "UPDATE tblWorker SET "
              "state = " << (int)ZM_Base::StateType::NOT_RESPONDING << ", "
              "stopTime = current_timestamp "
              "WHERE id = " << m.workerId << ";";
        break;
      case ZM_Base::MessType::INTERN_ERROR:
        if (m.workerId){
          ss << "INSERT INTO tblInternError (schedr, worker, message) VALUES("
                "'" << sId << "',"
                "'" << m.workerId << "',"
                "'" << m.result << "');";
        }else{
          ss << "INSERT INTO tblInternError (schedr, message) VALUES("
                "'" << sId << "',"
                "'" << m.result << "');";
        }
        break;
    }    
  }
  PGres pgr(PQexec(_pg, ss.str().c_str()));
  if (PQresultStatus(pgr.res) != PGRES_COMMAND_OK){
    errorMess(string("sendAllMessFromSchedr error: ") + PQerrorMessage(_pg));
    return false;
  }
  return true;
}

// for test
bool DbProvider::delAllTables(){
  lock_guard<mutex> lk(_mtx);
  stringstream ss;
  ss << "DROP TABLE IF EXISTS tblUPipelineTask CASCADE; "
        "DROP TABLE IF EXISTS tblTaskState CASCADE; "
        "DROP TABLE IF EXISTS tblTaskTime CASCADE; "
        "DROP TABLE IF EXISTS tblTaskResult CASCADE; "
        "DROP TABLE IF EXISTS tblPrevTask CASCADE; "
        "DROP TABLE IF EXISTS tblTaskParam CASCADE; "
        "DROP TABLE IF EXISTS tblTaskQueue CASCADE; "
        "DROP TABLE IF EXISTS tblUser CASCADE; "
        "DROP TABLE IF EXISTS tblScheduler CASCADE; "
        "DROP TABLE IF EXISTS tblWorker CASCADE; "
        "DROP TABLE IF EXISTS tblUPipeline CASCADE; "
        "DROP TABLE IF EXISTS tblUTaskTemplate CASCADE; "
        "DROP TABLE IF EXISTS tblInternError CASCADE; "
        "DROP TABLE IF EXISTS tblConnectPnt CASCADE; "
        "DROP TABLE IF EXISTS tblUTaskGroup CASCADE;"
        "DROP FUNCTION IF EXISTS funcStartTask;"
        "DROP FUNCTION IF EXISTS funcTasksOfSchedr;"
        "DROP FUNCTION IF EXISTS funcNewTasksForSchedr;";
  
  PGres pgr(PQexec(_pg, ss.str().c_str()));
  if (PQresultStatus(pgr.res) != PGRES_COMMAND_OK){
      errorMess(PQerrorMessage(_pg));
      return false;
  }
  return true;
}
}