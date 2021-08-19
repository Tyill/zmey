import os
import sqlite3
from contextlib import closing
from flask import(
  g, current_app
)

def init(app):
  app.before_request(userDb)
  app.teardown_appcontext(closeUserDb)

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
      "CREATE TABLE IF NOT EXISTS tblPipeline ( \
        id          INTEGER PRIMARY KEY AUTOINCREMENT, \
        isVisible   INT NOT NULL DEFAULT 0 CHECK (isVisible BETWEEN 0 AND 1), \
        isDelete    INT NOT NULL DEFAULT 0 CHECK (isDelete BETWEEN 0 AND 1), \
        name        TEXT NOT NULL,\
        description TEXT NOT NULL);"
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
        pplId       INT NOT NULL REFERENCES tblPipeline, \
        ttId        INT NOT NULL REFERENCES tblTaskTemplate, \
        isEnabled   INT NOT NULL DEFAULT 1 CHECK (isVisible BETWEEN 0 AND 1), \
        isVisible   INT NOT NULL DEFAULT 0 CHECK (isVisible BETWEEN 0 AND 1), \
        isDelete    INT NOT NULL DEFAULT 0 CHECK (isDelete BETWEEN 0 AND 1), \
        positionX   INT NOT NULL DEFAULT 0, \
        positionY   INT NOT NULL DEFAULT 0, \
        params      TEXT NOT NULL, \
        nextTasksId   TEXT NOT NULL, \
        nextEventsId  TEXT NOT NULL, \
        name        TEXT NOT NULL CHECK (name <> ''), \
        description TEXT NOT NULL);"   
    )
    cr.execute(
      "CREATE TABLE IF NOT EXISTS tblEvent( \
        id INTEGER PRIMARY KEY AUTOINCREMENT, \
        isEnabled   INT NOT NULL DEFAULT 1 CHECK (isVisible BETWEEN 0 AND 1), \
        isVisible   INT NOT NULL DEFAULT 0 CHECK (isVisible BETWEEN 0 AND 1), \
        isDelete    INT NOT NULL DEFAULT 0 CHECK (isDelete BETWEEN 0 AND 1), \
        positionX   INT NOT NULL DEFAULT 0, \
        positionY   INT NOT NULL DEFAULT 0, \
        params      TEXT NOT NULL, \
        nextTasksId   TEXT NOT NULL, \
        nextEventsId  TEXT NOT NULL, \
        name        TEXT NOT NULL CHECK (name <> ''), \
        description TEXT NOT NULL);"  
    )
    
def closeUserDb(e = None):
  db = g.pop('db', None)
  if db is not None:
      db.close()