#!/usr/bin/python3
# -*- coding: utf-8 -*-

import psycopg2

def createTables():
  with psycopg2.connect(dbname='zmeydb', user='postgres', password='postgres', host='localhost') as pg:
    csr = pg.cursor()
    csr.execute(
      
      "CREATE TABLE IF NOT EXISTS tblUser("
      "id           SERIAL PRIMARY KEY,"
      "name         TEXT NOT NULL CHECK (kind <> ''),"
      "description  TEXT NOT NULL,"
      "guiSetts     TEXT NOT NULL,"
      "isDeleted    BOOLEAN DEFAULT FALSE);"
      
      "CREATE TABLE IF NOT EXISTS tblPipeline ("
      "id            SERIAL PRIMARY KEY,"
      "name          TEXT NOT NULL,"
      "description   TEXT NOT NULL,"
      "setts         TEXT NOT NULL,"
      "isDeleted     BOOLEAN NOT NULL DEFAULT FALSE);"
  
      "CREATE TABLE IF NOT EXISTS tblTaskTemplate ("
      "id              SERIAL PRIMARY KEY,"
      "name            TEXT NOT NULL CHECK (name <> ''),"
      "description     TEXT NOT NULL,"
      "averDurationSec INT NOT NULL CHECK (averDurationSec >= 0),"
      "maxDurationSec  INT NOT NULL CHECK (maxDurationSec >= 0),"
      "scriptPath      TEXT NOT NULL CHECK (scriptPath <> ''),"
      "resultPath      TEXT NOT NULL CHECK (resultPath <> ''),"      
      "isDeleted       BOOLEAN NOT NULL DEFAULT FALSE);"
  
      "CREATE TABLE IF NOT EXISTS tblPipelineTask("
      "id             SERIAL PRIMARY KEY,"
      "pplId          INT NOT NULL REFERENCES tblPipeline,"
      "ttId           INT NOT NULL REFERENCES tblTaskTemplate,"
      "name           TEXT NOT NULL CHECK (name <> ''),"
      "description    TEXT NOT NULL,"
      "setts          TEXT NOT NULL,"
      "params         TEXT NOT NULL,"
      "nextTasksIds   TEXT NOT NULL,"
      "prevTasksIds   TEXT NOT NULL,"
      "isStartNextIds TEXT NOT NULL,"      
      "isEnabled      BOOLEAN NOT NULL DEFAULT TRUE,"
      "hasChanged     BOOLEAN NOT NULL DEFAULT FALSE,"
      "isDeleted      BOOLEAN NOT NULL DEFAULT FALSE);"
   
      "CREATE TABLE IF NOT EXISTS tblEvent("
      "id                     SERIAL PRIMARY KEY,"
      "name                   TEXT NOT NULL CHECK (name <> ''),"
      "description            TEXT NOT NULL,"   
      "tasksForStartIds       TEXT NOT NULL,"
      "timeStartEverySec      INT NOT NULL,"
      "timeLastStartEverySec  INT NOT NULL DEFAULT 0,"
      "timeStartOnceOfDay     TEXT NOT NULL,"
      "timeLastStartOnceOfDay TEXT NOT NULL DEFAULT '',"      
      "isEnabled              BOOLEAN NOT NULL DEFAULT TRUE,"
      "hasChanged             BOOLEAN NOT NULL DEFAULT FALSE,"
      "isDeleted              BOOLEAN NOT NULL DEFAULT FALSE);"

      "CREATE TABLE IF NOT EXISTS tblTask(,"
      "id               SERIAL PRIMARY KEY,"
      "pplTaskId        INT NOT NULL REFERENCES tblPipelineTask,"
      "starterPplTaskId INT REFERENCES tblPipelineTask,"
      "starterEventId   INT REFERENCES tblEvent,"
      "ttlId            INT NOT NULL REFERENCES tblTaskTemplate);"
    )
    csr.close()

if __name__ == "__main__":
  createTables()
