from flask import(
  Flask, Blueprint, render_template
)
import os
import sys

def create_app():
  app = Flask(__name__, instance_relative_config=True, 
                        static_url_path='', 
                        static_folder='',
                        template_folder='html') 
  
  app.config['SECRET_KEY'] = os.urandom(16)
  app.config['DbConnectStr'] = 'host=localhost port=5432 password=123 dbname=zmeydb connect_timeout=10'
  app.config['PostgreLibPath'] = 'c:/Program Files/PostgreSQL/10/bin/'
  app.config['ClientLibPath'] = 'c:/cpp/other/zmey/build/Release/'
  app.config['PyClientPath'] = 'c:/cpp/other/zmey'
      
  SCRIPT_DIR = os.path.dirname(os.path.realpath(os.path.join(os.getcwd(), os.path.expanduser(__file__))))
  sys.path.append(os.path.normpath(os.path.join(SCRIPT_DIR, app.config['PyClientPath'])))
  
  os.add_dll_directory(app.config['PostgreLibPath'])
  os.add_dll_directory(app.config['ClientLibPath'])

  os.makedirs(app.instance_path + '/users', exist_ok = True) 
 
  from . import api
  api.init(app.config['DbConnectStr'])
  app.register_blueprint(api.bp)

  from . import auth 
  app.register_blueprint(auth.bp)
   
  from . import user_db
  user_db.init(app)

  bp = Blueprint('gui', __name__)
  @bp.route('/')
  @auth.loginRequired
  def index():
    return render_template('gui/index.html')

  app.register_blueprint(bp)
  app.add_url_rule('/', endpoint='/index')

  
  return app 