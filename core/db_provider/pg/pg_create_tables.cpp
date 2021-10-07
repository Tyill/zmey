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

#include "pg_impl.h"

using namespace std;

namespace ZM_DB{

DbProvider::DbProvider(const ZM_DB::ConnectCng& cng) :
  m_impl(new DbProvider::Impl),
  m_connCng(cng){ 

  m_impl->m_db = (PGconn*)PQconnectdb(cng.connectStr.c_str());
  if (PQstatus(_pg) != CONNECTION_OK){
    errorMess(PQerrorMessage(_pg));
    return;
  }
}
DbProvider::~DbProvider(){  
  if (m_impl->m_thrEndTask.joinable()){
    m_impl->m_fClose = true; 
    m_impl->m_thrEndTask.join();
  }
  if (_pg){
    PQfinish(_pg);
  }
  delete m_impl;
}
bool DbProvider::createTables(){
  lock_guard<mutex> lk(m_impl->m_mtx);
  #define QUERY(req, sts){              \
    PGres pgr(PQexec(_pg, req));        \
    if (PQresultStatus(pgr.res) != sts){  \
      errorMess(string("createTables: ") + PQerrorMessage(_pg)); \
      return false;                     \
    }                                   \
  }
  QUERY("SELECT pg_catalog.set_config('search_path', 'public', false)", PGRES_TUPLES_OK);

  stringstream ss;
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
        "(" << (int)ZM_Base::StateType::ERRORT << ", 'error'),"
        "(" << (int)ZM_Base::StateType::CANCEL << ", 'cancel'),"
        "(" << (int)ZM_Base::StateType::NOT_RESPONDING << ", 'notResponding') ON CONFLICT (id) DO NOTHING;";
  QUERY(ss.str().c_str(), PGRES_COMMAND_OK);

  ss.str("");
  ss << "CREATE TABLE IF NOT EXISTS tblScheduler("
        "id           SERIAL PRIMARY KEY,"
        "connPnt      INT NOT NULL REFERENCES tblConnectPnt,"
        "state        INT NOT NULL REFERENCES tblState DEFAULT " << (int)ZM_Base::StateType::STOP << ","
        "capacityTask INT NOT NULL DEFAULT 10000 CHECK (capacityTask > 0),"
        "activeTask   INT NOT NULL DEFAULT 0 CHECK (activeTask >= 0),"
        "isDelete     INT NOT NULL DEFAULT 0 CHECK (isDelete BETWEEN 0 AND 1),"
        "startTime    TIMESTAMP NOT NULL DEFAULT current_timestamp,"
        "stopTime     TIMESTAMP NOT NULL DEFAULT current_timestamp,"
        "pingTime     TIMESTAMP NOT NULL DEFAULT current_timestamp,"
        "internalData TEXT NOT NULL DEFAULT '',"
        "name        TEXT NOT NULL DEFAULT '',"
        "description  TEXT NOT NULL DEFAULT '');";
  QUERY(ss.str().c_str(), PGRES_COMMAND_OK);

  ss.str("");
  ss << "CREATE TABLE IF NOT EXISTS tblWorker("
        "id           SERIAL PRIMARY KEY,"
        "connPnt      INT NOT NULL REFERENCES tblConnectPnt,"
        "schedr       INT NOT NULL REFERENCES tblScheduler,"
        "state        INT NOT NULL REFERENCES tblState DEFAULT " << (int)ZM_Base::StateType::STOP << ","
        "capacityTask INT NOT NULL DEFAULT 10 CHECK (capacityTask > 0),"
        "activeTask   INT NOT NULL DEFAULT 0 CHECK (activeTask >= 0),"
        "load         INT NOT NULL DEFAULT 0 CHECK (load BETWEEN 0 AND 100),"
        "isDelete     INT NOT NULL DEFAULT 0 CHECK (isDelete BETWEEN 0 AND 1),"
        "startTime    TIMESTAMP NOT NULL DEFAULT current_timestamp,"
        "stopTime     TIMESTAMP NOT NULL DEFAULT current_timestamp,"
        "pingTime     TIMESTAMP NOT NULL DEFAULT current_timestamp,"
        "name         TEXT NOT NULL DEFAULT '',"
        "description  TEXT NOT NULL DEFAULT '');";
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
  ss << "CREATE TABLE IF NOT EXISTS tblTaskTemplate("
        "id           SERIAL PRIMARY KEY,"
        "usr          INT NOT NULL,"
        "name         TEXT NOT NULL CHECK (name <> ''),"
        "description  TEXT NOT NULL,"
        "script       TEXT NOT NULL CHECK (script <> ''),"
        "averDurationSec INT NOT NULL CHECK (averDurationSec > 0),"
        "maxDurationSec  INT NOT NULL CHECK (maxDurationSec > 0),"
        "schedrPreset INT REFERENCES tblScheduler,"
        "workerPreset INT REFERENCES tblWorker,"
        "isDelete     INT NOT NULL DEFAULT 0 CHECK (isDelete BETWEEN 0 AND 1));";
  QUERY(ss.str().c_str(), PGRES_COMMAND_OK);

  ss.str("");
  ss << "CREATE TABLE IF NOT EXISTS tblTaskQueue("
        "id           SERIAL PRIMARY KEY,"
        "taskTempl    INT NOT NULL REFERENCES tblTaskTemplate,"      
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
        "startTime      TIMESTAMP,"
        "stopTime       TIMESTAMP);";
  QUERY(ss.str().c_str(), PGRES_COMMAND_OK);

