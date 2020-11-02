import functools

from flask import (
    Blueprint, flash, g, redirect, render_template, request, session, url_for
)
import zmBackend.zmey as zm

bp = Blueprint('auth', __name__, url_prefix='/auth')

@bp.route('/register', methods=('GET', 'POST'))
def register():
  if request.method == 'POST':
    username = request.form['username']
    password = request.form['password']
    error = None

    if not username:
      error = 'Username is required.'
    elif not password:
      error = 'Password is required.'
    elif len([usr for usr in zm.allUsers() if usr.name == username]):    
      error = 'User {} is already registered.'.format(username)
    elif not zm.addUser(zm.User(0, username, password)):
      error = zm.getLastError()
    
    if error is None:
      return redirect(url_for('auth.login'))

    flash(error)

    return render_template('auth/register.html')

@bp.route('/login', methods=('GET', 'POST'))
def login():
  if request.method == 'POST':
    username = request.form['username']
    password = request.form['password']
   
    error = None
    usr = zm.getUser(username, password)
    if usr is None:
      error = 'Incorrect username or password.'
   
    if error is None:
        session.clear()
        session['userId'] = usr.id
        return redirect(url_for('index'))

    flash(error)

  return render_template('auth/login.html')

@bp.before_app_request
def load_logged_in_user():
    g.userId = session.get('userId')
    
@bp.route('/logout')
def logout():
    session.clear()
    return redirect(url_for('index'))

def login_required(view):
    @functools.wraps(view)
    def wrapped_view(**kwargs):
        if g.userId is None:
            return redirect(url_for('auth.login'))

        return view(**kwargs)

    return wrapped_view