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

namespace DB{

DbProvider::DbProvider(const DB::ConnectCng& cng) :
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
  ss << "CREATE TABLE IF NOT EXISTS tblState("
        "id           SERIAL PRIMARY KEY,"
        "kind         TEXT NOT NULL CHECK (kind <> ''));";
  QUERY(ss.str().c_str(), PGRES_COMMAND_OK);
    
  ss.str("");
  ss << "INSERT INTO tblState VALUES"
        "(" << (int)Base::StateType::UNDEFINED << ", 'undefined'),"
        "(" << (int)Base::StateType::READY << ", 'ready'),"
        "(" << (int)Base::StateType::START << ", 'start'),"
        "(" << (int)Base::StateType::RUNNING << ", 'running'),"
        "(" << (int)Base::StateType::PAUSE << ", 'pause'),"
        "(" << (int)Base::StateType::STOP << ", 'stop'),"
        "(" << (int)Base::StateType::COMPLETED << ", 'completed'),"
        "(" << (int)Base::StateType::ERRORT << ", 'error'),"
        "(" << (int)Base::StateType::CANCEL << ", 'cancel'),"
        "(" << (int)Base::StateType::NOT_RESPONDING << ", 'notResponding') ON CONFLICT (id) DO NOTHING;";
  QUERY(ss.str().c_str(), PGRES_COMMAND_OK);

  ss.str("");
  ss << "CREATE TABLE IF NOT EXISTS tblScheduler("
        "id           SERIAL PRIMARY KEY,"
        "state        INT NOT NULL REFERENCES tblState DEFAULT " << (int)Base::StateType::STOP << ","
        "capacityTask INT NOT NULL DEFAULT 10000 CHECK (capacityTask > 0),"
        "activeTask   INT NOT NULL DEFAULT 0 CHECK (activeTask >= 0),"
        "isDelete     INT NOT NULL DEFAULT 0 CHECK (isDelete BETWEEN 0 AND 1),"
        "startTime    TIMESTAMP NOT NULL DEFAULT current_timestamp,"
        "stopTime     TIMESTAMP NOT NULL DEFAULT current_timestamp,"
        "pingTime     TIMESTAMP NOT NULL DEFAULT current_timestamp,"
        "connPnt      TEXT NOT NULL,"
        "internalData TEXT NOT NULL DEFAULT '',"
        "name        TEXT NOT NULL DEFAULT '',"
        "description  TEXT NOT NULL DEFAULT '');";
  QUERY(ss.str().c_str(), PGRES_COMMAND_OK);

  ss.str("");
  ss << "CREATE TABLE IF NOT EXISTS tblWorker("
        "id           SERIAL PRIMARY KEY,"
        "schedr       INT NOT NULL REFERENCES tblScheduler,"
        "state        INT NOT NULL REFERENCES tblState DEFAULT " << (int)Base::StateType::STOP << ","
        "capacityTask INT NOT NULL DEFAULT 10 CHECK (capacityTask > 0),"
        "activeTask   INT NOT NULL DEFAULT 0 CHECK (activeTask >= 0),"
        "load         INT NOT NULL DEFAULT 0 CHECK (load BETWEEN 0 AND 100),"
        "isDelete     INT NOT NULL DEFAULT 0 CHECK (isDelete BETWEEN 0 AND 1),"
        "startTime    TIMESTAMP NOT NULL DEFAULT current_timestamp,"
        "stopTime     TIMESTAMP NOT NULL DEFAULT current_timestamp,"
        "pingTime     TIMESTAMP NOT NULL DEFAULT current_timestamp,"
        "connPnt      TEXT NOT NULL,"
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
  ss << "CREATE TABLE IF NOT EXISTS tblTaskQueue("
        "id           SERIAL PRIMARY KEY,"
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
        "qtask           INT PRIMARY KEY REFERENCES tblTaskQueue,"
        "averDurationSec INT NOT NULL CHECK (averDurationSec >= 0),"
        "maxDurationSec  INT NOT NULL CHECK (maxDurationSec >= 0),"
        "schedrPreset    INT REFERENCES tblScheduler,"
        "workerPreset    INT REFERENCES tblWorker," 
        "params          TEXT NOT NULL,"
        "scriptPath      TEXT NOT NULL,"
        "resultPath      TEXT NOT NULL);";
  QUERY(ss.str().c_str(), PGRES_COMMAND_OK);
     
  ///////////////////////////////////////////////////////////////////////////
  /// INDEXES
  ss.str(""); 
  ss << "CREATE INDEX IF NOT EXISTS inxTSState ON tblTaskState(state);";
//   ss << "CREATE INDEX IF NOT EXISTS inxTQSchedr ON tblTaskQueue(schedr);";  not needed yet
//   ss << "CREATE INDEX IF NOT EXISTS inxTQWorker ON tblTaskQueue(worker);";
  QUERY(ss.str().c_str(), PGRES_COMMAND_OK);
      
  ///////////////////////////////////////////////////////////////////////////
  /// FUNCTIONS
  ss.str("");
  ss << "CREATE OR REPLACE FUNCTION "
        "funcStartTask(averDurationSec int, maxDurationSec int, schedrPreset int, workerPreset int, params TEXT, scriptPath TEXT, resultPath TEXT) "
        "RETURNS int AS $$ "
        "DECLARE "
        "  qId int := 0;"
        "BEGIN"
        
        "  INSERT INTO tblTaskQueue "
        "  DEFAULT VALUES "
        "  RETURNING id INTO qId;"
        
        "  INSERT INTO tblTaskTime (qtask) VALUES("
        "    qId);"

        "  INSERT INTO tblTaskParam (qtask, averDurationSec, maxDurationSec, schedrPreset, workerPreset, params, scriptPath, resultPath) VALUES("
        "    qId,"
        "    averDurationSec,"
        "    maxDurationSec,"
        "    NULLIF(schedrPreset, 0),"
        "    NULLIF(workerPreset, 0),"
        "    params,"
        "    scriptPath,"
        "    resultPath);"
                            
        "  INSERT INTO tblTaskState (qtask, state) VALUES("
        "    qId,"
        "" << int(Base::StateType::READY) << ");"

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
        "  workerPreset int,"
        "  averDurSec int,"
        "  maxDurSec int,"        
        "  params text,"
        "  scriptPath text,"        
        "  resultPath text) AS $$ "
        "DECLARE "
        "  t int;"
        "BEGIN"
        "  FOR qid, workerPreset, averDurSec, maxDurSec, params, scriptPath, resultPath IN"
        "    SELECT tq.id, COALESCE(tp.workerPreset, 0), tp.averDurationSec, tp.maxDurationSec,"
        "           tp.params, tp.scriptPath, tp.resultPath"
        "    FROM tblTaskQueue tq "
        "    JOIN tblTaskParam tp ON tp.qtask = tq.id "
        "    JOIN tblTaskState ts ON ts.qtask = tq.id "
        "    WHERE ts.state = " << int(Base::StateType::READY) << ""
        "      AND tq.schedr IS NULL "
        "      AND (tp.schedrPreset IS NULL OR tp.schedrPreset = sId) ORDER BY tq.id"
        "    LIMIT maxTaskCnt "
        "    FOR UPDATE OF tq SKIP LOCKED"
        "  LOOP"
        "    UPDATE tblTaskQueue SET"
        "      schedr = sId"
        "    WHERE id = qid;"
        
        "    UPDATE tblTaskState SET"
        "      state = " << int(Base::StateType::START) << ""
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