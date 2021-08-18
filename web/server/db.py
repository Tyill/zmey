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
  if ('db' not in g) and ('userName' in g):
    try: 
      dbPath = current_app.instance_path + '/users/{}.db'.format(g.userName)
      isExist = os.path.exists(dbPath)
      g.db = sqlite3.connect(
        dbPath,
        detect_types=sqlite3.PARSE_DECLTYPES
      )    
      with closing(g.db.cursor()) as cr:
        cr.execute('PRAGMA journal_mode=wal')
      g.db.row_factory = sqlite3.Row
      if not isExist:
        initUserDb(g.db)
    except Exception as err:
      print('Local db failed: %s' % str(err))

def initUserDb(db):
  with closing(db.cursor()) as cr:
    cr.execute(
      "CREATE TABLE IF NOT EXISTS tblPipeline ( \
        id          INTEGER PRIMARY KEY AUTOINCREMENT, \
        isDelete    INTEGER NOT NULL DEFAULT 0 CHECK (isDelete BETWEEN 0 AND 1), \
        name        TEXT NOT NULL,\
        description TEXT NOT NULL);"
    )
    cr.execute(
      "CREATE TABLE IF NOT EXISTS tblTaskTemplate ( \
        id INTEGER PRIMARY KEY AUTOINCREMENT, \
        averDurationSec INTEGER NOT NULL CHECK (averDurationSec > 0), \
        maxDurationSec  INTEGER NOT NULL CHECK (maxDurationSec > 0), \
        isDelete        INTEGER NOT NULL DEFAULT 0 CHECK (isDelete BETWEEN 0 AND 1),  \
        script          TEXT NOT NULL CHECK (script <> ''), \
        name            TEXT NOT NULL CHECK (name <> ''), \
        description     TEXT NOT NULL);"
    )
    cr.execute(
      "CREATE TABLE IF NOT EXISTS tblPipelineTask( \
        id INTEGER PRIMARY KEY AUTOINCREMENT, \
        pipeline    INT NOT NULL REFERENCES tblPipeline, \
        taskTempl   INT NOT NULL REFERENCES tblTaskTemplate, \
        isDelete    INT NOT NULL DEFAULT 0 CHECK (isDelete BETWEEN 0 AND 1), \
        params      TEXT NOT NULL, \
        nextTasks   TEXT NOT NULL, \
        nextEvents  TEXT NOT NULL, \
        name        TEXT NOT NULL CHECK (name <> ''), \
        description TEXT NOT NULL);"   
    )
    cr.execute(
      "CREATE TABLE IF NOT EXISTS tblEvent( \
        id INTEGER PRIMARY KEY AUTOINCREMENT, \
        isDelete    INT NOT NULL DEFAULT 0 CHECK (isDelete BETWEEN 0 AND 1), \
        params      TEXT NOT NULL, \
        nextTasks   TEXT NOT NULL, \
        nextEvents  TEXT NOT NULL, \
        name        TEXT NOT NULL CHECK (name <> ''), \
        description TEXT NOT NULL);"  
    )
    
def closeUserDb(e = None):
  db = g.pop('db', None)
  if db is not None:
      db.close()