  ss.str("");
  ss << "CREATE TABLE IF NOT EXISTS tblTaskParam("
        "qtask        INT PRIMARY KEY REFERENCES tblTaskQueue,"        
        "params       TEXT NOT NULL);";
  QUERY(ss.str().c_str(), PGRES_COMMAND_OK);

  ss.str("");
  ss << "CREATE TABLE IF NOT EXISTS tblTaskResult("
        "qtask        INT PRIMARY KEY REFERENCES tblTaskQueue,"
        "result       TEXT NOT NULL);";
  QUERY(ss.str().c_str(), PGRES_COMMAND_OK);
    
  ///////////////////////////////////////////////////////////////////////////
  /// INDEXES
  ss.str(""); 
  ss << "CREATE INDEX IF NOT EXISTS inxTSState ON tblTaskState(state);";
  ss << "CREATE INDEX IF NOT EXISTS inxTQSchedr ON tblTaskQueue(schedr);";
  ss << "CREATE INDEX IF NOT EXISTS inxTQWorker ON tblTaskQueue(worker);";
  QUERY(ss.str().c_str(), PGRES_COMMAND_OK);
      
  ///////////////////////////////////////////////////////////////////////////
  /// FUNCTIONS
  ss.str("");
  ss << "CREATE OR REPLACE FUNCTION "
        "funcStartTask(ttlId int, tParams TEXT) "
        "RETURNS int AS $$ "
        "DECLARE "
        "  qId int := 0;"
        "BEGIN"
        
        "  INSERT INTO tblTaskQueue(taskTempl) "
        "  VALUES(ttlId) RETURNING id INTO qId;"
        
        "  INSERT INTO tblTaskTime (qtask) VALUES("
        "    qId);"

        "  INSERT INTO tblTaskParam (qtask, params) VALUES("
        "    qId,"
        "    tParams);"

        "  INSERT INTO tblTaskResult (qtask, result) VALUES("
        "    qId,"
        "    '');"
                     
        "  INSERT INTO tblTaskState (qtask, state) VALUES("
        "    qId,"
        "" << int(ZM_Base::StateType::READY) << ");"

        "  RETURN qId;"
        "END;"
        "$$ LANGUAGE plpgsql;";
  QUERY(ss.str().c_str(), PGRES_COMMAND_OK);

  ss.str("");
  ss << "CREATE OR REPLACE FUNCTION "
        "funcStartTaskNotify() "
        "RETURNS trigger AS $$ "
        "BEGIN "
        " NOTIFY " << m_impl->NOTIFY_NAME_NEW_TASK << ";"
        " RETURN NEW;"
        "END; "
        "$$ LANGUAGE plpgsql;";
  QUERY(ss.str().c_str(), PGRES_COMMAND_OK);

  ss.str("");
  ss << "CREATE OR REPLACE FUNCTION "
        "funcChangeTaskNotify() "
        "RETURNS trigger AS $$ "
        "BEGIN "
        " NOTIFY " << m_impl->NOTIFY_NAME_CHANGE_TASK << ";"
        " RETURN NEW;"
        "END; "
        "$$ LANGUAGE plpgsql;";
  QUERY(ss.str().c_str(), PGRES_COMMAND_OK);
  
  ss.str("");
  ss << "CREATE OR REPLACE FUNCTION "
        "funcNewTasksForSchedr(sId int, maxTaskCnt int) "
        "RETURNS TABLE("
        "  qid int,"
        "  averDurSec int,"
        "  maxDurSec int,"
        "  workerPreset int,"
        "  script text,"
        "  params text) AS $$ "
        "DECLARE "
        "  t int;"
        "BEGIN"
        "  FOR qid, averDurSec, maxDurSec, workerPreset, script, params IN"
        "    SELECT tq.id, tt.averDurationSec, tt.maxDurationSec, COALESCE(tt.workerPreset, 0),"
        "           tt.script, tp.params"
        "    FROM tblTaskTemplate tt "
        "    JOIN tblTaskQueue tq ON tq.taskTempl = tt.id "
        "    JOIN tblTaskState ts ON ts.qtask = tq.id "
        "    JOIN tblTaskParam tp ON tp.qtask = tq.id "
        "    WHERE ts.state = " << int(ZM_Base::StateType::READY) << ""
        "      AND tq.schedr IS NULL "
        "      AND (tt.schedrPreset IS NULL OR tt.schedrPreset = sId) ORDER BY tq.id"
        "    LIMIT maxTaskCnt "
        "    FOR UPDATE OF tq SKIP LOCKED"
        "  LOOP"
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

  ///////////////////////////////////////////////////////////////////////////
  /// TRIGGERS
  ss.str(""); 
  ss << "DROP TRIGGER IF EXISTS trgNewTask ON tblTaskQueue; "
        "CREATE TRIGGER trgNewTask "
        "AFTER INSERT ON tblTaskQueue "
        "FOR EACH STATEMENT "
        "EXECUTE PROCEDURE funcStartTaskNotify();";
  QUERY(ss.str().c_str(), PGRES_COMMAND_OK);

  ss.str(""); 
  ss << "DROP TRIGGER IF EXISTS trgTaskChange ON tblTaskState; "
        "CREATE TRIGGER trgTaskChange "
        "AFTER UPDATE ON tblTaskState "
        "FOR EACH STATEMENT "
        "EXECUTE PROCEDURE funcChangeTaskNotify();";
  QUERY(ss.str().c_str(), PGRES_COMMAND_OK);

  return true;
#undef QUERY
}
}