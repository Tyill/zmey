from flask import Flask
import os

def create_app():
  app = Flask(__name__, instance_relative_config=True, 
                        static_url_path='', 
                        static_folder='',
                        template_folder='templates') 
  
  app.config['SECRET_KEY'] = os.urandom(16)
  app.config['ZmeyConnectStr'] = 'host=localhost port=5432 password=123 dbname=zmeydb connect_timeout=10'
  app.config['PostgreLibPath'] = 'c:/Program Files/PostgreSQL/10/bin/'
  
  os.add_dll_directory(app.config['PostgreLibPath'])
  os.add_dll_directory(os.path.dirname(__file__))

  os.makedirs(app.instance_path + '/users', exist_ok = True) 
  
  from . import api
  api.initApp(app.config['ZmeyConnectStr'])
  app.register_blueprint(api.bp)
   
  from . import gui
  gui.initApp(app) 
  app.register_blueprint(gui.bp)
  app.add_url_rule('/', endpoint='/index')

  from . import auth 
  app.register_blueprint(auth.bp)
  
  return app 