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
        "id           SERIAL PRIMARY KEY,"
        "name         TEXT NOT NULL UNIQUE CHECK (name <> ''),"
        "passwHash    TEXT NOT NULL,"
        "description  TEXT NOT NULL,"
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
  ss << "CREATE TABLE IF NOT EXISTS tblTask("
        "id           SERIAL PRIMARY KEY,"
        "script       TEXT NOT NULL CHECK (script <> ''),"
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
        "(7, 'cancel'),"
        "(8, 'notResponding') ON CONFLICT (id) DO NOTHING;";
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
        "capacityTask INT NOT NULL DEFAULT 10 CHECK (capacityTask > 0),"
        "activeTask   INT NOT NULL DEFAULT 0 CHECK (activeTask >= 0),"
        "rating       INT NOT NULL DEFAULT 10 CHECK (rating BETWEEN 1 AND 10),"
        "isDelete     INT NOT NULL DEFAULT 0 CHECK (isDelete BETWEEN 0 AND 1));";
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
        "task         INT NOT NULL REFERENCES tblTask,"       
        "usr          INT NOT NULL REFERENCES tblUser,"
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
        "usr          INT NOT NULL REFERENCES tblUser,"
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
        "params       TEXT[] NOT NULL,"    // ['param1','param2'..]
        "screenRect   TEXT NOT NULL,"
        "isDelete     INT NOT NULL DEFAULT 0 CHECK (isDelete BETWEEN 0 AND 1));";
  QUERY(ss.str().c_str(), PGRES_COMMAND_OK);
  
  ///////////////////////////////////////////////////////////////////////////
  /// INDEXES
  ss.str(""); 
  ss << "CREATE INDEX IF NOT EXISTS inxTSState ON tblTaskState(state);"
        "CREATE INDEX IF NOT EXISTS inxPPTQTask ON tblUPipelineTask(qtask);"
        "CREATE INDEX IF NOT EXISTS inxPPTQTaskTempl ON tblUPipelineTask(taskTempl);"
        "CREATE INDEX IF NOT EXISTS inxTQWorker ON tblTaskQueue(worker);"
        "CREATE INDEX IF NOT EXISTS inxIECreateTime ON tblInternError(createTime);"
        "CREATE INDEX IF NOT EXISTS inxTTCreateTime ON tblTaskTime(createTime);";

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
        "    LOOP"
        "      PERFORM id FROM tblUPipelineTask WHERE id = t;"
        "      IF NOT FOUND THEN"
        "        RETURN 0;"
        "      END IF;"
        "    END LOOP;"
        "  FOREACH t IN ARRAY task.nextTasks"
        "    LOOP"
        "      PERFORM id FROM tblUPipelineTask WHERE id = t;"
        "      IF NOT FOUND THEN"
        "        RETURN 0;"
        "      END IF;"
        "    END LOOP;"
        "  INSERT INTO tblUPipelineTask (pipeline, taskTempl, priority,"
        "                              prevTasks, nextTasks, params, screenRect)"
        "    VALUES(task.pipeline,"
        "           task.taskTempl,"
        "           task.priority,"
        "           task.prevTasks,"
        "           task.nextTasks,"
        "           task.params,"
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
        "    LOOP"
        "      PERFORM id FROM tblUPipelineTask WHERE id = t;"
        "      IF NOT FOUND THEN"
        "        RETURN 0;"
        "      END IF;"
        "    END LOOP;"
        "  FOREACH t IN ARRAY task.nextTasks"
        "    LOOP"
        "      PERFORM id FROM tblUPipelineTask WHERE id = t;"
        "      IF NOT FOUND THEN"
        "        RETURN 0;"
        "      END IF;"
        "    END LOOP;"
        "  UPDATE tblUPipelineTask SET "
        "    pipeline = task.pipeline,"
        "    taskTempl = task.taskTempl,"
        "    priority = task.priority,"
        "    prevTasks = task.prevTasks,"
        "    nextTasks = task.nextTasks,"
        "    params = task.params,"
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

        "  INSERT INTO tblTaskQueue (task, usr) VALUES("
        "    task.taskTempl,"
        "    (SELECT usr FROM tblUPipeline WHERE id = task.pipeline)) RETURNING id INTO qId;"

        "  UPDATE tblUPipelineTask SET"
        "    qtask = qId"
        "  WHERE id = task.id;"

        "  INSERT INTO tblTaskTime (qtask) VALUES("
        "    qId);"

        "  INSERT INTO tblTaskParam (qtask, priority, params) VALUES("
        "    qId,"
        "    task.priority,"
        "    task.params);"

        "  INSERT INTO tblTaskResult (qtask, result) VALUES("
        "    qId,"
        "    '');"

        "  FOREACH t IN ARRAY task.prevTasks"
        "    LOOP"
        "      SELECT qtask INTO prqId FROM tblUPipelineTask WHERE id = t;"
        "      CONTINUE WHEN prqId IS NULL;"
        "      SELECT prvTasks || prqId INTO prvTasks;"
        "    END LOOP;"

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
        "  tid int,"
        "  averDurSec int,"
        "  maxDurSec int,"
        "  script text,"
        "  params text[],"
        "  prevTasks int[]) AS $$ "
        "DECLARE "
        "  t int;"
        "BEGIN"
        "  FOR qid, tid, averDurSec, maxDurSec, script, params, prevTasks IN"
        "    SELECT tq.id, tt.id, tt.averDurationSec, tt.maxDurationSec,"
        "           tt.script, tp.params, pt.prevTasks"
        "    FROM tblTask tt "
        "    JOIN tblTaskQueue tq ON tq.task = tt.id "
        "    JOIN tblTaskState ts ON ts.qtask = tq.id "
        "    JOIN tblTaskParam tp ON tp.qtask = tq.id "
        "    JOIN tblPrevTask pt ON pt.qtask = tq.id "
        "    WHERE tq.schedr = sId AND (ts.state BETWEEN 1 AND 3)" // 1 - start, 2 - running, 3 - pause
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
        "  tid int,"
        "  averDurSec int,"
        "  maxDurSec int,"
        "  script text,"
        "  params text[],"
        "  prevTasks int[]) AS $$ "
        "DECLARE "
        "  t int;"
        "BEGIN"
        "  <<mBegin>> "
        "  FOR qid, tid, averDurSec, maxDurSec, script, params, prevTasks IN"
        "    SELECT tq.id, tt.id, tt.averDurationSec, tt.maxDurationSec,"
        "           tt.script, tp.params, pt.prevTasks"
        "    FROM tblTask tt "
        "    JOIN tblTaskQueue tq ON tq.task = tt.id "
        "    JOIN tblTaskState ts ON ts.qtask = tq.id "
        "    JOIN tblTaskParam tp ON tp.qtask = tq.id "
        "    JOIN tblTaskTime tm ON tm.qtask = tq.id "
        "    JOIN tblPrevTask pt ON pt.qtask = tq.id "
        "    WHERE ts.state = " << int(ZM_Base::stateType::ready) << ""
        "      AND tq.schedr IS NULL AND tm.takeInWorkTime IS NULL "
        "    ORDER BY tp.priority LIMIT maxTaskCnt "
        "    FOR UPDATE OF tq SKIP LOCKED"
        "  LOOP"
        "    FOREACH t IN ARRAY prevTasks"
        "      LOOP"
        "        PERFORM * FROM tblTaskState"
        "        WHERE qtask = t AND state = " << int(ZM_Base::stateType::completed) << ";"
        "        CONTINUE mBegin WHEN NOT FOUND;"

        "        SELECT params || (SELECT result FROM tblTaskResult WHERE qtask = t) INTO params;"
        "      END LOOP;"
        
        "    UPDATE tblTaskQueue SET"
        "      schedr = sId"
        "    WHERE id = qid;"
        
        "    UPDATE tblTaskState SET"
        "      state = " << int(ZM_Base::stateType::start) << ""
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
        "INSERT INTO tblWorker (connPnt, schedr, state, capacityTask) VALUES("
        "(SELECT id FROM ncp),"
        "'" << (int)worker.sId << "',"
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
  ss << "SELECT cp.ipAddr, cp.port, w.schedr, w.state, w.capacityTask FROM tblWorker w "
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
  cng.state = (ZM_Base::stateType)atoi(PQgetvalue(res, 0, 3));
  cng.capacityTask = atoi(PQgetvalue(res, 0, 4));
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
  ss << "WITH ntsk AS (INSERT INTO tblTask (script, averDurationSec, maxDurationSec) VALUES("
        "'" << cng.base.script << "',"
        "'" << cng.base.averDurationSec << "',"
        "'" << cng.base.maxDurationSec << "') RETURNING id) "
        "INSERT INTO tblUTaskTemplate (task, usr, name, description, isShared) VALUES("
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
  ss << "SELECT tt.usr, tt.name, tt.description, tt.isShared, t.script, t.averDurationSec, t.maxDurationSec "
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
  outTCng.base.averDurationSec = atoi(PQgetvalue(res, 0, 5));
  outTCng.base.maxDurationSec = atoi(PQgetvalue(res, 0, 6));
  PQclear(res); 
  return true;
};
bool DbPGProvider::changeTaskTemplate(uint64_t tId, const ZM_Base::uTaskTemplate& newTCng, uint64_t& outTId){
  lock_guard<mutex> lk(_mtx);
  stringstream ss;
  ss << "UPDATE tblUTaskTemplate SET "
        "isDelete = 1 "
        "WHERE task = " << tId << ";"
        
        "WITH ntsk AS (INSERT INTO tblTask (script, averDurationSec, maxDurationSec) VALUES("
        "'" << newTCng.base.script << "',"
        "'" << newTCng.base.averDurationSec << "',"
        "'" << newTCng.base.maxDurationSec << "') RETURNING id) "
        "INSERT INTO tblUTaskTemplate (task, usr, name, description, isShared) VALUES("
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

  ////////////////////////////////////////////////////////////////////////

  ss.str(""); 
  ss << "UPDATE tblUPipelineTask SET "
        "taskTempl = "<< outTId << " "
        "WHERE taskTempl = " << tId << ";";
  
  res = PQexec(_pg, ss.str().c_str());
  if (PQresultStatus(res) != PGRES_COMMAND_OK){
      errorMess(string("changeTaskTemplate error: ") + PQerrorMessage(_pg));
      PQclear(res);
      return false;
  }
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
std::vector<uint64_t> DbPGProvider::getAllTaskTemplates(uint64_t usr){
  lock_guard<mutex> lk(_mtx);
  stringstream ss;
  ss << "SELECT task FROM tblUTaskTemplate "
        "WHERE usr = " << usr << " AND isDelete = 0;";

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
  
  string prevTasks = "[" + cng.prevTasks + "]",
         nextTasks = "[" + cng.nextTasks + "]",
         params = "['" + cng.base.params + "']";
  ZM_Aux::replace(params, ",", "','");
  
  stringstream ss;
  ss << "SELECT * FROM funcAddTask("
        "(" << 0 << ","
            << cng.pplId << ","
            << cng.base.tId << ","
            << 0 << ","
            << cng.base.priority << ","
            << "ARRAY" << prevTasks << "::INT[],"
            << "ARRAY" << nextTasks << "::INT[],"
            << "ARRAY" << params << "::TEXT[],"
            << "'" << cng.screenRect << "',"
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
        "params, screenRect "
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
  outTCng.prevTasks = outTCng.prevTasks.substr(1,  outTCng.prevTasks.size() - 2);
  
  outTCng.nextTasks = PQgetvalue(res, 0, 4);
  outTCng.nextTasks = outTCng.nextTasks.substr(1,  outTCng.nextTasks.size() - 2);
  
  outTCng.base.params = PQgetvalue(res, 0, 5);
  ZM_Aux::replace(outTCng.base.params, "\"", "");
  outTCng.base.params = outTCng.base.params.substr(1,  outTCng.base.params.size() - 2);  

  outTCng.screenRect = PQgetvalue(res, 0, 6);
  PQclear(res); 
  return true;
}
bool DbPGProvider::changeTask(uint64_t tId, const ZM_Base::uTask& newCng){
  lock_guard<mutex> lk(_mtx);

  string prevTasks = "[" + newCng.prevTasks + "]",
         nextTasks = "[" + newCng.nextTasks + "]",
         params = "['" + newCng.base.params + "']";
  ZM_Aux::replace(params, ",", "','");

  stringstream ss;
  ss << "SELECT * FROM funcUpdateTask("
        "(" << tId << ","
            << newCng.pplId << ","
            << newCng.base.tId << ","
            << 0 << ","
            << newCng.base.priority << ","
            << "ARRAY" << prevTasks << "::INT[],"
            << "ARRAY" << nextTasks << "::INT[],"
            << "ARRAY" << params << "::TEXT[],"
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
    errorMess("startTask error: previously deleted task is start OR \
               not found one or more task from prevTask or nextTasks");
    PQclear(res);
    return false;
  }
  PQclear(res); 
  return true;
}
bool DbPGProvider::cancelTask(uint64_t tId){
  lock_guard<mutex> lk(_mtx);
  stringstream ss;
  ss << "UPDATE tblTaskState ts SET "
        "state = " << int(ZM_Base::stateType::cancel) << " "
        "FROM tblTaskQueue tq, tblUPipelineTask tpp "
        "WHERE tpp.id = " << tId << " AND "
        "      tpp.qtask = tq.id AND "
        "      ts.qtask = tq.id AND "
        "      ts.state = " << int(ZM_Base::stateType::ready) << " AND "
        "      tpp.isDelete = 0 "
        "RETURNING ts.qtask;";
        
  auto res = PQexec(_pg, ss.str().c_str());
  if (PQresultStatus(res) != PGRES_TUPLES_OK){
    errorMess(string("cancelTask error: ") + PQerrorMessage(_pg));
    PQclear(res);
    return false;
  }  
  if (PQntuples(res) != 1){
    errorMess(string("cancelTask error: task already take in work") + PQerrorMessage(_pg));
    PQclear(res);  
    return false;
  }
  PQclear(res); 
  return true;
}
bool DbPGProvider::taskState(const std::vector<uint64_t>& tId, std::vector<ZM_DB::tskState>& outState){
  lock_guard<mutex> lk(_mtx);
  string stId;
  stId = accumulate(tId.begin(), tId.end(), stId,
                [](string& s, uint64_t v){
                  return s.empty() ? to_string(v) : s + "," + to_string(v);
                }); 
  stringstream ss;
  ss << "SELECT pt.id, ts.state, ts.progress "
        "FROM tblTaskState ts "
        "JOIN tblTaskQueue qt ON qt.id = ts.qtask "
        "JOIN tblUPipelineTask pt ON pt.qtask = qt.id "
        "WHERE pt.id IN (" << stId << ") AND pt.isDelete = 0 ORDER BY pt.id;";

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
  for (size_t i = 0; i < tsz; ++i){
    outState[i].state = (ZM_Base::stateType)atoi(PQgetvalue(res, i, 1));
    outState[i].progress = atoi(PQgetvalue(res, i, 2));
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
    errorMess("taskResult error: task delete OR not taken to work");
    PQclear(res);        
    return false;
  } 
  out = PQgetvalue(res, 0, 0);
  PQclear(res); 
  return true;
}
bool DbPGProvider::taskTime(uint64_t tId, ZM_DB::taskTime& out){
  lock_guard<mutex> lk(_mtx);
  stringstream ss;
  ss << "SELECT createTime, takeInWorkTime, startTime, stopTime "
        "FROM tblTaskTime "
        "WHERE qtask = (SELECT qtask FROM tblUPipelineTask WHERE id = " << tId << " AND isDelete = 0);";

  auto res = PQexec(_pg, ss.str().c_str());
  if (PQresultStatus(res) != PGRES_TUPLES_OK){      
    errorMess(string("taskTime error: ") + PQerrorMessage(_pg));
    PQclear(res);        
    return false;
  } 
  if (PQntuples(res) != 1){
    errorMess("taskTime error: task delete OR not taken to work");
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
bool DbPGProvider::getWorkerByTask(uint64_t tId, uint64_t& qtId, ZM_Base::worker& wcng){
  lock_guard<mutex> lk(_mtx);
  stringstream ss;
  ss << "SELECT tq.id, wkr.id, wkr.connPnt "
        "FROM tblTaskQueue tq "
        "JOIN tblUPipelineTask tpp ON tpp.qtask = tq.id "
        "JOIN tblWorker wkr ON wkr.id = tq.worker "
        "WHERE tpp.id = " << tId << " AND tpp.isDelete = 0;";

  auto res = PQexec(_pg, ss.str().c_str());
  if (PQresultStatus(res) != PGRES_TUPLES_OK){
    errorMess(string("getWorkerByTask error: ") + PQerrorMessage(_pg));
    PQclear(res);
    return false;
  }
  if (PQntuples(res) != 1){
    errorMess(string("getWorkerByTask error: task delete OR not taken to work"));
    PQclear(res);
    return false;
  }
  qtId = stoull(PQgetvalue(res, 0, 0));
  wcng.id = stoull(PQgetvalue(res, 0, 1));
  wcng.connectPnt = PQgetvalue(res, 0, 2);

  PQclear(res); 
  return true;
}

vector<ZM_DB::messError> DbPGProvider::getInternErrors(uint64_t sId, uint64_t wId, uint32_t mCnt){
  lock_guard<mutex> lk(_mtx);
  if (mCnt == 0){
    mCnt = INT32_MAX;
  }
  stringstream ss;
  ss << "SELECT schedr, worker, message, createTime "
        "FROM tblInternError "
        "WHERE (schedr = " << sId << " OR " << sId << " = 0)" << " AND "
        "      (worker = " << wId << " OR " << wId << " = 0) ORDER BY createTime LIMIT " << mCnt << ";";

  auto res = PQexec(_pg, ss.str().c_str());
  if (PQresultStatus(res) != PGRES_TUPLES_OK){
    errorMess(string("getInternErrors error: ") + PQerrorMessage(_pg));
    PQclear(res);
    return vector<ZM_DB::messError>();
  }  
  int rows = PQntuples(res);
  std::vector<ZM_DB::messError> ret(rows);
  for (int i = 0; i < rows; ++i){
    ret[i].schedrId = stoull(PQgetvalue(res, i, 0));
    ret[i].workerId = stoull(PQgetvalue(res, i, 1));
    ret[i].message = PQgetvalue(res, i, 2);
    ret[i].createTime = PQgetvalue(res, i, 3);
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
  ss << "SELECT * FROM funcTasksOfSchedr(" << sId << ");";
     
  auto res = PQexec(_pg, ss.str().c_str());
  if (PQresultStatus(res) != PGRES_TUPLES_OK){
    errorMess(string("getTasksOfSchedr error: ") + PQerrorMessage(_pg));
    PQclear(res);
    return false;
  }
  int tsz = PQntuples(res);
  for (int i = 0; i < tsz; ++i){   

    string params = PQgetvalue(res, i, 5);
    ZM_Aux::replace(params, "\"", "");
    params = string(params.data() + 1, params.size() - 2); // remove '{' and '}'

    out.push_back(ZM_DB::schedrTask{stoull(PQgetvalue(res, i, 0)),
                                    ZM_Base::task{stoull(PQgetvalue(res, i, 1)),
                                                  atoi(PQgetvalue(res, i, 2)),
                                                  atoi(PQgetvalue(res, i, 3)),
                                                  PQgetvalue(res, i, 4)
                                                },
                                    params});
  }
  PQclear(res);
  return true;
}
bool DbPGProvider::getWorkersOfSchedr(uint64_t sId, std::vector<ZM_Base::worker>& out){
  lock_guard<mutex> lk(_mtx);
  stringstream ss;
  ss << "SELECT w.id, w.state, w.capacityTask, w.activeTask, w.rating, cp.ipAddr, cp.port "
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
                                   atoi(PQgetvalue(res, i, 2)),
                                   atoi(PQgetvalue(res, i, 3)),
                                   atoi(PQgetvalue(res, i, 4)),
                                   PQgetvalue(res, i, 5) + string(":") + PQgetvalue(res, i, 6)});
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

    string params = PQgetvalue(res, i, 5);
    ZM_Aux::replace(params, "\"", "");
    params = string(params.data() + 1, params.size() - 2); // remove '{' and '}'
    
    out.push_back(ZM_DB::schedrTask{stoull(PQgetvalue(res, i, 0)),
                                    ZM_Base::task{stoull(PQgetvalue(res, i, 1)),
                                                  atoi(PQgetvalue(res, i, 2)),
                                                  atoi(PQgetvalue(res, i, 3)),
                                                  PQgetvalue(res, i, 4)
                                                },
                                    params});
  }
  PQclear(res);
  return true;
}
bool DbPGProvider::sendAllMessFromSchedr(uint64_t sId, std::vector<ZM_DB::messSchedr>& mess){
  lock_guard<mutex> lk(_mtx);
  
  if(mess.empty()){
    return true;
  }
  stringstream ss;
  for (auto& m : mess){
    switch (m.type){      
      case ZM_Base::messType::taskError:
        ss << "UPDATE tblTaskTime tt SET "
              "stopTime = current_timestamp "
              "FROM tblTaskQueue tq "
              "WHERE tt.qtask = " << m.taskId << " AND tq.worker = " << m.workerId << " AND tt.stopTime IS NULL;"
              
              "UPDATE tblTaskResult tr SET "
              "result = '" << m.result << "' "
              "FROM tblTaskQueue tq "
              "WHERE tr.qtask = " << m.taskId << " AND tq.worker = " << m.workerId << " AND tr.result = '';"

              "UPDATE tblTaskState ts SET "
              "state = " << (int)ZM_Base::stateType::error << " "
              "FROM tblTaskQueue tq "
              "WHERE ts.qtask = " << m.taskId << " AND tq.worker = " << m.workerId << " AND ts.state = " << (int)ZM_Base::stateType::running << ";"

              "UPDATE tblScheduler SET "
              "activeTask = " << m.schedrActiveTask << " "
              "WHERE id = " << sId << ";"

              "UPDATE tblWorker SET "
              "activeTask = " << m.workerActiveTask << " "
              "WHERE id = " << m.workerId << ";";
        break; 
      case ZM_Base::messType::taskCompleted: 
        ss << "UPDATE tblTaskTime tt SET "
              "stopTime = current_timestamp "
              "FROM tblTaskQueue tq "
              "WHERE tt.qtask = " << m.taskId << " AND tq.worker = " << m.workerId << " AND tt.stopTime IS NULL;"

              "UPDATE tblTaskResult tr SET "
              "result = '" << m.result << "' "
              "FROM tblTaskQueue tq "
              "WHERE tr.qtask = " << m.taskId << " AND tq.worker = " << m.workerId << " AND tr.result = '';"
              
              "UPDATE tblTaskState ts SET "
              "state = " << (int)ZM_Base::stateType::completed << " "
              "FROM tblTaskQueue tq "
              "WHERE ts.qtask = " << m.taskId << " AND tq.worker = " << m.workerId << " AND ts.state = " << (int)ZM_Base::stateType::running << ";"

              "UPDATE tblScheduler SET "
              "activeTask = " << m.schedrActiveTask << " "
              "WHERE id = " << sId << ";"

              "UPDATE tblWorker SET "
              "activeTask = " << m.workerActiveTask << " "
              "WHERE id = " << m.workerId << ";";
        break;  
      case ZM_Base::messType::taskStart: // when first launched 
        ss << "UPDATE tblTaskState SET "
              "state = " << (int)ZM_Base::stateType::running << " "
              "WHERE qtask = " << m.taskId << " AND state = " << (int)ZM_Base::stateType::start << ";"

              "UPDATE tblTaskQueue SET "
              "worker = " << m.workerId << " "
              "WHERE id = " << m.taskId << " AND worker IS NULL;"

              "UPDATE tblTaskTime SET "
              "startTime = current_timestamp "
              "WHERE qtask = " << m.taskId << " AND startTime IS NULL;";              
        break;         
      case ZM_Base::messType::taskRunning: // when continue
        ss << "UPDATE tblTaskState SET "
              "state = " << (int)ZM_Base::stateType::running << " "
              "WHERE qtask = " << m.taskId << " AND state = " << (int)ZM_Base::stateType::pause << ";";                        
        break;         
      case ZM_Base::messType::taskPause:
        ss << "UPDATE tblTaskState ts SET "
              "state = " << (int)ZM_Base::stateType::pause << " "
              "FROM tblTaskQueue tq "
              "WHERE ts.qtask = " << m.taskId << " AND tq.worker = " << m.workerId << " AND ts.state = " << (int)ZM_Base::stateType::running << ";"; 
        break;        
      case ZM_Base::messType::taskStop:        
        ss << "UPDATE tblTaskTime tt SET "
              "stopTime = current_timestamp "
              "FROM tblTaskQueue tq "
              "WHERE tt.qtask = " << m.taskId << " AND tq.worker = " << m.workerId << " AND tt.stopTime IS NULL;"
              
              "UPDATE tblTaskState ts SET "
              "state = " << (int)ZM_Base::stateType::stop << " "
              "FROM tblTaskQueue tq "
              "WHERE ts.qtask = " << m.taskId << " AND tq.worker = " << m.workerId  << " AND (ts.state BETWEEN 2 AND 3);"; // running, pause
        break;
      case ZM_Base::messType::justStartWorker:
        ss << "UPDATE tblTaskTime SET "
              "takeInWorkTime = NULL, "
              "startTime = NULL "
              "FROM tblTaskState ts, tblTaskQueue tq "
              "WHERE tq.id = ts.qtask AND tq.worker = " << m.workerId << " AND (ts.state BETWEEN 2 AND 3);"
              
              "WITH taskUpd AS ("
              "UPDATE tblTaskQueue tq SET "
              "schedr = NULL,"
              "worker = NULL " 
              "FROM tblTaskState ts "
              "WHERE tq.id = ts.qtask AND tq.worker = " << m.workerId << " AND (ts.state BETWEEN 2 AND 3) " // running, pause
              "RETURNING tq.id) "
                         
              "UPDATE tblTaskState SET "
              "state = " << (int)ZM_Base::stateType::ready << " "
              "WHERE qtask IN (SELECT id FROM taskUpd) " << " AND (state BETWEEN 2 AND 3);"; // running, pause
        break;
      case ZM_Base::messType::progress:
        ss << "UPDATE tblTaskState ts SET "
              "progress = " << m.progress << " "
              "FROM tblTaskQueue tq "
              "WHERE ts.qtask = " << m.taskId << " AND tq.worker = " << m.workerId <<  ";";
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
        ss << "UPDATE tblTaskTime SET "
              "takeInWorkTime = NULL, "
              "startTime = NULL "
              "FROM tblTaskState ts, tblTaskQueue tq "
              "WHERE tq.id = ts.qtask AND tq.worker = " << m.workerId << " AND (ts.state BETWEEN 2 AND 3);"
              
              "WITH taskUpd AS ("
              "UPDATE tblTaskQueue tq SET "
              "schedr = NULL,"
              "worker = NULL " 
              "FROM tblTaskState ts "
              "WHERE tq.id = ts.qtask AND tq.worker = " << m.workerId << " AND (ts.state BETWEEN 2 AND 3) " // running, pause
              "RETURNING tq.id) "
                         
              "UPDATE tblTaskState SET "
              "state = " << (int)ZM_Base::stateType::ready << " "
              "WHERE qtask IN (SELECT id FROM taskUpd) " << " AND (state BETWEEN 2 AND 3);" // running, pause

              "UPDATE tblWorker SET "
              "state = " << (int)ZM_Base::stateType::notResponding << " "
              "WHERE id = " << m.workerId << ";";
        break;
      case ZM_Base::messType::internError:
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
  auto res = PQexec(_pg, ss.str().c_str());
  if (PQresultStatus(res) != PGRES_COMMAND_OK){
    errorMess(string("sendAllMessFromSchedr error: ") + PQerrorMessage(_pg));
    PQclear(res);
    return false;
  }
  PQclear(res);
  return true;
}

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
        "TRUNCATE tblTaskState CASCADE;"
        "TRUNCATE tblTaskTime CASCADE;"
        "TRUNCATE tblTaskResult CASCADE;"
        "TRUNCATE tblPrevTask CASCADE;"
        "TRUNCATE tblTaskParam CASCADE;"
        "TRUNCATE tblTaskQueue CASCADE;";
  
  auto res = PQexec(_pg, ss.str().c_str());
  if (PQresultStatus(res) != PGRES_COMMAND_OK){
      errorMess(PQerrorMessage(_pg));
      PQclear(res);
      return false;
  }
  return true;
}
