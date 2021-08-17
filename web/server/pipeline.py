from typing import List

class Pipeline: 
  """Pipeline config""" 
  def __init__(self,
               id : int = 0,
               uId : int = 0,
               name : str = "",
               description : str = ""):
    self.id = id
    self.userId = uId           
    self.name = name    
    self.description = description 