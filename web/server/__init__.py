from flask import(
  Flask, Blueprint, render_template
)
import os
import configparser

def create_app():
  app = Flask(__name__, instance_relative_config=True, 
                        static_url_path='', 
                        static_folder='static',
                        template_folder='html') 

  config = configparser.ConfigParser()
  iniPath = 'zmserver.cng'
  if len(config.read(iniPath)) == 0:
    raise RuntimeError(f"Not found '{iniPath}'")
  
  app.config['SECRET_KEY'] = os.urandom(16)
  dbConnectStr = config['Params']['DbConnectStr']
  if os.name == 'nt':
    os.add_dll_directory(config['Params']['PostgreLibPath'])
    os.add_dll_directory(config['Params']['CoreLibPath'])
      
  from . import user
  user.init(app.instance_path) 

  from . import core
  core.init(dbConnectStr)

  from . import events_process
  events_process.init(app.instance_path)

  from .api import api
  app.register_blueprint(api.bp)

  from . import auth 
  app.register_blueprint(auth.bp)
           
  from . import db
  db.init(app)

  bp = Blueprint('gui', __name__)
  @bp.route('/')
  @auth.loginRequired
  def index():
    return render_template('gui/index.html')

  @bp.route('/admin')
  @auth.adminRequired
  def admin():
    return render_template('gui/admin.html')

  app.register_blueprint(bp)
  app.add_url_rule('/', endpoint='/index')

  return app 