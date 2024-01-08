import os
import sqlite3
from contextlib import closing
from flask import(
  g, current_app
)
from . import zm_client as zm

def init(app):
  app.before_request(userDb)
  app.teardown_appcontext(closeUserDb)
  global m_instance_path

def userDb():
  if ('db' not in g) and g.userName and g.userName != 'admin':
    try: 
      dbPath = current_app.instance_path + '/users/{}.db'.format(g.userName)
      isExist = os.path.exists(dbPath)
      g.db = sqlite3.connect(
        dbPath,
        detect_types=sqlite3.PARSE_DECLTYPES,
        check_same_thread = False
      )    
      g.db.row_factory = sqlite3.Row      
    except Exception as err:
      print('userDb failed: %s' % str(err))

def closeUserDb(e = None):
  db = g.pop('db', None)
  if db is not None:
    db.close()

def createDb(userName : str):
  return createDbWithPath(m_instance_path, userName)
  
def createDbWithPath(path : str, userName : str):
  dbPath = path + '/users/{}.db'.format(userName)
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