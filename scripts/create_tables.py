#!/usr/bin/python3
# -*- coding: utf-8 -*-


import psycopg2

def createTables():
  with psycopg2.connect(dbname='passdb', user='postgres', password='postgres', host='localhost') as pg:
    csr = pg.cursor()
    csr.execute(

      "SELECT pg_catalog.set_config('search_path', 'public', false)"

      "CREATE TABLE IF NOT EXISTS tblState("
      "id           SERIAL PRIMARY KEY,"
      "kind         TEXT NOT NULL CHECK (kind <> ''));"
      
      "INSERT INTO tblState VALUES"
        "(0, 'undefined'),"
        "(1, 'ready'),"
        "(2, 'start'),"
        "(3, 'running'),"
        "(4, 'pause'),"
        "(5, 'stop'),"
        "(6, 'completed'),"
        "(7, 'error'),"
        "(8, 'cancel'),"
        "(9, 'notResponding') ON CONFLICT (id) DO NOTHING;"
  
      "CREATE TABLE IF NOT EXISTS tblScheduler("
        "id           SERIAL PRIMARY KEY,"
        "state        INT NOT NULL REFERENCES tblState DEFAULT 5,"
        "capacityTask INT NOT NULL DEFAULT 10000 CHECK (capacityTask > 0),"
        "activeTask   INT NOT NULL DEFAULT 0 CHECK (activeTask >= 0),"
        "isDelete     INT NOT NULL DEFAULT 0 CHECK (isDelete BETWEEN 0 AND 1),"
        "startTime    TIMESTAMP NOT NULL DEFAULT current_timestamp,"
        "stopTime     TIMESTAMP NOT NULL DEFAULT current_timestamp,"
        "pingTime     TIMESTAMP NOT NULL DEFAULT current_timestamp,"
        "connPnt      TEXT NOT NULL);"
     
      "CREATE TABLE IF NOT EXISTS tblWorker("
        "id           SERIAL PRIMARY KEY,"
        "schedr       INT NOT NULL REFERENCES tblScheduler,"
        "state        INT NOT NULL REFERENCES tblState DEFAULT 5,"
        "capacityTask INT NOT NULL DEFAULT 10 CHECK (capacityTask > 0),"
        "activeTask   INT NOT NULL DEFAULT 0 CHECK (activeTask >= 0),"
        "load         INT NOT NULL DEFAULT 0 CHECK (load BETWEEN 0 AND 100),"
        "isDelete     INT NOT NULL DEFAULT 0 CHECK (isDelete BETWEEN 0 AND 1),"
        "startTime    TIMESTAMP NOT NULL DEFAULT current_timestamp,"
        "stopTime     TIMESTAMP NOT NULL DEFAULT current_timestamp,"
        "pingTime     TIMESTAMP NOT NULL DEFAULT current_timestamp,"
        "connPnt      TEXT NOT NULL);"

      "CREATE TABLE IF NOT EXISTS tblInternError("
        "id           SERIAL PRIMARY KEY,"
        "schedr       INT REFERENCES tblScheduler,"
        "worker       INT REFERENCES tblWorker,"
        "createTime   TIMESTAMP NOT NULL DEFAULT current_timestamp,"
        "message      TEXT NOT NULL);"
  
      "CREATE TABLE IF NOT EXISTS tblTaskQueue("
        "id           SERIAL PRIMARY KEY,"
        "schedr       INT REFERENCES tblScheduler,"
        "worker       INT REFERENCES tblWorker);"

      "CREATE TABLE IF NOT EXISTS tblTaskState("
        "qtask        INT PRIMARY KEY REFERENCES tblTaskQueue,"        
        "state        INT NOT NULL REFERENCES tblState,"
        "progress     INT NOT NULL DEFAULT 0 CHECK (progress BETWEEN 0 AND 100));"

      "CREATE TABLE IF NOT EXISTS tblTaskTime("
        "qtask          INT PRIMARY KEY REFERENCES tblTaskQueue,"        
        "createTime     TIMESTAMP NOT NULL DEFAULT current_timestamp,"
        "takeInWorkTime TIMESTAMP CHECK (takeInWorkTime >= createTime),"
        "startTime      TIMESTAMP,"
        "stopTime       TIMESTAMP);"
  
      "CREATE TABLE IF NOT EXISTS tblTaskParam("
        "qtask           INT PRIMARY KEY REFERENCES tblTaskQueue,"
        "averDurationSec INT NOT NULL CHECK (averDurationSec >= 0),"
        "maxDurationSec  INT NOT NULL CHECK (maxDurationSec >= 0),"
        "schedrPreset    INT REFERENCES tblScheduler,"
        "workerPreset    INT REFERENCES tblWorker," 
        "params          TEXT NOT NULL,"
        "scriptPath      TEXT NOT NULL,"
        "resultPath      TEXT NOT NULL);"

###############################################################################
### INDEXES

      "CREATE INDEX IF NOT EXISTS inxTSState ON tblTaskState(state);"
      "CREATE INDEX IF NOT EXISTS inxTQSchedr ON tblTaskQueue(schedr);"
      "CREATE INDEX IF NOT EXISTS inxTQWorker ON tblTaskQueue(worker);"
      
###############################################################################
### FUNCTIONS
     
      "CREATE OR REPLACE FUNCTION "
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
        "" << int(base::StateType::READY) << ");"

        "  RETURN qId;"
        "END;"
        "$$ LANGUAGE plpgsql;"
  
      "CREATE OR REPLACE FUNCTION "
        "funcStartTaskNotify() "
        "RETURNS trigger AS $$ "
        "BEGIN "
        " NOTIFY " << m_impl->NOTIFY_NAME_NEW_TASK << ";"
        " RETURN NEW;"
        "END; "
        "$$ LANGUAGE plpgsql;"
  
      "CREATE OR REPLACE FUNCTION "
        "funcChangeTaskNotify() "
        "RETURNS trigger AS $$ "
        "BEGIN "
        " NOTIFY " << m_impl->NOTIFY_NAME_CHANGE_TASK << ";"
        " RETURN NEW;"
        "END; "
        "$$ LANGUAGE plpgsql;"
  
      "CREATE OR REPLACE FUNCTION "
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
        "    WHERE ts.state = " << int(base::StateType::READY) << ""
        "      AND tq.schedr IS NULL "
        "      AND (tp.schedrPreset IS NULL OR tp.schedrPreset = sId) ORDER BY tq.id"
        "    LIMIT maxTaskCnt "
        "    FOR UPDATE OF tq SKIP LOCKED"
        "  LOOP"
        "    UPDATE tblTaskQueue SET"
        "      schedr = sId"
        "    WHERE id = qid;"
        
        "    UPDATE tblTaskState SET"
        "      state = " << int(base::StateType::START) << ""
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
        "$$ LANGUAGE plpgsql;"
  
###############################################################################
### TRIGGERS

      "DROP TRIGGER IF EXISTS trgNewTask ON tblTaskQueue; "
        "CREATE TRIGGER trgNewTask "
        "AFTER INSERT ON tblTaskQueue "
        "FOR EACH STATEMENT "
        "EXECUTE PROCEDURE funcStartTaskNotify();"
  
      "DROP TRIGGER IF EXISTS trgTaskChange ON tblTaskState; "
        "CREATE TRIGGER trgTaskChange "
        "AFTER UPDATE ON tblTaskState "
        "FOR EACH STATEMENT "
        "EXECUTE PROCEDURE funcChangeTaskNotify();"
    )
    csr.close()

if __name__ == "__main__":
  createTables()
