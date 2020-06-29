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
        "launcher     INT NOT NULL REFERENCES tblUser,"
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
        "takeInWorkTime TIMESTAMP CHECK (takeInWorkTime > createTime),"
        "startTime      TIMESTAMP CHECK (startTime > takeInWorkTime),"
        "stopTime       TIMESTAMP CHECK (stopTime > startTime));";
  QUERY(ss.str().c_str(), PGRES_COMMAND_OK);

  ss.str("");
  ss << "CREATE TABLE IF NOT EXISTS tblTaskParam("
        "qtask        INT PRIMARY KEY REFERENCES tblTaskQueue,"        
        "priority     INT NOT NULL DEFAULT 1 CHECK (priority BETWEEN 1 AND 3),"
        "params       TEXT[][3] NOT NULL);"; // [['key', 'sep', 'val'],[..]..]
  QUERY(ss.str().c_str(), PGRES_COMMAND_OK);

  ss.str("");
  ss << "CREATE TABLE IF NOT EXISTS tblTaskResult("
        "qtask        INT PRIMARY KEY REFERENCES tblTaskQueue,"
        "result       TEXT[3] NOT NULL);"; // ['key', 'sep', 'val']
  QUERY(ss.str().c_str(), PGRES_COMMAND_OK);

  ss.str("");
  ss << "CREATE TABLE IF NOT EXISTS tblPrevTask("
        "qtask        INT PRIMARY KEY REFERENCES tblTaskQueue,"        
        "prevTasks     INT[] NOT NULL);";
  QUERY(ss.str().c_str(), PGRES_COMMAND_OK);
  
  ///////////////////////////////////////////////////////////////////////////
  /// USER TABLES
  ss.str("");
  ss << "CREATE TABLE IF NOT EXISTS tblUPipeline("
        "id           SERIAL PRIMARY KEY,"
        "usr          INT NOT NULL REFERENCES tblUser,"
        "name         TEXT NOT NULL CHECK (name <> ''),"
        "description  TEXT NOT NULL,"
        "isDelete     INT NOT NULL DEFAULT 0 CHECK (isDelete BETWEEN 0 AND 1),"
        "isShared     INT NOT NULL DEFAULT 0 CHECK (isShared BETWEEN 0 AND 1));";
  QUERY(ss.str().c_str(), PGRES_COMMAND_OK);

  ss.str("");
  ss << "CREATE TABLE IF NOT EXISTS tblUTaskTemplate("
        "task         INT PRIMARY KEY REFERENCES tblTask,"
        "parent       INT NOT NULL REFERENCES tblUser,"
        "name         TEXT NOT NULL CHECK (name <> ''),"
        "description  TEXT NOT NULL,"
        "isDelete     INT NOT NULL DEFAULT 0 CHECK (isDelete BETWEEN 0 AND 1),"
        "isShared     INT NOT NULL DEFAULT 0 CHECK (isShared BETWEEN 0 AND 1));";
  QUERY(ss.str().c_str(), PGRES_COMMAND_OK);

  ss.str("");
  ss << "CREATE TABLE IF NOT EXISTS tblUPipelineTask("
        "id           SERIAL PRIMARY KEY,"
        "pipeline     INT NOT NULL REFERENCES tblUPipeline,"
        "taskTempl    INT NOT NULL REFERENCES tblUTaskTemplate,"
        "qtask        INT REFERENCES tblTaskQueue,"
        "priority     INT NOT NULL DEFAULT 1 CHECK (priority BETWEEN 1 AND 3),"
        "prevTasks    INT[] NOT NULL,"     // [..]  
        "nextTasks    INT[] NOT NULL,"     // [..]
        "params       TEXT[][3] NOT NULL," // [[key, sep, val],[..],[..]..]
        "result       TEXT[3] NOT NULL,"   // [[key, sep, ""]
        "screenRect   TEXT NOT NULL,"
        "isDelete     INT NOT NULL DEFAULT 0 CHECK (isDelete BETWEEN 0 AND 1));";
  QUERY(ss.str().c_str(), PGRES_COMMAND_OK);
  
  ///////////////////////////////////////////////////////////////////////////
  /// INDEXES


  ///////////////////////////////////////////////////////////////////////////
  /// FUNCTIONS
  ss.str("");
  ss << "CREATE OR REPLACE FUNCTION "
        "funcAddTask(task tblUPipelineTask) "
        "RETURNS int AS $$ "
        "DECLARE "
        "  tId int := 0;"
        "  t int;"
        "BEGIN"
        "  FOREACH t IN ARRAY task.prevTasks"
        "  LOOP"
        "    PERFORM id FROM tblUPipelineTask WHERE id = t;"
        "    IF NOT FOUND THEN"
        "      RETURN 0;"
        "    END IF;"
        "  END LOOP;"
        "  FOREACH t IN ARRAY task.nextTasks"
        "  LOOP"
        "    PERFORM id FROM tblUPipelineTask WHERE id = t;"
        "    IF NOT FOUND THEN"
        "      RETURN 0;"
        "    END IF;"
        "  END LOOP;"
        "  INSERT INTO tblUPipelineTask (pipeline, taskTempl, priority,"
        "                              prevTasks, nextTasks, params, result, screenRect)"
        "    VALUES(task.pipeline,"
        "           task.taskTempl,"
        "           task.priority,"
        "           task.prevTasks,"
        "           task.nextTasks,"
        "           task.params,"
        "           task.result,"
        "           task.screenRect) RETURNING id INTO tId;"
        "  RETURN tId;" 
        "END;"
        "$$ LANGUAGE plpgsql;";
  QUERY(ss.str().c_str(), PGRES_COMMAND_OK);

  ss.str("");
  ss << "CREATE OR REPLACE FUNCTION "
        "funcUpdateTask(task tblUPipelineTask) "
        "RETURNS int AS $$ "
        "DECLARE"
        "  t int;"
        "BEGIN"
        "  FOREACH t IN ARRAY task.prevTasks"
        "  LOOP"
        "    PERFORM id FROM tblUPipelineTask WHERE id = t;"
        "    IF NOT FOUND THEN"
        "      RETURN 0;"
        "    END IF;"
        "  END LOOP;"
        "  FOREACH t IN ARRAY task.nextTasks"
        "  LOOP"
        "    PERFORM id FROM tblUPipelineTask WHERE id = t;"
        "    IF NOT FOUND THEN"
        "      RETURN 0;"
        "    END IF;"
        "  END LOOP;"
        "  UPDATE tblUPipelineTask SET "
        "    pipeline = task.pipeline,"
        "    taskTempl = task.taskTempl,"
        "    priority = task.priority,"
        "    prevTasks = task.prevTasks,"
        "    nextTasks = task.nextTasks,"
        "    params = task.params,"
        "    result = task.result,"
        "    screenRect = task.screenRect"
        "  WHERE id = task.id AND task.isDelete = 0;"
        "  IF NOT FOUND THEN"
        "    RETURN 0;"
        "  END IF;"
        "  RETURN task.id;" 
        "END;"
        "$$ LANGUAGE plpgsql;";
  QUERY(ss.str().c_str(), PGRES_COMMAND_OK);

  ss.str("");
  ss << "CREATE OR REPLACE FUNCTION "
        "funcStartTask(tId int) "
        "RETURNS int AS $$ "
        "DECLARE "
        "  task tblUPipelineTask;"
        "  qId int := 0;"
        "  prqId int := 0;"
        "  prvTasks int[] := ARRAY[]::INT[];"
        "  t int;"
        "BEGIN"
        "  SELECT * INTO task "
        "  FROM tblUPipelineTask "
        "  WHERE id = tId AND isDelete = 0;"
        "  IF NOT FOUND THEN"
        "    RETURN 0;"
        "  END IF;"

        "  INSERT INTO tblTaskQueue (task, launcher) VALUES("
        "    task.taskTempl,"       
        "    (SELECT usr FROM tblUPipeline WHERE id = task.pipeline)) RETURNING id INTO qId;"

        "  INSERT INTO tblTaskState (qtask, state) VALUES("
        "    qId,"
        "    0);"  // ready

        "  INSERT INTO tblTaskTime (qtask) VALUES("
        "    qId);"

        "  INSERT INTO tblTaskParam (qtask, priority, params) VALUES("
        "    qId,"
        "    task.priority,"
        "    task.params);"

        "  INSERT INTO tblTaskResult (qtask, result) VALUES("
        "    qId,"
        "    task.result);"

        "  FOREACH t IN ARRAY task.prevTasks"
        "    LOOP"
        "      SELECT qtask INTO prqId FROM tblUPipelineTask WHERE id = t;"
        "      CONTINUE WHEN NOT FOUND;"
        "      SELECT prvTasks || prqId INTO prvTasks;"
        "    END LOOP;"

        "  INSERT INTO tblPrevTask (qtask, prevTasks) VALUES("
        "    qId,"
        "    prvTasks);"

        "  UPDATE tblUPipelineTask SET"
        "    qtask = qId"
        "  WHERE id = task.id;"

        "  RETURN qId;"
        "END;"
        "$$ LANGUAGE plpgsql;";
  QUERY(ss.str().c_str(), PGRES_COMMAND_OK);

  ss.str("");
  ss << "CREATE OR REPLACE FUNCTION "
        "funcNewTasksForSchedr(sId int, maxTaskCnt int) "
        "RETURNS TABLE("
        "  qid int,"
        "  tid int,"
        "  exr int,"
        "  averDurSec int,"
        "  maxDurSec int,"
        "  script text,"
        "  params text[][],"
        "  prevTasks int[]) AS $$ "
        "DECLARE "
        "  t int;"
        "BEGIN"
        "  <<mBegin>> "
        "  FOR qid, tid, exr, averDurSec, maxDurSec, script, params, prevTasks IN"
        "    SELECT tq.id, tt.id, tt.executor, tt.averDurationSec, tt.maxDurationSec,"
        "           tt.script, tp.params, pt.prevTasks"
        "    FROM tblTask tt "
        "    JOIN tblTaskQueue tq ON tq.task = tt.id "
        "    JOIN tblTaskState ts ON ts.qtask = tq.id "
        "    JOIN tblTaskParam tp ON tp.qtask = tq.id "
        "    JOIN tblPrevTask pt ON pt.qtask = tq.id "
        "    WHERE ts.state = 0 ORDER BY tp.priority LIMIT maxTaskCnt"
        "  LOOP"
        "    FOREACH t IN ARRAY prevTasks"
        "      LOOP"
        "        PERFORM * FROM tblTaskState"
        "        WHERE qtask = t AND state = 5;" // completed
        "        CONTINUE mBegin WHEN NOT FOUND;"

        "        SELECT params || (SELECT result FROM tblTaskResult WHERE qtask = t) INTO params;"
        "      END LOOP;"
        
        "    UPDATE tblTaskQueue SET"
        "      schedr = sId"
        "    WHERE id = qid AND schedr IS NULL;"
        "    CONTINUE mBegin WHEN NOT FOUND;"
        
        "    UPDATE tblTaskState SET"
        "      state = 1"  // start
        "    WHERE qtask = qid;"
        
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
  if (PQresultStatus(res) != PGRES_TUPLES_OK){
    errorMess(string("getUser error: ") + PQerrorMessage(_pg));
    PQclear(res);
    return false;
  }
  if (PQntuples(res) != 1){
    errorMess(string("getUser error: such user does not exist"));
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
  if (PQresultStatus(res) != PGRES_TUPLES_OK){
    errorMess(string("getUser error: ") + PQerrorMessage(_pg));
    PQclear(res);
    return false;
  }
  if (PQntuples(res) != 1){
    errorMess(string("getUser error: such user does not exist"));
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
        "WHERE id = " << userId << " AND isDelete = 0;";

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
bool DbPGProvider::getSchedr(uint64_t sId, ZM_Base::scheduler& cng){
  lock_guard<mutex> lk(_mtx);
  stringstream ss;
  ss << "SELECT cp.ipAddr, cp.port, s.state, s.capacityTask FROM tblScheduler s "
        "JOIN tblConnectPnt cp ON cp.id = s.connPnt "
        "WHERE s.id = " << sId << " AND s.isDelete = 0;";

  auto res = PQexec(_pg, ss.str().c_str());
  if (PQresultStatus(res) != PGRES_TUPLES_OK){
    errorMess(string("getSchedr error: ") + PQerrorMessage(_pg));
    PQclear(res);
    return false;
  }
  if (PQntuples(res) != 1){
    errorMess(string("getSchedr error: such schedr does not exist"));
    PQclear(res);
    return false;
  }
  cng.connectPnt = PQgetvalue(res, 0, 0) + string(":") + PQgetvalue(res, 0, 1);
  cng.state = (ZM_Base::stateType)atoi(PQgetvalue(res, 0, 2));
  cng.capacityTask = atoi(PQgetvalue(res, 0, 3));
  PQclear(res); 
  return true;
}
bool DbPGProvider::changeSchedr(uint64_t sId, const ZM_Base::scheduler& newCng){  
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
        "WHERE id = " << sId << " AND isDelete = 0; "

        "UPDATE tblConnectPnt SET "
        "ipAddr = '" << connPnt[0] << "',"
        "port = '" << connPnt[1] << "' "
        "WHERE id = (SELECT connPnt FROM tblScheduler WHERE id = " << sId << " AND isDelete = 0);";

  auto res = PQexec(_pg, ss.str().c_str());
  if (PQresultStatus(res) != PGRES_COMMAND_OK){
      errorMess(string("changeSchedr error: ") + PQerrorMessage(_pg));
      PQclear(res);
      return false;
  }  
  PQclear(res); 
  return true;
}
bool DbPGProvider::delSchedr(uint64_t sId){  
  lock_guard<mutex> lk(_mtx);
  stringstream ss;
  ss << "UPDATE tblScheduler SET "
        "isDelete = 1 "
        "WHERE id = " << sId << ";";

  auto res = PQexec(_pg, ss.str().c_str());
  if (PQresultStatus(res) != PGRES_COMMAND_OK){
      errorMess(string("delSchedr error: ") + PQerrorMessage(_pg));
      PQclear(res);
      return false;
  }  
  PQclear(res); 
  return true;
}
bool DbPGProvider::schedrState(uint64_t sId, ZM_Base::stateType& state){
  lock_guard<mutex> lk(_mtx);
  stringstream ss;
  ss << "SELECT state FROM tblScheduler "
        "WHERE id = " << sId << " AND isDelete = 0;";

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
bool DbPGProvider::getWorker(uint64_t wId, ZM_Base::worker& cng){
  lock_guard<mutex> lk(_mtx);
  stringstream ss;
  ss << "SELECT cp.ipAddr, cp.port, w.schedr, w.executor, w.state, w.capacityTask FROM tblWorker w "
        "JOIN tblConnectPnt cp ON cp.id = w.connPnt "
        "WHERE w.id = " << wId << " AND w.isDelete = 0;";

  auto res = PQexec(_pg, ss.str().c_str());
  if (PQresultStatus(res) != PGRES_TUPLES_OK){
    errorMess(string("getWorker error: ") + PQerrorMessage(_pg));
    PQclear(res);
    return false;
  }
  if (PQntuples(res) != 1){
    errorMess(string("getWorker error: such worker does not exist"));
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
bool DbPGProvider::changeWorker(uint64_t wId, const ZM_Base::worker& newCng){
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
        "WHERE id = " << wId << " AND isDelete = 0; "

        "UPDATE tblConnectPnt SET "
        "ipAddr = '" << connPnt[0] << "',"
        "port = '" << connPnt[1] << "' "
        "WHERE id = (SELECT connPnt FROM tblWorker WHERE id = " << wId << " AND isDelete = 0);";

  auto res = PQexec(_pg, ss.str().c_str());
  if (PQresultStatus(res) != PGRES_COMMAND_OK){
      errorMess(string("changeWorker error: ") + PQerrorMessage(_pg));
      PQclear(res);
      return false;
  }  
  PQclear(res); 
  return true;
}
bool DbPGProvider::delWorker(uint64_t wId){
  lock_guard<mutex> lk(_mtx);
  stringstream ss;
  ss << "UPDATE tblWorker SET "
        "isDelete = 1 "
        "WHERE id = " << wId << ";";

  auto res = PQexec(_pg, ss.str().c_str());
  if (PQresultStatus(res) != PGRES_COMMAND_OK){
      errorMess(string("delWorker error: ") + PQerrorMessage(_pg));
      PQclear(res);
      return false;
  }  
  PQclear(res); 
  return true;
}
bool DbPGProvider::workerState(const std::vector<uint64_t>& wId, std::vector<ZM_Base::stateType>& state){
  lock_guard<mutex> lk(_mtx);
  string swId;
  swId = accumulate(wId.begin(), wId.end(), swId,
                [](string& s, uint64_t v){
                  return s.empty() ? to_string(v) : s + "," + to_string(v);
                }); 
  stringstream ss;
  ss << "SELECT state FROM tblWorker "
        "WHERE id IN (" << swId << ")  AND isDelete = 0 ORDER BY id;";

  auto res = PQexec(_pg, ss.str().c_str());
  if (PQresultStatus(res) != PGRES_TUPLES_OK){
    errorMess(string("workerState error: ") + PQerrorMessage(_pg));
    PQclear(res);
    return false;
  }
  size_t wsz = wId.size();
  if (PQntuples(res) != wsz){
    errorMess("workerState error: PQntuples(res) != wsz");
    PQclear(res);
    return false;
  }
  state.resize(wsz);
  for (size_t i = 0; i < wsz; ++i){
    state[i] = (ZM_Base::stateType)atoi(PQgetvalue(res, i, 0));
  }
  PQclear(res); 
  return true;
}
std::vector<uint64_t> DbPGProvider::getAllWorkers(uint64_t sId, ZM_Base::stateType state){
  lock_guard<mutex> lk(_mtx);
  stringstream ss;
  ss << "SELECT id FROM tblWorker "
        "WHERE (state = " << (int)state << " OR " << (int)state << " = -1) "
        "AND schedr = " << sId << " "
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
  ss << "INSERT INTO tblUPipeline (usr, name, description, isShared) VALUES("
        "'" << ppl.uId << "',"
        "'" << ppl.name << "',"
        "'" << ppl.description << "',"
        "'" << ppl.isShared << "') RETURNING id;";

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
  ss << "SELECT usr, name, description, isShared FROM tblUPipeline "
        "WHERE id = " << pplId << " AND isDelete = 0;";

  auto res = PQexec(_pg, ss.str().c_str());
  if (PQresultStatus(res) != PGRES_TUPLES_OK){
    errorMess(string("getPipeline error: ") + PQerrorMessage(_pg));
    PQclear(res);
    return false;
  }
  if (PQntuples(res) != 1){
    errorMess(string("getPipeline error: such pipeline does not exist"));
    PQclear(res);
    return false;
  }
  cng.uId = stoull(PQgetvalue(res, 0, 0));
  cng.name = PQgetvalue(res, 0, 1);
  cng.description = PQgetvalue(res, 0, 2);  
  cng.isShared = atoi(PQgetvalue(res, 0, 3)); 
  PQclear(res); 
  return true;
}
bool DbPGProvider::changePipeline(uint64_t pplId, const ZM_Base::uPipeline& newCng){
  lock_guard<mutex> lk(_mtx);
  stringstream ss;
  ss << "UPDATE tblUPipeline SET "
        "usr = '" << newCng.uId << "',"
        "name = '" << newCng.name << "',"
        "description = '" << newCng.description << "', "
        "isShared = '" << newCng.isShared << "' "
        "WHERE id = " << pplId << " AND isDelete = 0;";

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
        "INSERT INTO tblUTaskTemplate (task, parent, name, description, isShared) VALUES("
        "(SELECT id FROM ntsk),"
        "'" << cng.uId << "',"
        "'" << cng.name << "',"
        "'" << cng.description << "',"
        "'" << cng.isShared << "') RETURNING task;";

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
  ss << "SELECT tt.parent, tt.name, tt.description, tt.isShared, t.script, t.executor, t.averDurationSec, t.maxDurationSec "
        "FROM tblUTaskTemplate tt "
        "JOIN tblTask t ON t.id = tt.task "
        "WHERE t.id = " << tId << " AND isDelete = 0;";

  auto res = PQexec(_pg, ss.str().c_str());
  if (PQresultStatus(res) != PGRES_TUPLES_OK){
    errorMess(string("getTaskTemplate error: ") + PQerrorMessage(_pg));
    PQclear(res);
    return false;
  }
  if (PQntuples(res) != 1){
    errorMess(string("getPipeline error: such taskTemplate does not exist"));
    PQclear(res);
    return false;
  }
  outTCng.uId = stoull(PQgetvalue(res, 0, 0));
  outTCng.name = PQgetvalue(res, 0, 1);
  outTCng.description = PQgetvalue(res, 0, 2);  
  outTCng.isShared = atoi(PQgetvalue(res, 0, 3)); 
  outTCng.base.script = PQgetvalue(res, 0, 4);  
  outTCng.base.exr = (ZM_Base::executorType)atoi(PQgetvalue(res, 0, 5));
  outTCng.base.averDurationSec = atoi(PQgetvalue(res, 0, 6));
  outTCng.base.maxDurationSec = atoi(PQgetvalue(res, 0, 7));
  PQclear(res); 
  return true;
};
bool DbPGProvider::changeTaskTemplate(uint64_t tId, const ZM_Base::uTaskTemplate& newTCng, uint64_t& outTId){
  lock_guard<mutex> lk(_mtx);
  stringstream ss;
  ss << "UPDATE tblUTaskTemplate SET "
        "isDelete = 1 "
        "WHERE task = " << tId << ";"
        "WITH ntsk AS (INSERT INTO tblTask (script, executor, averDurationSec, maxDurationSec) VALUES("
        "'" << newTCng.base.script << "',"
        "'" << (int)newTCng.base.exr << "',"
        "'" << newTCng.base.averDurationSec << "',"
        "'" << newTCng.base.maxDurationSec << "') RETURNING id) "
        "INSERT INTO tblUTaskTemplate (task, parent, name, description, isShared) VALUES("
        "(SELECT id FROM ntsk),"
        "'" << newTCng.uId << "',"
        "'" << newTCng.name << "',"
        "'" << newTCng.description << "',"
        "'" << newTCng.isShared << "') RETURNING task;";
  
  auto res = PQexec(_pg, ss.str().c_str());
  if (PQresultStatus(res) != PGRES_TUPLES_OK){
      errorMess(string("changeTaskTemplate error: ") + PQerrorMessage(_pg));
      PQclear(res);
      return false;
  }
  outTId = stoull(PQgetvalue(res, 0, 0));
  PQclear(res); 
  return true;
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
  ss << "SELECT * FROM funcAddTask("
        "(" << 0 << ","
            << cng.pplId << ","
            << cng.base.tId << ","
            << 0 << ","
            << cng.base.priority << ","
            << "ARRAY" << cng.prevTasks << "::INT[],"
            << "ARRAY" << cng.nextTasks << "::INT[],"
            << "ARRAY" << cng.base.params << "::TEXT[][3],"
            << "ARRAY" << cng.base.result << "::TEXT[3],"
            << "'" << cng.screenRect << "'" << ","
            << 0 << "));";

  auto res = PQexec(_pg, ss.str().c_str());
  if (PQresultStatus(res) != PGRES_TUPLES_OK){
      errorMess(string("addTask error: ") + PQerrorMessage(_pg));
      PQclear(res);
      return false;
  }
  outTId = stoull(PQgetvalue(res, 0, 0));
  if (outTId == 0){
    errorMess("addTask error: not found one or more task from prevTask or nextTasks");
    PQclear(res);
    return false;
  }
  PQclear(res); 
  return true;
}
bool DbPGProvider::getTask(uint64_t tId, ZM_Base::uTask& outTCng){
  lock_guard<mutex> lk(_mtx);
  stringstream ss;
  ss << "SELECT pipeline, taskTempl, priority, prevTasks, nextTasks, "
        "params, result, screenRect "
        "FROM tblUPipelineTask "
        "WHERE id = " << tId << " AND isDelete = 0;";

  auto res = PQexec(_pg, ss.str().c_str());
  if (PQresultStatus(res) != PGRES_TUPLES_OK){
    errorMess(string("getTask error: ") + PQerrorMessage(_pg));
    PQclear(res);
    return false;
  }
  if (PQntuples(res) != 1){
    errorMess(string("getTask error: such task does not exist"));
    PQclear(res);
    return false;
  }
  outTCng.pplId = stoull(PQgetvalue(res, 0, 0));
  outTCng.base.tId = stoull(PQgetvalue(res, 0, 1));
  outTCng.base.priority = atoi(PQgetvalue(res, 0, 2));  
  outTCng.prevTasks = PQgetvalue(res, 0, 3);
  ZM_Aux::replace(outTCng.prevTasks, "{", "[");
  ZM_Aux::replace(outTCng.prevTasks, "}", "]");
  
  outTCng.nextTasks = PQgetvalue(res, 0, 4);
  ZM_Aux::replace(outTCng.nextTasks, "{", "[");
  ZM_Aux::replace(outTCng.nextTasks, "}", "]");

  outTCng.base.params = PQgetvalue(res, 0, 5);
  ZM_Aux::replace(outTCng.base.params, "}", "]");
  ZM_Aux::replace(outTCng.base.params, "{", "[");
  ZM_Aux::replace(outTCng.base.params, "[[", "[['");
  ZM_Aux::replace(outTCng.base.params, "]]", "']]");
  ZM_Aux::replace(outTCng.base.params, ",", "','");
  ZM_Aux::replace(outTCng.base.params, "]','[", "'],['");

  outTCng.base.result = PQgetvalue(res, 0, 6);
  ZM_Aux::replace(outTCng.base.result, "}", "']");
  ZM_Aux::replace(outTCng.base.result, "{", "['");
  ZM_Aux::replace(outTCng.base.result, ",", "','");

  outTCng.screenRect = PQgetvalue(res, 0, 7);
  PQclear(res); 
  return true;
}
bool DbPGProvider::changeTask(uint64_t tId, const ZM_Base::uTask& newCng){
  lock_guard<mutex> lk(_mtx);
  stringstream ss;
  ss << "SELECT * FROM funcUpdateTask("
        "(" << tId << ","
            << newCng.pplId << ","
            << newCng.base.tId << ","
            << 0 << ","
            << newCng.base.priority << ","
            << "ARRAY" << newCng.prevTasks << "::INT[],"
            << "ARRAY" << newCng.nextTasks << "::INT[],"
            << "ARRAY" << newCng.base.params << "::TEXT[][3],"
            << "ARRAY" << newCng.base.result << "::TEXT[3],"
            << "'" << newCng.screenRect << "',"
            << 0 << "));";

  auto res = PQexec(_pg, ss.str().c_str());
  if (PQresultStatus(res) != PGRES_TUPLES_OK){
    errorMess(string("changeTask error: ") + PQerrorMessage(_pg));
    PQclear(res);
    return false;
  }
  if (stoull(PQgetvalue(res, 0, 0)) == 0){
    errorMess("changeTask error: previously deleted task is updated OR not found one or more task from prevTask or nextTasks");
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
  stringstream ss;
  ss << "SELECT * FROM funcStartTask(" << tId << ");";

  auto res = PQexec(_pg, ss.str().c_str());
  if (PQresultStatus(res) != PGRES_TUPLES_OK){
    errorMess(string("startTask error: ") + PQerrorMessage(_pg));
    PQclear(res);
    return false;
  }
  if (stoull(PQgetvalue(res, 0, 0)) == 0){
    errorMess("startTask error: previously deleted task is start OR not found one or more task from prevTask or nextTasks");
    PQclear(res);
    return false;
  }
  PQclear(res); 
  return true;
}
bool DbPGProvider::taskState(const std::vector<uint64_t>& tId, std::vector<ZM_DB::taskPrsAState>& outState){
  lock_guard<mutex> lk(_mtx);
  string stId;
  stId = accumulate(tId.begin(), tId.end(), stId,
                [](string& s, uint64_t v){
                  return s.empty() ? to_string(v) : s + "," + to_string(v);
                }); 
  stringstream ss;
  ss << "WITH tblQTaskId AS (SELECT qtask FROM tblUPipelineTask WHERE id IN (" << stId << ") AND isDelete = 0) "
        "SELECT ts.state, ts.progress FROM tblTaskState ts "
        "JOIN tblQTaskId qt ON qt.qtask = ts.qtask;";

  auto res = PQexec(_pg, ss.str().c_str());
  if (PQresultStatus(res) != PGRES_TUPLES_OK){
    errorMess(string("taskState error: ") + PQerrorMessage(_pg));
    PQclear(res);
    return false;
  }
  size_t tsz = tId.size();
  if (PQntuples(res) != tsz){      
    errorMess("taskState error: PQntuples(res) != tsz");
    PQclear(res);        
    return false;
  }
  outState.resize(tsz);
  for (auto& s : outState){
    s.progress = atoi(PQgetvalue(res, 0, 0));
    s.state = (ZM_Base::stateType)atoi(PQgetvalue(res, 0, 1));
  }
  PQclear(res); 
  return true;
}
bool DbPGProvider::taskResult(uint64_t tId, std::string& out){
  lock_guard<mutex> lk(_mtx);
  stringstream ss;
  ss << "SELECT result FROM tblTaskResult "
        "WHERE qtask = (SELECT qtask FROM tblUPipelineTask WHERE id = " << tId << " AND isDelete = 0);";

  auto res = PQexec(_pg, ss.str().c_str());
  if (PQresultStatus(res) != PGRES_TUPLES_OK){      
    errorMess(string("taskResult error: ") + PQerrorMessage(_pg));
    PQclear(res);        
    return false;
  } 
  if (PQntuples(res) != 1){
    errorMess("taskResult error: such task does not exist");
    PQclear(res);        
    return false;
  } 
  out = PQgetvalue(res, 0, 0);
  ZM_Aux::replace(out, "{", "[");
  ZM_Aux::replace(out, "}", "]");
  ZM_Aux::replace(out, ",", "','");
  PQclear(res); 
  return true;
}
bool DbPGProvider::taskTime(uint64_t tId, ZM_DB::taskTime& out){
  lock_guard<mutex> lk(_mtx);
  stringstream ss;
  ss << "SELECT createTime, takeInWorkTime, startTime, stopTime FROM tblTaskTime "
        "WHERE qtask = (SELECT qtask FROM tblUPipelineTask WHERE id = " << tId << " AND isDelete = 0);";

  auto res = PQexec(_pg, ss.str().c_str());
  if (PQresultStatus(res) != PGRES_TUPLES_OK){      
    errorMess(string("taskTime error: ") + PQerrorMessage(_pg));
    PQclear(res);        
    return false;
  } 
  if (PQntuples(res) != 1){
    errorMess("taskTime error: such task does not exist");
    PQclear(res);        
    return false;
  } 
  out.createTime = PQgetvalue(res, 0, 0);
  out.takeInWorkTime = PQgetvalue(res, 0, 1);
  out.startTime = PQgetvalue(res, 0, 2);
  out.stopTime = PQgetvalue(res, 0, 3);
  PQclear(res); 
  return true;
}
std::vector<uint64_t> DbPGProvider::getAllTasks(uint64_t pplId, ZM_Base::stateType state){  
  lock_guard<mutex> lk(_mtx);
  stringstream ss;
  ss << "SELECT ut.id FROM tblUPipelineTask ut "
        "LEFT JOIN tblTaskState ts ON ts.qtask = ut.qtask "
        "WHERE (ts.state = " << (int)state << " OR " << (int)state << " = -1) "
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
  if (PQresultStatus(res) != PGRES_TUPLES_OK){
    errorMess(string("getSchedr error: ") + PQerrorMessage(_pg));
    PQclear(res);
    return false;
  }
  if (PQntuples(res) == 0){
    errorMess("getSchedr error: such schedr does not exist");
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
bool DbPGProvider::getTasksOfSchedr(uint64_t sId, std::vector<ZM_DB::schedrTask>& out){
  lock_guard<mutex> lk(_mtx);
  stringstream ss;
  ss << "SELECT tq.id, t.id, t.executor, t.averDurationSec, t.maxDurationSec, t.script, tp.params "
        "FROM tblTask t "
        "JOIN tblTaskQueue tq ON tq.task = t.id "
        "JOIN tblTaskState ts ON ts.qtask = tq.id "
        "JOIN tblTaskParam tp ON tp.qtask = tq.id "
        "WHERE tq.schedr = " << sId << " AND (ts.state BETWEEN 1 AND 3)"; // 1 - start, 2 - running, 3 - pause
        
  auto resTsk = PQexec(_pg, ss.str().c_str());
  if (PQresultStatus(resTsk) != PGRES_TUPLES_OK){
    errorMess(string("getTasksOfSchedr error: ") + PQerrorMessage(_pg));
    PQclear(resTsk);
    return false;
  }
  int tsz = PQntuples(resTsk);
  for (int i = 0; i < tsz; ++i){    
    out.push_back(ZM_DB::schedrTask{stoull(PQgetvalue(resTsk, i, 0)),
                                    ZM_Base::task{stoull(PQgetvalue(resTsk, i, 1)),
                                                  (ZM_Base::executorType)atoi(PQgetvalue(resTsk, i, 2)),
                                                  atoi(PQgetvalue(resTsk, i, 3)),
                                                  atoi(PQgetvalue(resTsk, i, 4)),
                                                  PQgetvalue(resTsk, i, 5)
                                                },
                                    PQgetvalue(resTsk, i, 6)});
  }
  PQclear(resTsk);
  return true;
}
bool DbPGProvider::getWorkersOfSchedr(uint64_t sId, std::vector<ZM_Base::worker>& out){
  lock_guard<mutex> lk(_mtx);
  stringstream ss;
  ss << "SELECT w.id, w.state, w.executor, w.capacityTask, w.activeTask, w.rating, cp.ipAddr, cp.port "
        "FROM tblWorker w "
        "JOIN tblConnectPnt cp ON cp.id = w.connPnt "
        "WHERE w.schedr = " << sId << " AND w.isDelete = 0;";

  auto res = PQexec(_pg, ss.str().c_str());
  if (PQresultStatus(res) != PGRES_TUPLES_OK){
    errorMess(string("getWorkersOfSchedr error: ") + PQerrorMessage(_pg));
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
bool DbPGProvider::getNewTasksForSchedr(uint64_t sId, int maxTaskCnt, std::vector<ZM_DB::schedrTask>& out){
  lock_guard<mutex> lk(_mtx);  
  stringstream ss;
  ss << "SELECT * FROM funcNewTasksForSchedr(" << sId << "," << maxTaskCnt << ");";

  auto res = PQexec(_pg, ss.str().c_str());
  if (PQresultStatus(res) != PGRES_TUPLES_OK){
    errorMess(string("getNewTasksForSchedr error: ") + PQerrorMessage(_pg));
    PQclear(res);
    return false;
  }
  int tsz = PQntuples(res);
  for (int i = 0; i < tsz; ++i){    
    out.push_back(ZM_DB::schedrTask{stoull(PQgetvalue(res, i, 0)),
                                    ZM_Base::task{stoull(PQgetvalue(res, i, 1)),
                                                  (ZM_Base::executorType)atoi(PQgetvalue(res, i, 2)),
                                                  atoi(PQgetvalue(res, i, 3)),
                                                  atoi(PQgetvalue(res, i, 4)),
                                                  PQgetvalue(res, i, 5)
                                                },
                                    PQgetvalue(res, i, 6)});
  }
  PQclear(res);
  return true;
}
bool DbPGProvider::sendAllMessFromSchedr(uint64_t sId, std::vector<ZM_DB::messSchedr>& mess){
  lock_guard<mutex> lk(_mtx);

  stringstream ss;
  for (auto& m : mess){
    switch (m.type){
      case ZM_Base::messType::taskError:
        ss << "UPDATE tblTaskState SET "
              "state = " << (int)ZM_Base::stateType::error << " "
              "WHERE qtask = " << m.taskId << ";"

              "UPDATE tblTaskTime SET "
              "stopTime = current_timestamp "
              "WHERE qtask = " << m.taskId << ";"
              
              "UPDATE tblTaskResult SET "
              "result = ARRAY" << m.result << "::TEXT[3] "
              "WHERE qtask = " << m.taskId << ";";
        break; 
      case ZM_Base::messType::taskCompleted: 
        ss << "UPDATE tblTaskState SET "
              "state = " << (int)ZM_Base::stateType::completed << " "
              "WHERE qtask = " << m.taskId << ";"

              "UPDATE tblTaskTime SET "
              "stopTime = current_timestamp "
              "WHERE qtask = " << m.taskId << ";"

              "UPDATE tblTaskResult SET "
              "result = ARRAY" << m.result << "::TEXT[3] "
              "WHERE qtask = " << m.taskId << ";";
        break;  
      case ZM_Base::messType::taskStart: 
      case ZM_Base::messType::taskRunning: 
        ss << "UPDATE tblTaskQueue SET "
              "worker = " << m.workerId << " "
              "WHERE id = " << m.taskId << ";"

              "UPDATE tblTaskState SET "
              "state = " << (int)ZM_Base::stateType::running << " "
              "WHERE qtask = " << m.taskId << ";"

              "UPDATE tblTaskTime SET "
              "startTime = current_timestamp "
              "WHERE qtask = " << m.taskId << " AND startTime IS NULL;";              
        break;         
      case ZM_Base::messType::taskPause:
        ss << "UPDATE tblTaskState SET "
              "state = " << (int)ZM_Base::stateType::pause << " "
              "WHERE qtask = " << m.taskId << ";";
        break;        
      case ZM_Base::messType::taskStop:        
        ss << "UPDATE tblTaskState SET "
              "state = " << (int)ZM_Base::stateType::stop << " "
              "WHERE qtask = " << m.taskId << ";"

              "UPDATE tblTaskTime SET "
              "stopTime = current_timestamp "
              "WHERE qtask = " << m.taskId << ";";
        break;
      case ZM_Base::messType::justStartWorker:
        ss << "WITH taskUpd AS ("
              "UPDATE tblTaskQueue tq SET "
              "schedr = NULL,"
              "worker = NULL " 
              "FROM tblTaskState ts "
              "WHERE tq.id = ts.qtask AND tq.worker = " << m.workerId << " AND (ts.state BETWEEN 2 AND 3) " // running, pause
              "RETURNING tq.id) "

              "UPDATE tblTaskState SET "
              "state = " << (int)ZM_Base::stateType::ready << " "
              "WHERE qtask = (SELECT id FROM taskUpd);";
        break;
      case ZM_Base::messType::progress:
        ss << "UPDATE tblTaskState SET "
              "progress = " << m.progress << " "
              "WHERE qtask = " << m.taskId << ";";
        break;
      case ZM_Base::messType::pauseSchedr:
        ss << "UPDATE tblScheduler SET "
              "state = " << (int)ZM_Base::stateType::pause << " "
              "WHERE id = " << sId << ";";
        break;
      case ZM_Base::messType::startSchedr:
        ss << "UPDATE tblScheduler SET "
              "state = " << (int)ZM_Base::stateType::running << " "
              "WHERE id = " << sId << ";";
        break;
      case ZM_Base::messType::pauseWorker:
        ss << "UPDATE tblWorker SET "
              "state = " << (int)ZM_Base::stateType::pause << " "
              "WHERE id = " << m.workerId << ";";
        break;
      case ZM_Base::messType::startWorker:
        ss << "UPDATE tblWorker SET "
              "state = " << (int)ZM_Base::stateType::running << " "
              "WHERE id = " << m.workerId << ";";
        break;
      case ZM_Base::messType::workerRating:
        ss << "UPDATE tblWorker SET "
              "rating = " << m.workerRating << " "
              "WHERE id = " << m.workerId << ";";
        break;
      case ZM_Base::messType::workerNotResponding:
        ss << "WITH taskUpd AS ("
              "UPDATE tblTaskQueue tq SET "
              "schedr = NULL,"
              "worker = NULL " 
              "FROM tblTaskState ts "
              "WHERE tq.id = ts.qtask AND tq.worker = " << m.workerId << " AND (ts.state BETWEEN 2 AND 3) " // running, pause
              "RETURNING tq.id) "

              "UPDATE tblTaskState SET "
              "state = " << (int)ZM_Base::stateType::ready << " "
              "WHERE qtask = (SELECT id FROM taskUpd);"

              "UPDATE tblWorker SET "
              "state = " << (int)ZM_Base::stateType::notResponding << " "
              "WHERE id = " << m.workerId << ";";
        break;
    }    
  }
  auto res = PQexec(_pg, ss.str().c_str());
  if (PQresultStatus(res) != PGRES_COMMAND_OK){
    errorMess(string("sendAllMessFromSchedr error: ") + PQerrorMessage(_pg));
    PQclear(res);
    return false;
  }
  PQclear(res);
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
  ss << "TRUNCATE tblUPipelineTask CASCADE;"
        "TRUNCATE tblTaskQueue CASCADE;";
  
  auto res = PQexec(_pg, ss.str().c_str());
  if (PQresultStatus(res) != PGRES_COMMAND_OK){
      errorMess(PQerrorMessage(_pg));
      PQclear(res);
      return false;
  }
  return true;
}
#endif // DEBUG