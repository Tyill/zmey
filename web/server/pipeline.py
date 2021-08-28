from typing import List
from contextlib import closing
from flask import g

class Pipeline: 
  """Pipeline config""" 
  def __init__(self,
               id : int = 0,
               name : str = "",
               description : str = "",
               setts = "{}"):
    self.id = id  
    self.name = name    
    self.description = description 
    self.setts = setts 
  def __repr__(self):
      return f"Pipeline: id {self.id} name {self.name} description {self.description} setts {self.setts}"
  def __str__(self):
    return self.__repr__()

def add(ppl : Pipeline) -> bool:
  if 'db' in g:
    try:
      with closing(g.db.cursor()) as cr:
        cr.execute(
          "INSERT INTO tblPipeline (name, description, setts) VALUES("
          f'"{ppl.name}",'
          f'"{ppl.description}",'
          f'"{ppl.setts}");'
        )
        ppl.id = cr.lastrowid
        g.db.commit()
      return True
    except Exception as err:
      print("{0} local db query failed: {1}".format("Pipeline.add", str(err)))
  return False

def change(ppl : Pipeline) -> bool:
  if 'db' in g:
    try:
      with closing(g.db.cursor()) as cr:
        cr.execute(
          "UPDATE tblPipeline SET "
          f'name = "{ppl.name}",'
          f'description = "{ppl.description}",'
          f'setts = "{ppl.setts}" '
          f'WHERE id = {ppl.id};'  
        )
        g.db.commit()
      return True
    except Exception as err:
      print("{0} local db query failed: {1}".format("Pipeline.change", str(err)))
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
      print("{0} local db query failed: {1}".format("Pipeline.delete", str(err)))
  return False

def all() -> List[Pipeline]:
  if 'db' in g:
    try:
      ppls = []
      with closing(g.db.cursor()) as cr:
        cr.execute(
          "SELECT id, name, description, setts "
          "FROM tblPipeline "
          "WHERE isDelete = 0;"
        )
        rows = cr.fetchall()
        for row in rows:
          ppls.append(Pipeline(id=row[0], name=row[1], description=row[2], setts=row[3]))       
      return ppls  
    except Exception as err:
      print("{0} local db query failed: {1}".format("Pipeline.all", str(err)))
  return []