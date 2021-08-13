import os
from flask import(
  g
)

def init(app):
  app.teardown_appcontext(closeUserDb)

def userDb(uname : str):
  if 'db' not in g:
    dbPath = current_app.instance_path + '/users/{}.sqlite'.format(uname)
    isExist = os.path(dbPath).isExist()
    g.db = sqlite3.connect(
      dbPath,
      detect_types=sqlite3.PARSE_DECLTYPES
    )    
    g.db.cursor().execute('PRAGMA journal_mode=wal')
    g.db.row_factory = sqlite3.Row
    if not isExist:
      initUserDb(uname)
  return g.db

def initUserDb(db):
  cr = db.cursor() 
  cr.execute(
    "CREATE TABLE tblMessType (             \
      id INTEGER PRIMARY KEY AUTOINCREMENT, \
      name       TEXT NOT NULL);"
  ) 
  cr.execute(
    "CREATE TABLE tblJournalMess (          \
      id INTEGER PRIMARY KEY AUTOINCREMENT, \
      messType   INTEGER NOT NULL,          \
      messJSON   TEXT NOT NULL,             \
      dateTime   TEXT NOT NULL);"
  )
  cr.execute(
    "CREATE TABLE tblTaskHistory (          \
      id INTEGER PRIMARY KEY AUTOINCREMENT, \
      taskId     INTEGER NOT NULL,          \
      taskState  INTEGER NOT NULL,          \
      beginTime  TEXT NOT NULL,             \
      endTime    TEXT NOT NULL,             \
      params     TEXT NOT NULL,             \
      result     TEXT NOT NULL,             \
      otherJSON  TEXT NOT NULL);"
  ) 
  cr.execute(
    "CREATE TABLE tblObjectType (           \
      id INTEGER PRIMARY KEY AUTOINCREMENT, \
      name       TEXT NOT NULL);"
  ) 
  cr.execute(
    "CREATE TABLE tblViewObject (           \
      id INTEGER PRIMARY KEY AUTOINCREMENT, \
      pipeline   INTEGER,                   \
      objType    INTEGER NOT NULL,          \
      attrJSON   TEXT NOT NULL);"
  )
  
def closeUserDb(e = None):
  db = g.pop('db', None)
  if db is not None:
      db.close()
