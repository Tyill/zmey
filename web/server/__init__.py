from flask import(
  Flask, Blueprint, render_template
)
import os

def create_app():
  app = Flask(__name__, instance_relative_config=True, 
                        static_url_path='', 
                        static_folder='static',
                        template_folder='html') 
  
  app.config['SECRET_KEY'] = os.urandom(16)
  app.config['DbConnectStr'] = 'host=localhost port=5432 password=123 dbname=zmeydb connect_timeout=10'
  app.config['PostgreLibPath'] = 'c:/Program Files/PostgreSQL/10/bin/'
  app.config['CoreLibPath'] = 'c:/cpp/other/zmey/build/Release/'
  
  os.add_dll_directory(app.config['PostgreLibPath'])
  os.add_dll_directory(app.config['CoreLibPath'])
    
  from . import user
  user.init(app.instance_path) 

  from . import core
  core.init(app.config['DbConnectStr'])

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