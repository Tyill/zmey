class User: 
  """User config""" 
  def __init__(self, 
               id : int = 0,
               name : str = "",
               passw : str = ""):
    self.id = id
    self.name = name
    self.passw = passw

def add(usr : User) -> bool:
  return _zmCommon.addUser(usr)

def get(uname : str, passw : str) -> User:  
  usr = User(0, uname, passw)
  return usr if _zmCommon.getUserId(usr) else None

def change(usr : User) -> bool:
  return _zmCommon.changeUser(usr)

def all() -> List[User]:
  return _zmCommon.getAllUsers()