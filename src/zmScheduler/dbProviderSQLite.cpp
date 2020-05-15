//
// zmey Project
// Copyright (C) 2018 by Contributors <https://github.com/Tyill/zmey>
//
// This code is licensed under the MIT License.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files(the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
        
#include "sqlite/sqlite.h"
#include "dbProvider.h"


using namespace std;

bool DbProvider::connect(const std::string& dbServer, const std::string& dbName){

  if (_db) return true;
          
  if (!init(dbServer)) return false;

  stringstream ss;
  ss << "CREATE TABLE IF NOT EXISTS 'tblExecutor' ("
      "'id' INTEGER PRIMARY KEY NOT NULL UNIQUE,"
      "'kind'            TEXT);";       
  if (sqlite3_exec(_db, ss.str().c_str(), nullptr, 0, nullptr) != SQLITE_OK)
    return false;        

  ss.str("");
  ss << "CREATE TABLE IF NOT EXISTS 'tblTask' ("
      "'id' INTEGER PRIMARY KEY NOT NULL UNIQUE,"
      "'managerId'       INTEGER," // id from tblManager
      "'executorId'      INTEGER," // id from tblExecutor
      "'script'          TEXT,"    // exec script
      "'averDuration'    TEXT,"    // average time execute
      "'maxDuration'     TEXT,"    // max time execute
      "FOREIGN KEY(managerId) REFERENCES tblManager(id),"
      "FOREIGN KEY(executorId) REFERENCES tblExecutor(id));";
  if (sqlite3_exec(_db, ss.str().c_str(), nullptr, 0, nullptr) != SQLITE_OK)
    return false; 

  ss.str("");
  ss << "CREATE TABLE IF NOT EXISTS 'tblState' ("
      "'id' INTEGER PRIMARY KEY NOT NULL UNIQUE,"
      "'kind'            TEXT);";
  if (sqlite3_exec(_db, ss.str().c_str(), nullptr, 0, nullptr) != SQLITE_OK)
    return false; 

  ss.str("");
  ss << "CREATE TABLE IF NOT EXISTS 'tblManager' ("
      "'id' INTEGER PRIMARY KEY NOT NULL UNIQUE,"
      "'stateId'         INTEGER," // id from tblState
      "'connectPnt'      TEXT,"    // IP or DNS ':' port
      "FOREIGN KEY(stateId) REFERENCES tblState(id));";
  if (sqlite3_exec(_db, ss.str().c_str(), nullptr, 0, nullptr) != SQLITE_OK)
    return false; 

  ss.str("");
  ss << "CREATE TABLE IF NOT EXISTS 'tblScheduler' ("
      "'id' INTEGER PRIMARY KEY NOT NULL UNIQUE,"
      "'managerId'       INTEGER," // id from tblManager
      "'stateId'         INTEGER," // id from tblState
      "'connectPnt'      TEXT,"    // IP or DNS ':' port
      "'capacityTask'    INTEGER," // default [10000]
      "'activeTask'      INTEGER," // default [0..10000]
      "FOREIGN KEY(managerId) REFERENCES tblManager(id),"
      "FOREIGN KEY(stateId) REFERENCES tblState(id));";
  if (sqlite3_exec(_db, ss.str().c_str(), nullptr, 0, nullptr) != SQLITE_OK)
    return false;

  ss.str("");
  ss << "CREATE TABLE IF NOT EXISTS 'tblWorker' ("
      "'id' INTEGER PRIMARY KEY NOT NULL UNIQUE,"
      "'managerId'       INTEGER," // id from tblManager
      "'stateId'         INTEGER," // id from tblState
      "'executorId'      INTEGER," // id from tblExecutor
      "'connectPnt'      TEXT,"    // IP or DNS ':' port
      "'capacityTask'    INTEGER," // default [10000]
      "'activeTask'      INTEGER," // default [0..10000]
      "'rating'          INTEGER," // default [0..10]
      "FOREIGN KEY(managerId) REFERENCES tblManager(id),"
      "FOREIGN KEY(stateId) REFERENCES tblState(id),"
      "FOREIGN KEY(executorId) REFERENCES tblExecutor(id));";
  if (sqlite3_exec(_db, ss.str().c_str(), nullptr, 0, nullptr) != SQLITE_OK)
    return false;

  ss.str("");
  ss << "CREATE TABLE IF NOT EXISTS 'tblTaskQueue' ("
      "'id' INTEGER PRIMARY KEY NOT NULL UNIQUE,"
      "'taskId'          INTEGER," // id from tblTask
      "'managerId'       INTEGER," // id from tblManager
      "'schedulerId'     INTEGER," // id from tblScheduler
      "'workerId'        INTEGER," // id from tblWorker
      "'stateId'         INTEGER," // id from tblState
      "'priority'        INTEGER," // [1..3]
      "'percentCompl'    INTEGER," // [0..100]
      "'params'          TEXT,"    // 
      "'createTime'      TEXT,"    // 
      "'beginTime'       TEXT,"    // 
      "'endTime'         TEXT,"    // 
      "'lastMess'        TEXT,"    // 
      "FOREIGN KEY(taskId) REFERENCES tblTask(id),"
      "FOREIGN KEY(managerId) REFERENCES tblManager(id),"
      "FOREIGN KEY(schedulerId) REFERENCES tblScheduler(id),"
      "FOREIGN KEY(workerId) REFERENCES tblWorker(id),"
      "FOREIGN KEY(stateId) REFERENCES tblState(id));";
  if (sqlite3_exec(_db, ss.str().c_str(), nullptr, 0, nullptr) != SQLITE_OK)
    return false; 

  ss.str("");
  ss << "CREATE TABLE IF NOT EXISTS 'tblPrevTasks' ("
      "'id' INTEGER PRIMARY KEY NOT NULL UNIQUE,"
      "'taskQueueId'     INTEGER," // id from tblTaskQueue
      "'prevTaskQueueId' INTEGER," // id from tblTaskQueue
      "FOREIGN KEY(taskQueueId) REFERENCES tblTaskQueue(id),"
      "FOREIGN KEY(prevTaskQueueId) REFERENCES tblTaskQueue(id));";
  if (sqlite3_exec(_db, ss.str().c_str(), nullptr, 0, nullptr) != SQLITE_OK)
    return false; 

    return true;
}
void DbProvider::disconnect(){

    if (_db){
        sqlite3_close(_db);
        _db = nullptr;
    }
}
DbProvider::~DbProvider(){

    disconnect();
}
bool DbProvider::init(const string& pathDb){

  if (sqlite3_shutdown() ||
      sqlite3_config(SQLITE_CONFIG_SINGLETHREAD) ||
      sqlite3_initialize())
    return false;

  if (sqlite3_open(pathDb.c_str(), &_db) != SQLITE_OK)
    return false;
  
  string req = "PRAGMA journal_mode = WAL;";
  if (sqlite3_exec(_db, ss.str().c_str(), nullptr, 0, nullptr) != SQLITE_OK)
    return false; 

  req = "PRAGMA foreign_keys = ON;";
  if (sqlite3_exec(_db, ss.str().c_str(), nullptr, 0, nullptr) != SQLITE_OK)
    return false; 
  
  req = "PRAGMA synchronous = OFF;";
  if (sqlite3_exec(_db, ss.str().c_str(), nullptr, 0, nullptr) != SQLITE_OK)
    return false; 
          
  return true;
}
bool DbProvider::query(const string& query, vector<vector<string>>& results) const{

  FUNC_BEGIN

  results.clear();

  sqlite3_stmt* statement;
  if (sqlite3_prepare_v2(_db, query.c_str(), -1, &statement, 0) == SQLITE_OK){

    int cols = sqlite3_column_count(statement);
    int result = 0;
    while (true){

      result = sqlite3_step(statement);

      if (result == SQLITE_ROW){
        vector<string> values;
        for (int col = 0; col < cols; col++){
          char* rv = (char*)sqlite3_column_text(statement, col);
          if (rv) values.push_back(rv);
        }
        if (!values.empty()) results.push_back(values);
      }
      else break;
    }

    sqlite3_finalize(statement);

  }
  else
    return false;

  FUNC_END
}
