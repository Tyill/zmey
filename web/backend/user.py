import sqlite3
from flask import current_app, g

def _db(uname : str):
  if 'db' not in g:
    g.db = sqlite3.connect(
        current_app.instance_path + '/users/{}.sqlite'.format(uname),
        detect_types=sqlite3.PARSE_DECLTYPES
    )
    g.db.row_factory = sqlite3.Row
  return g.db

def _closeDb(e=None):
    db = g.pop('db', None)

    if db is not None:
        db.close()

def initApp(app):
    app.teardown_appcontext(_closeDb)