import functools
from markupsafe import escape
from flask import (
  Blueprint, flash, g, redirect, render_template, request, session, url_for
)
from . import user

bp = Blueprint('auth', __name__, url_prefix='/auth')

@bp.route('/register', methods=('GET', 'POST'))
def register():
  try:
    if request.method == 'POST':
      username = escape(request.form['username'])
      password = escape(request.form['password'])
      error = None
        
      if not username:
        error = 'Username is required.'
      elif not password:
        error = 'Password is required.'
      elif len([usr for usr in user.all() if usr and usr.name == username]):       
        error = 'User {} is already registered.'.format(username)       
      elif not user.add(username, password):
        error = 'Internal error'

      if error is None:      
        return redirect(url_for('auth.login'))

      flash(error)
  except Exception as err:
    print(f'/register POST failed: %s' % str(err))

  return render_template('auth/register.html')

@bp.route('/login', methods=('GET', 'POST'))
def login():
  try:
    if request.method == 'POST':
      username = escape(request.form['username'])
      password = escape(request.form['password'])
      error = None
    
      usr = user.get(username, password)
      if usr is None:
        error = 'Incorrect username or password.'
      else:
        session.clear()
        session['userName'] = username
        session['userId'] = usr.id      
        return redirect(url_for('gui.index')) if username != 'admin' else redirect(url_for('gui.admin'))

      flash(error)
  except Exception as err:
    print(f'/login POST failed: %s' % str(err))

  return render_template('auth/login.html')

@bp.before_app_request
def load():
  g.userName = session.get('userName')
  g.userId = session.get('userId')
    
@bp.route('/logout')
def logout():
  session.clear()
  return redirect(url_for('auth.login'))
  
def loginRequired(view):
  @functools.wraps(view)
  def wrapped_view(**kwargs):
    if g.userId is None or g.userName == 'admin':
      return redirect(url_for('auth.login'))
    return view(**kwargs)
  return wrapped_view

def adminRequired(view):
  @functools.wraps(view)
  def wrapped_view(**kwargs):
    if g.userId is None or g.userName != 'admin':
      return redirect(url_for('auth.login'))
    return view(**kwargs)
  return wrapped_view
