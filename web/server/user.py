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

def init(instance_path):
  os.makedirs(instance_path + '/users', exist_ok = True) 
  with open(instance_path + '/users/users.ini', 'a') as file:
    file.write("")

def calcHash(passw, salt):  
  return hashlib.sha256(salt.encode() + passw.encode()).hexdigest()

def add(name, passw) -> bool:
  salt = uuid.uuid4().hex
  hash = calcHash(passw, salt)
  with open(app.instance_path + '/users/users.ini', 'a') as file:
    file.write(f'{len(all())}:{name}:{hash}:{salt}\n')
  return True

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
