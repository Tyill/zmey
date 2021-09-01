import os
import sqlite3
from contextlib import closing
from flask import(
  g, current_app
)
from . import zm_client as zm

m_instance_path = ""

def init(app):
  app.before_request(userDb)
  app.teardown_appcontext(closeUserDb)
  global m_instance_path
  m_instance_path = app.instance_path

def userDb():
  if ('db' not in g) and g.userName:
    try: 
      dbPath = current_app.instance_path + '/users/{}.db'.format(g.userName)
      isExist = os.path.exists(dbPath)
      g.db = sqlite3.connect(
        dbPath,
        detect_types=sqlite3.PARSE_DECLTYPES,
        check_same_thread = False
      )    
      g.db.row_factory = sqlite3.Row
      if not isExist:
        initUserDb(g.db)
    except Exception as err:
      print('Local db failed: %s' % str(err))

def initUserDb(db):
  with closing(db.cursor()) as cr:
    cr.execute('PRAGMA journal_mode=wal')
  
    cr.execute(
      "CREATE TABLE IF NOT EXISTS tblState ( \
        id   INTEGER PRIMARY KEY AUTOINCREMENT, \
        kind TEXT NOT NULL CHECK (kind <> ''));"
    )    
    cr.execute(f"INSERT INTO tblState(id, kind) VALUES ('{zm.StateType.UNDEFINED.value}', 'undefined');")
    cr.execute(f"INSERT INTO tblState(id, kind) VALUES ('{zm.StateType.READY.value}', 'ready');")
    cr.execute(f"INSERT INTO tblState(id, kind) VALUES ('{zm.StateType.START.value}', 'start');")
    cr.execute(f"INSERT INTO tblState(id, kind) VALUES ('{zm.StateType.RUNNING.value}', 'running');")
    cr.execute(f"INSERT INTO tblState(id, kind) VALUES ('{zm.StateType.PAUSE.value}', 'pause');")
    cr.execute(f"INSERT INTO tblState(id, kind) VALUES ('{zm.StateType.STOP.value}', 'stop');")
    cr.execute(f"INSERT INTO tblState(id, kind) VALUES ('{zm.StateType.COMPLETED.value}', 'completed');")
    cr.execute(f"INSERT INTO tblState(id, kind) VALUES ('{zm.StateType.ERROR.value}', 'error');")
    cr.execute(f"INSERT INTO tblState(id, kind) VALUES ('{zm.StateType.CANCEL.value}', 'cancel');")
    cr.execute(f"INSERT INTO tblState(id, kind) VALUES ('{zm.StateType.NOT_RESPONDING.value}', 'notResponding');")
    db.commit()
    
    cr.execute(
      "CREATE TABLE IF NOT EXISTS tblPipeline ( \
        id            INTEGER PRIMARY KEY AUTOINCREMENT, \
        isDelete      INT NOT NULL DEFAULT 0 CHECK (isDelete BETWEEN 0 AND 1), \
        name          TEXT NOT NULL, \
        description   TEXT NOT NULL, \
        setts TEXT NOT NULL);"
    )
    cr.execute(
      "CREATE TABLE IF NOT EXISTS tblTaskTemplate ( \
        id INTEGER PRIMARY KEY AUTOINCREMENT, \
        averDurationSec INT NOT NULL CHECK (averDurationSec > 0), \
        maxDurationSec  INT NOT NULL CHECK (maxDurationSec > 0), \
        isDelete        INT NOT NULL DEFAULT 0 CHECK (isDelete BETWEEN 0 AND 1),  \
        script          TEXT NOT NULL CHECK (script <> ''), \
        name            TEXT NOT NULL CHECK (name <> ''), \
        description     TEXT NOT NULL);"
    )
    cr.execute(
      "CREATE TABLE IF NOT EXISTS tblPipelineTask( \
        id INTEGER PRIMARY KEY AUTOINCREMENT, \
        pplId         INT NOT NULL REFERENCES tblPipeline, \
        ttId          INT NOT NULL REFERENCES tblTaskTemplate, \
        isEnabled     INT NOT NULL DEFAULT 1 CHECK (isEnabled BETWEEN 0 AND 1), \
        isDelete      INT NOT NULL DEFAULT 0 CHECK (isDelete BETWEEN 0 AND 1), \
        params        TEXT NOT NULL, \
        nextTasksId   TEXT NOT NULL, \
        prevTasksId   TEXT NOT NULL, \
        isStartNext   TEXT NOT NULL, \
        isSendResultToNext TEXT NOT NULL, \
        name          TEXT NOT NULL CHECK (name <> ''), \
        description   TEXT NOT NULL, \
        setts TEXT NOT NULL);"   
    )
    cr.execute(
      "CREATE INDEX inxPipelineTaskPplId ON tblPipelineTask(PplId);"
    )
  
    cr.execute(
      "CREATE TABLE IF NOT EXISTS tblEvent( \
        id INTEGER PRIMARY KEY AUTOINCREMENT, \
        isEnabled     INT NOT NULL DEFAULT 1 CHECK (isEnabled BETWEEN 0 AND 1), \
        isDelete      INT NOT NULL DEFAULT 0 CHECK (isDelete BETWEEN 0 AND 1), \
        nextTasksId   TEXT NOT NULL, \
        name          TEXT NOT NULL CHECK (name <> ''), \
        description   TEXT NOT NULL, \
        startTime     TEXT NOT NULL DEFAULT '');"
    )
    cr.execute(
      'CREATE TABLE IF NOT EXISTS tblTask( \
        id INTEGER PRIMARY KEY AUTOINCREMENT, \
        pplTaskId       INT NOT NULL REFERENCES tblPipelineTask, \
        starterPplTaskId INT REFERENCES tblPipelineTask, \
        starterEventId   INT REFERENCES tblEvent, \
        ttlId           INT NOT NULL REFERENCES tblTaskTemplate, \
        state           INT NOT NULL REFERENCES tblState,  \
        progress        INT NOT NULL DEFAULT 0,  \
        script          TEXT NOT NULL, \
        params          TEXT NOT NULL, \
        result          TEXT NOT NULL DEFAULT "", \
        createTime      TEXT NOT NULL DEFAULT "", \
        takeInWorkTime  TEXT NOT NULL DEFAULT "", \
        startTime       TEXT NOT NULL DEFAULT "", \
        stopTime        TEXT NOT NULL DEFAULT "");'  
    )
    cr.execute(
      "CREATE INDEX inxTaskPplTaskId ON tblTask(pplTaskId);"
    )    
    
def closeUserDb(e = None):
  db = g.pop('db', None)
  if db is not None:
    db.close()

def createDb(userName : str):
  dbPath = m_instance_path + '/users/{}.db'.format(userName)
  if os.path.exists(dbPath):
    db = sqlite3.connect(
      dbPath,
      detect_types=sqlite3.PARSE_DECLTYPES,
      check_same_thread = False
    )    
    db.row_factory = sqlite3.Row
    return db
  return None

def closeDb(db):
  if db is not None:
    db.close()