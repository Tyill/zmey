from flask import Flask
import os

def create_app():
  app = Flask(__name__, instance_relative_config=True, 
                        static_url_path='', 
                        static_folder='',
                        template_folder='templates') 
  app.config.from_mapping(
    SECRET_KEY='dev',
  )
  app.config.from_pyfile('config.ini', silent=True) 

  try:
    os.makedirs(app.instance_path)
  except OSError:
    pass

  from . import user
  user.initApp(app)
      
  from . import auth 
  app.register_blueprint(auth.bp)

  # from . import blog
  # app.register_blueprint(blog.bp)
  # app.add_url_rule('/', endpoint='index')

  return app