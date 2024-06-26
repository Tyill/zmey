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
#include "pg_impl.h"
#include "base/messages.h"

using namespace std;

namespace db{

bool DbProvider::setListenNewTaskNotify(bool on){
  lock_guard<mutex> lk(m_impl->m_mtx);

  string cmd = on ? "LISTEN " : "UNLISTEN ";
  cmd += m_impl->NOTIFY_NAME_NEW_TASK;

  PGres pgr(PQexec(pg_, cmd.c_str()));
  if (PQresultStatus(pgr.res) != PGRES_COMMAND_OK){
    errorMess(string("setListenNewTaskNotify: ") + PQerrorMessage(pg_));
    return false;
  }
  return true;
}

bool DbProvider::getSchedr(const std::string& connPnt, base::Scheduler& outCng){
  lock_guard<mutex> lk(m_impl->m_mtx);
  
  stringstream ss;
  ss << "SELECT id, state, capacityTask, activeTask "
        "FROM tblScheduler "
        "WHERE connPnt = '" << connPnt << "' AND isDeleted = FALSE;";

  PGres pgr(PQexec(pg_, ss.str().c_str()));
  if (PQresultStatus(pgr.res) != PGRES_TUPLES_OK){
    errorMess(string("getSchedr: ") + PQerrorMessage(pg_));
    return false;
  }
  if (PQntuples(pgr.res) == 0){
    errorMess("getSchedr error: such schedr does not exist");
    return false;
  }
  outCng.sId = stoi(PQgetvalue(pgr.res, 0, 0));
  outCng.sConnectPnt = connPnt;
  outCng.sState = base::StateType(atoi(PQgetvalue(pgr.res, 0, 1)));
  outCng.sCapacityTaskCount = atoi(PQgetvalue(pgr.res, 0, 2));
  outCng.sActiveTaskCount = atoi(PQgetvalue(pgr.res, 0, 3));
  return true;
}
bool DbProvider::getTasksOfSchedr(int sId, std::vector<base::Task>& out){
  lock_guard<mutex> lk(m_impl->m_mtx);
  stringstream ss;
  ss << "SELECT tq.id, COALESCE(tp.workerPreset, 0), tp.averDurationSec, tp.maxDurationSec,"
        "       tp.params, tp.scriptPath, tp.resultPath, ts.state "
        "FROM tblTaskQueue tq "
        "JOIN tblTaskState ts ON ts.qtask = tq.id "
        "JOIN tblTaskParam tp ON tp.qtask = tq.id "
        "WHERE tq.schedr = " << sId << " AND tq.worker IS NULL AND (ts.state BETWEEN " << (int)base::StateType::START << " AND " << (int)base::StateType::PAUSE << ")";

  PGres pgr(PQexec(pg_, ss.str().c_str()));
  if (PQresultStatus(pgr.res) != PGRES_TUPLES_OK){
    errorMess(string("getTasksOfSchedr: ") + PQerrorMessage(pg_));
    return false;
  }
  int tsz = PQntuples(pgr.res);
  for (int i = 0; i < tsz; ++i){
    out.push_back(base::Task {
      stoi(PQgetvalue(pgr.res, i, 0)),
      stoi(PQgetvalue(pgr.res, i, 1)),
      stoi(PQgetvalue(pgr.res, i, 2)),
      stoi(PQgetvalue(pgr.res, i, 3)),
      PQgetvalue(pgr.res, i, 4),
      PQgetvalue(pgr.res, i, 5),
      PQgetvalue(pgr.res, i, 6),
      base::StateType(stoi(PQgetvalue(pgr.res, i, 7)))
    });
  }
  return true;
}
bool DbProvider::getTasksOfWorker(int sId, int wId, std::vector<base::Task>& out){
  lock_guard<mutex> lk(m_impl->m_mtx);
  stringstream ss;
  ss << "SELECT tq.id, tp.averDurationSec, tp.maxDurationSec, tp.params, tp.scriptPath, tp.resultPath, ts.state "
        "FROM tblTaskQueue tq "
        "JOIN tblTaskState ts ON ts.qtask = tq.id "
        "JOIN tblTaskParam tp ON tp.qtask = tq.id "
        "WHERE tq.schedr = " << sId << " AND tq.worker = " << wId << " AND (ts.state BETWEEN " << (int)base::StateType::START << " AND " << (int)base::StateType::PAUSE << ")";

  PGres pgr(PQexec(pg_, ss.str().c_str()));
  if (PQresultStatus(pgr.res) != PGRES_TUPLES_OK){
    errorMess(string("getTasksOfWorker: ") + PQerrorMessage(pg_));
    return false;
  }
  int tsz = PQntuples(pgr.res);
  for (int i = 0; i < tsz; ++i){
    out.push_back(base::Task {
      stoi(PQgetvalue(pgr.res, i, 0)),
      wId,
      stoi(PQgetvalue(pgr.res, i, 1)),
      stoi(PQgetvalue(pgr.res, i, 2)),
      PQgetvalue(pgr.res, i, 3),
      PQgetvalue(pgr.res, i, 4),
      PQgetvalue(pgr.res, i, 5),
      base::StateType(stoi(PQgetvalue(pgr.res, i, 6)))
    });
  }
  return true;
}
bool DbProvider::getWorkersOfSchedr(int sId, std::vector<base::Worker>& out){
  lock_guard<mutex> lk(m_impl->m_mtx);
  stringstream ss;
  ss << "SELECT id, state, capacityTask, activeTask, connPnt "
        "FROM tblWorker "
        "WHERE schedr = " << sId << " AND isDeleted = FALSE;";

  PGres pgr(PQexec(pg_, ss.str().c_str()));
  if (PQresultStatus(pgr.res) != PGRES_TUPLES_OK){
    errorMess(string("getWorkersOfSchedr: ") + PQerrorMessage(pg_));
    return false;
  }
  int wsz = PQntuples(pgr.res);
  for(int i =0; i < wsz; ++i){
    base::Worker w;{ 
      w.wId = atoi(PQgetvalue(pgr.res, i, 0));
      w.sId = sId;
      w.wState = w.wStateMem = base::StateType(atoi(PQgetvalue(pgr.res, i, 1)));
      w.wCapacityTaskCount = atoi(PQgetvalue(pgr.res, i, 2));
      w.wActiveTaskCount = atoi(PQgetvalue(pgr.res, i, 3));
      w.wConnectPnt = PQgetvalue(pgr.res, i, 4);
      w.wIsActive = w.wState != base::StateType::NOT_RESPONDING;
    }
    out.push_back(w);
  }
  return true;
}
bool DbProvider::getNewTasksForSchedr(int sId, int maxTaskCnt, std::vector<base::Task>& out){
  lock_guard<mutex> lk(m_impl->m_mtx);  
  
  PQconsumeInput(pg_);
  m_impl->m_notifyAuxCheckTOut.updateCycTime();
  
  bool isNewTask = false;
  PGnotify* notify = PQnotifies(pg_);
  if (notify){
    isNewTask = std::string(notify->relname) == m_impl->NOTIFY_NAME_NEW_TASK;
    PQfreemem(notify);
  }
  bool auxCheckTimeout = m_impl->m_notifyAuxCheckTOut.onDelayOncSec(true, 10, 0);
  if (!isNewTask && m_impl->m_firstReqNewTasks && !auxCheckTimeout){
    return true;
  }
  m_impl->m_firstReqNewTasks = true;
  
  stringstream ss;
  ss << "SELECT * FROM funcNewTasksForSchedr(" << sId << "," << maxTaskCnt << ");";

  PGres pgr(PQexec(pg_, ss.str().c_str()));
  if (PQresultStatus(pgr.res) != PGRES_TUPLES_OK){
    errorMess(string("getNewTasksForSchedr ") + PQerrorMessage(pg_));
    return false;
  }
  int tsz = PQntuples(pgr.res);
  for (int i = 0; i < tsz; ++i){
    out.push_back(base::Task {
      stoi(PQgetvalue(pgr.res, i, 0)),
      stoi(PQgetvalue(pgr.res, i, 1)),
      stoi(PQgetvalue(pgr.res, i, 2)),
      stoi(PQgetvalue(pgr.res, i, 3)),
      PQgetvalue(pgr.res, i, 4),
      PQgetvalue(pgr.res, i, 5),
      PQgetvalue(pgr.res, i, 6)
    });
  }
  int cnt = 1;
  while ((cnt < tsz) && ((notify = PQnotifies(pg_)) != nullptr)){
    PQfreemem(notify);
    ++cnt;
  }
  return true;
}
bool DbProvider::sendAllMessFromSchedr(int sId, std::vector<db::MessSchedr>& mess){
  lock_guard<mutex> lk(m_impl->m_mtx);
  
  if(mess.empty()){
    return true;
  }
  stringstream ss;
  for (const auto& m : mess){
    switch (m.type){     
      case mess::MessType::TASK_ERROR:
      case mess::MessType::TASK_COMPLETED: 
        ss << "UPDATE tblTaskTime SET "
              "stopTime = current_timestamp "
              "WHERE qtask = " << m.taskId << ";";
         
        if (m.type == mess::MessType::TASK_ERROR){ 
          ss << "UPDATE tblTaskState SET "
                "state = " << (int)base::StateType::ERRORT << " "
                "WHERE qtask = " << m.taskId << ";";
        }
        else if (m.type == mess::MessType::TASK_COMPLETED){                      
          ss << "UPDATE tblTaskState SET "
                "state = " << (int)base::StateType::COMPLETED << " "
                "WHERE qtask = " << m.taskId << ";";
        }
        break;                
      case mess::MessType::TASK_RUNNING:
        ss << "UPDATE tblTaskState SET "
              "state = " << (int)base::StateType::RUNNING << " "
              "WHERE qtask = " << m.taskId << ";"

              "UPDATE tblTaskQueue SET "
              "worker = " << m.workerId << " "
              "WHERE id = " << m.taskId << ";"
          
              "UPDATE tblTaskTime SET "
              "startTime = current_timestamp "
              "WHERE qtask = " << m.taskId << ";";
        break;         
      case mess::MessType::TASK_PAUSE:
        ss << "UPDATE tblTaskState SET "
              "state = " << (int)base::StateType::PAUSE << " "
              "WHERE qtask = " << m.taskId << ";"; 
        break;     
      case mess::MessType::TASK_CONTINUE: // worker talk, when run task
        ss << "UPDATE tblTaskState SET "
              "state = " << (int)base::StateType::RUNNING << " "
              "WHERE qtask = " << m.taskId << ";"; 
        break;    
      case mess::MessType::TASK_STOP:     
        ss << "UPDATE tblTaskTime SET "
              "stopTime = current_timestamp "
              "WHERE qtask = " << m.taskId << ";"
              
              "UPDATE tblTaskState SET "
              "state = " << (int)base::StateType::STOP << " "
              "WHERE qtask = " << m.taskId << ";"; 
        break;
      case mess::MessType::TASK_PROGRESS:
        ss << "UPDATE tblTaskState SET "
              "progress = " << m.taskProgress << " "
              "WHERE qtask = " << m.taskId << ";";
        break;
      case mess::MessType::PAUSE_SCHEDR:
        ss << "UPDATE tblScheduler SET "
              "state = " << (int)base::StateType::PAUSE << " "
              "WHERE id = " << sId << ";";
        break;
      case mess::MessType::START_SCHEDR:
        ss << "UPDATE tblScheduler SET "
              "state = " << (int)base::StateType::RUNNING << ", "
              "startTime = current_timestamp "
              "WHERE id = " << sId << ";";
        break;
      case mess::MessType::STOP_SCHEDR:
        ss << "UPDATE tblScheduler SET "
              "state = " << (int)base::StateType::STOP << ", "
              "stopTime = current_timestamp "
              "WHERE id = " << sId << ";";
        break;
      case mess::MessType::START_AFTER_PAUSE_SCHEDR:
        ss << "UPDATE tblScheduler SET "
              "state = " << (int)base::StateType::RUNNING << " "
              "WHERE id = " << sId << ";";
        break;
      case mess::MessType::PING_SCHEDR:{
        ss << "UPDATE tblScheduler SET "
              "state = " << (int)base::StateType::RUNNING << ", "
              "activeTask = " << m.activeTaskCount << ", "
              "pingTime = current_timestamp "
              "WHERE id = " << sId << ";";
        break;
      }
      case mess::MessType::PAUSE_WORKER:
        ss << "UPDATE tblWorker SET "
              "state = " << (int)base::StateType::PAUSE << " "
              "WHERE id = " << m.workerId << ";";
        break;
      case mess::MessType::START_WORKER:
        ss << "UPDATE tblWorker SET "
              "state = " << (int)base::StateType::RUNNING << ", "
              "startTime = current_timestamp "
              "WHERE id = " << m.workerId << ";";
        break;
      case mess::MessType::START_AFTER_PAUSE_WORKER:
        ss << "UPDATE tblWorker SET "
              "state = " << (int)base::StateType::RUNNING << " "
              "WHERE id = " << m.workerId << ";";
        break;
      case mess::MessType::PING_WORKER:{
        ss << "UPDATE tblWorker SET "
              "activeTask = " << m.activeTaskCount << ", "
              "pingTime = current_timestamp, "
              "load = " << m.workerLoad << " "
              "WHERE id = " << m.workerId << ";";
        break;
      }        
      case mess::MessType::WORKER_NOT_RESPONDING:
        ss << "UPDATE tblWorker SET "
              "state = " << (int)base::StateType::NOT_RESPONDING << ", "
              "stopTime = current_timestamp "
              "WHERE id = " << m.workerId << ";";
        break;
      case mess::MessType::JUST_START_WORKER:
        ss << "UPDATE tblWorker SET "
              "startTime = current_timestamp "
              "WHERE id = " << m.workerId << ";";
        break;
      case mess::MessType::STOP_WORKER:             
        ss << "UPDATE tblWorker SET "
              "state = " << (int)base::StateType::STOP << ", "
              "stopTime = current_timestamp "
              "WHERE id = " << m.workerId << ";";
        break;
      case mess::MessType::INTERN_ERROR:
        if (m.workerId){
          ss << "INSERT INTO tblInternError (schedr, worker, message) VALUES("
                "'" << sId << "',"
                "'" << m.workerId << "',"
                "'" << m.message << "');";
        }else{
          ss << "INSERT INTO tblInternError (schedr, message) VALUES("
                "'" << sId << "',"
                "'" << m.message << "');";
        }
        break;
    }    
  }
  PGres pgr(PQexec(pg_, ss.str().c_str()));
  if (PQresultStatus(pgr.res) != PGRES_COMMAND_OK){
    errorMess(string("sendAllMessFromSchedr: ") + PQerrorMessage(pg_));
    return false;
  }
  return true;
}
}