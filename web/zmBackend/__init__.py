from flask import Flask
import os

def create_app():
  app = Flask(__name__, instance_relative_config=True, 
                        static_url_path='', 
                        static_folder='',
                        template_folder='templates') 
  
  app.config['SECRET_KEY'] = 'dev'
  app.config['ZMEY_CONN_STR'] = 'host=localhost port=5432 password=123 dbname=zmeyDb connect_timeout=10'
  app.config.from_pyfile('config.ini', silent=True) 
  
  try: 
    os.makedirs(app.instance_path)
  except OSError:
    pass
   
  from . import zmey
  zmey.initApp(app.config['ZMEY_CONN_STR']) 

  from . import auth 
  app.register_blueprint(auth.bp)

  from . import user
  user.initApp(app)
  app.register_blueprint(user.bp)
  app.add_url_rule('/', endpoint='index')

  return app 