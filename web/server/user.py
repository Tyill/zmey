from typing import List
import os
import uuid
import hashlib
from flask import(
  current_app as app
)

class User: 
  """User config""" 
  def __init__(self, 
               id : int = 0,
               name : str = ""):
    self.id = id
    self.name = name
  def __repr__(self):
      return f"User: id {self.id} name {self.name}"
  def __str__(self):
    return self.__repr__()

def init(instance_path):
  if not os.path.exists(instance_path + '/users/users.ini'):
    os.makedirs(instance_path + '/users', exist_ok = True) 
    addp(0, 'admin', 'password', instance_path)

def calcHash(passw, salt):  
  return hashlib.sha256(salt.encode() + passw.encode()).hexdigest()

def addp(id, name, passw, path) -> bool:
  salt = uuid.uuid4().hex
  hash = calcHash(passw, salt)
  if id < 0:
    id = len(all())
  with open(path + '/users/users.ini', 'a') as file:
    file.write(f'{id}:{name}:{hash}:{salt}\n')
  return True

def add(name, passw) -> bool:
  return addp(-1, name, passw, app.instance_path)

def get(uname, passw) -> User:
  with open(app.instance_path + '/users/users.ini', 'r') as file:
    for l in file.readlines():
      id, name, hash, salt = l[:-1].split(':')
      if (name == uname) and (calcHash(passw, salt) == hash): 
        return User(int(id), name) 
  return None

def all() -> List[User]:  
  users = []
  with open(app.instance_path + '/users/users.ini', 'r') as file:
    for l in file.readlines():
      id, name, _, _ = l.split(':')
      users.append(User(int(id), name))       
  return users
