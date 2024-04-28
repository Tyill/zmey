from flask import g
import psycopg2

def init(app):
  app.before_request(userDb)
  app.teardown_appcontext(closeUserDb)
  global m_instance_path

def userDb():
  if ('db' not in g) and g.userName and g.userName != 'admin':
    try: 
      g.db = psycopg2.connect(dbname='zmeydb', user='postgres', password='postgres', host='localhost')
    except Exception as err:
      print('userDb failed: %s' % str(err))

def closeUserDb(e = None):
  db = g.pop('db', None)
  if db is not None:
    db.close()

def createDb():
  db = None
  try: 
    db = psycopg2.connect(dbname='zmeydb', user='postgres', password='postgres', host='localhost')
  except Exception as err:
    print('userDb failed: %s' % str(err))
  return db
  
def closeDb(db):
  if db is not None:
    db.close()