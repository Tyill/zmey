import psycopg2

def clearDB():
  with psycopg2.connect(dbname='zmeydb', password='123', host='localhost') as pg:
    csr = pg.cursor()
    csr.execute("drop table if exists tblScheduler cascade;" +
                "drop table if exists tblWorker cascade;" +
                "drop table if exists tblTaskTemplate cascade;" +
                "drop table if exists tblTaskState cascade;" +
                "drop table if exists tblTaskTime cascade;" + 
                "drop table if exists tblTaskResult cascade;" +
                "drop table if exists tblTaskParam cascade;" +
                "drop table if exists tblTaskQueue cascade;" + 
                "drop table if exists tblState cascade;" +
                "drop table if exists tblInternError cascade;" + 
                "drop table if exists tblConnectPnt cascade;" + 
                "drop function if exists funcstarttask(integer,text);" +
                "drop function if exists funcnewtasksforschedr(integer,integer);" + 
                "drop function if exists funcStartTaskNotify();" +
                "drop index if exists inxTSState cascade;" + 
                "drop index if exists inxTQSchedr cascade;" + 
                "drop index if exists inxTQTaskTempl cascade;" + 
                "drop index if exists inxTQWorker cascade;" + 
                "drop trigger if exists trgTaskQueue ON tblTaskQueue cascade;")
    csr.close()
