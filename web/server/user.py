from typing import List
import os
import uuid
import hashlib
import threading

m_lock = threading.Lock()

m_instance_path = ""

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
  global m_instance_path
  m_instance_path = instance_path
  if not os.path.exists(instance_path + '/users/users.ini'):
    os.makedirs(instance_path + '/users', exist_ok = True) 
    addp(0, 'admin', 'p@ssw0rd', instance_path)

def calcHash(passw, salt):  
  return hashlib.sha256(salt.encode() + passw.encode()).hexdigest()

def addp(id, name, passw, path) -> bool:
  salt = uuid.uuid4().hex
  hash = calcHash(passw, salt)
  if id < 0:
    id = len(all())
  m_lock.acquire()  
  with open(path + '/users/users.ini', 'a') as file:
    file.write(f'{id}:{name}:{hash}:{salt}\n')
  m_lock.release() 
  return True

def add(name, passw) -> bool:
  return addp(-1, name, passw, m_instance_path)

def get(uname, passw) -> User:
  with open(m_instance_path + '/users/users.ini', 'r') as file:
    for l in file.readlines():
      id, name, hash, salt = l[:-1].split(':')
      if (name == uname) and (calcHash(passw, salt) == hash): 
        return User(int(id), name) 
  return None

def getById(uid : int) -> User:
  with open(m_instance_path + '/users/users.ini', 'r') as file:
    for l in file.readlines():
      id, name, _, _ = l[:-1].split(':')
      if (uid == int(id)): 
        return User(uid, name)
  return None

def all() -> List[User]:  
  return allWithPath(m_instance_path)

def allWithPath(path) -> List[User]:  
  users = []  
  with open(path + '/users/users.ini', 'r') as file:
    for l in file.readlines():
      id, name, _, _ = l.split(':')
      users.append(User(int(id), name))       
  return users
