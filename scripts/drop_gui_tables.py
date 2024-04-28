#!/usr/bin/python3
# -*- coding: utf-8 -*-


import psycopg2

def dropTables():
  with psycopg2.connect(dbname='zmeydb', user='postgres', password='postgres', host='localhost') as pg:
    csr = pg.cursor()
    csr.execute(
        "DROP TABLE IF EXISTS tblUser cascade;"
        "DROP TABLE IF EXISTS tblPipeline cascade;"
        "DROP TABLE IF EXISTS tblTaskTemplate cascade;"
        "DROP TABLE IF EXISTS tblPipelineTask cascade;"
        "DROP TABLE IF EXISTS tblEvent cascade;"
        "DROP TABLE IF EXISTS tblTask cascade;"
    )
    csr.close()

if __name__ == "__main__":
  dropTables()
