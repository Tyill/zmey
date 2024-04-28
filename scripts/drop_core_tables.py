#!/usr/bin/python3
# -*- coding: utf-8 -*-


import psycopg2

def dropTables():
  with psycopg2.connect(dbname='zmeydb', user='postgres', password='postgres', host='localhost') as pg:
    csr = pg.cursor()
    csr.execute(
        "DROP TABLE IF EXISTS tblState cascade;"
        "DROP TABLE IF EXISTS tblScheduler cascade;"
        "DROP TABLE IF EXISTS tblWorker cascade;"
        "DROP TABLE IF EXISTS tblInternError cascade;"
        "DROP TABLE IF EXISTS tblTaskQueue cascade;"
        "DROP TABLE IF EXISTS tblTaskState cascade;"
        "DROP TABLE IF EXISTS tblTaskTime cascade;"
        "DROP TABLE IF EXISTS tblTaskParam cascade;"

        "DROP INDEX IF EXISTS inxTSState cascade;"
        "DROP INDEX IF EXISTS inxTQSchedr cascade;"
        "DROP INDEX IF EXISTS inxTQWorker cascade;" 

        "DROP FUNCTION IF EXISTS funcStartTask cascade;"
        "DROP FUNCTION IF EXISTS funcStartTaskNotify cascade;"
        "DROP FUNCTION IF EXISTS funcChangeTaskNotify cascade;"
        "DROP FUNCTION IF EXISTS funcNewTasksForSchedr cascade;"
        
        "DROP TRIGGER IF EXISTS trgNewTask ON tblTaskQueue cascade;"
        "DROP TRIGGER IF EXISTS trgTaskChange ON tblTaskState cascade;"
    )
    csr.close()

if __name__ == "__main__":
  dropTables()
