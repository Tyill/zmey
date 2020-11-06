from flask import Flask
import os

appPath = os.path.dirname(__file__)

os.environ['PATH'] = appPath + os.pathsep + os.environ['PATH']

def create_app():
  app = Flask(__name__, instance_relative_config=True, 
                        static_url_path='', 
                        static_folder='',
                        template_folder='templates') 
  
  app.config['SECRET_KEY'] = os.urandom(16)
  app.config['ZMEY_CONNECTION_STR'] = 'host=localhost port=5432 password=123 dbname=zmeyDb connect_timeout=10'
  app.config.from_pyfile('config.ini', silent=True) 
  
  try: 
    os.makedirs(app.instance_path + '/users')
  except OSError:
    pass

  from . import zmey
  zmey.initApp(app.config['ZMEY_CONNECTION_STR']) 
  app.register_blueprint(zmey.bp)
   
  from . import gui
  gui.initApp(app) 
  app.register_blueprint(gui.bp)
  app.add_url_rule('/', endpoint='/index')

  from . import auth 
  app.register_blueprint(auth.bp)
  
  return app 