from typing import List
from contextlib import closing
from flask import g

class Pipeline: 
  """Pipeline config""" 
  def __init__(self,
               id : int = 0,
               isVisible : int = 0,
               name : str = "",
               description : str = ""):
    self.id = id  
    self.isVisible = isVisible 
    self.name = name    
    self.description = description 
  def __repr__(self):
      return f"Pipeline: id {self.id} name {self.name} description {self.description}"
  def __str__(self):
    return self.__repr__()

def add(ppl : Pipeline) -> bool:
  if 'db' in g:
    try:
      with closing(g.db.cursor()) as cr:
        cr.execute(
          "INSERT INTO tblPipeline (isVisible, name, description) VALUES("
          f"'{ppl.isVisible}',"
          f"'{ppl.name}',"
          f"'{ppl.description}');"
        )
        ppl.id = cr.lastrowid
        g.db.commit()
      return True
    except Exception as err:
      print('Local db query failed: %s' % str(err))
  return False

def change(ppl : Pipeline) -> bool:
  if 'db' in g:
    try:
      with closing(g.db.cursor()) as cr:
        cr.execute(
          "UPDATE tblPipeline SET "
          f"isVisible = '{ppl.isVisible}',"
          f"name = '{ppl.name}',"
          f"description = '{ppl.description}' "
          f"WHERE id = {ppl.id};"  
        )
        g.db.commit()
      return True
    except Exception as err:
      print('Local db query failed: %s' % str(err))
  return False

def delete(pplId) -> bool:
  if 'db' in g:    
    try:
      with closing(g.db.cursor()) as cr:
        cr.execute(
          "UPDATE tblPipeline SET "
          "isDelete = 1 "
          f"WHERE id = {pplId};" 
        )
        g.db.commit()
      return True
    except Exception as err:
      print('Local db query failed: %s' % str(err))
  return False

def all() -> List[Pipeline]:
  if 'db' in g:
    try:
      ppls = []
      with closing(g.db.cursor()) as cr:
        cr.execute(
          "SELECT id, name, description, isVisible "
          "FROM tblPipeline "
          "WHERE isDelete = 0;"
        )
        rows = cr.fetchall()
        for row in rows:
          ppls.append(Pipeline(id=row[0], name=row[1], description=row[2], isVisible=row[3]))       
      return ppls  
    except Exception as err:
      print('Local db query failed: %s' % str(err))
  return []