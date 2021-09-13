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

using namespace std;

namespace ZM_DB{

bool DbProvider::setListenNewTaskNotify(bool on){
  lock_guard<mutex> lk(m_impl->m_mtx);

  string cmd = on ? "LISTEN " : "UNLISTEN ";
  cmd += m_impl->NOTIFY_NAME_NEW_TASK;

  PGres pgr(PQexec(_pg, cmd.c_str()));
  if (PQresultStatus(pgr.res) != PGRES_COMMAND_OK){
    errorMess(string("setListenNewTaskNotify: ") + PQerrorMessage(_pg));
    return false;
  }
  return true;
}

bool DbProvider::getSchedr(const std::string& connPnt, ZM_Base::Scheduler& outCng){
  lock_guard<mutex> lk(m_impl->m_mtx);
  auto cp = ZM_Aux::split(connPnt, ':');
  if (cp.size() != 2){
    errorMess(string("getSchedr error: connPnt not correct"));
    return false;
  }
  stringstream ss;
  ss << "SELECT s.id, s.state, s.capacityTask, s.activeTask, s.internalData, s.name, s.description FROM tblScheduler s "
        "JOIN tblConnectPnt cp ON cp.id = s.connPnt "
        "WHERE cp.ipAddr = '" << cp[0] << "' AND cp.port = '" << cp[1] << "' AND s.isDelete = 0;";

  PGres pgr(PQexec(_pg, ss.str().c_str()));
  if (PQresultStatus(pgr.res) != PGRES_TUPLES_OK){
    errorMess(string("getSchedr: ") + PQerrorMessage(_pg));
    return false;
  }
  if (PQntuples(pgr.res) == 0){
    errorMess("getSchedr error: such schedr does not exist");
    return false;
  }
  outCng.id = stoull(PQgetvalue(pgr.res, 0, 0));
  outCng.connectPnt = connPnt;
  outCng.state = (ZM_Base::StateType)atoi(PQgetvalue(pgr.res, 0, 1));
  outCng.capacityTask = atoi(PQgetvalue(pgr.res, 0, 2));
  outCng.activeTask = atoi(PQgetvalue(pgr.res, 0, 3));
  outCng.internalData = PQgetvalue(pgr.res, 0, 4);
  outCng.name = PQgetvalue(pgr.res, 0, 5);
  outCng.description = PQgetvalue(pgr.res, 0, 6);
  return true;
}
bool DbProvider::getTasksById(uint64_t sId, const std::vector<uint64_t>& tasksId, std::vector<ZM_Base::Task>& out){
  lock_guard<mutex> lk(m_impl->m_mtx);

  string tId;
  tId = accumulate(tasksId.begin(), tasksId.end(), tId,
                [](string& s, uint64_t v){
                  return s.empty() ? to_string(v) : s + "," + to_string(v);
                }); 
  stringstream ss;
  ss << "SELECT tq.id, tt.averDurationSec, tt.maxDurationSec, COALESCE(tt.workerPreset, 0),"
        "       tt.script, tp.params "
        "FROM tblTaskTemplate tt "
        "JOIN tblTaskQueue tq ON tq.taskTempl = tt.id "
        "JOIN tblTaskState ts ON ts.qtask = tq.id "
        "JOIN tblTaskParam tp ON tp.qtask = tq.id "
        "WHERE tq.schedr = " << sId << " AND tq.id IN (" << tId << ") AND (ts.state BETWEEN " << (int)ZM_Base::StateType::START << " AND " << (int)ZM_Base::StateType::PAUSE << ")";

  PGres pgr(PQexec(_pg, ss.str().c_str()));
  if (PQresultStatus(pgr.res) != PGRES_TUPLES_OK){
    errorMess(string("getTasksById: ") + PQerrorMessage(_pg));
    return false;
  }
  int tsz = PQntuples(pgr.res);
  for (int i = 0; i < tsz; ++i){
    out.push_back(ZM_Base::Task {
      stoull(PQgetvalue(pgr.res, i, 0)),
      stoull(PQgetvalue(pgr.res, i, 3)),
      atoi(PQgetvalue(pgr.res, i, 1)),
      atoi(PQgetvalue(pgr.res, i, 2)),
      PQgetvalue(pgr.res, i, 4),
      PQgetvalue(pgr.res, i, 5)
    });
  }
  return true;
}  
bool DbProvider::getTasksOfSchedr(uint64_t sId, std::vector<ZM_Base::Task>& out){
  lock_guard<mutex> lk(m_impl->m_mtx);
  stringstream ss;
  ss << "SELECT tq.id, tt.averDurationSec, tt.maxDurationSec, COALESCE(tt.workerPreset, 0),"
        "       tt.script, tp.params "
        "FROM tblTaskTemplate tt "
        "JOIN tblTaskQueue tq ON tq.taskTempl = tt.id "
        "JOIN tblTaskState ts ON ts.qtask = tq.id "
        "JOIN tblTaskParam tp ON tp.qtask = tq.id "
        "WHERE tq.schedr = " << sId << " AND tq.worker IS NULL AND (ts.state BETWEEN " << (int)ZM_Base::StateType::START << " AND " << (int)ZM_Base::StateType::PAUSE << ")";

  PGres pgr(PQexec(_pg, ss.str().c_str()));
  if (PQresultStatus(pgr.res) != PGRES_TUPLES_OK){
    errorMess(string("getTasksOfSchedr: ") + PQerrorMessage(_pg));
    return false;
  }
  int tsz = PQntuples(pgr.res);
  for (int i = 0; i < tsz; ++i){
    out.push_back(ZM_Base::Task {
      stoull(PQgetvalue(pgr.res, i, 0)),
      stoull(PQgetvalue(pgr.res, i, 3)),
      atoi(PQgetvalue(pgr.res, i, 1)),
      atoi(PQgetvalue(pgr.res, i, 2)),
      PQgetvalue(pgr.res, i, 4),
      PQgetvalue(pgr.res, i, 5)
    });
  }
  return true;
}
bool DbProvider::getTasksOfWorker(uint64_t sId, uint64_t wId, std::vector<uint64_t>& outTasksId){
  lock_guard<mutex> lk(m_impl->m_mtx);
  stringstream ss;
  ss << "SELECT tq.id "
        "FROM tblTaskQueue tq "
        "JOIN tblTaskState ts ON ts.qtask = tq.id "
        "WHERE tq.schedr = " << sId << " AND tq.worker = " << wId << " AND (ts.state BETWEEN " << (int)ZM_Base::StateType::START << " AND " << (int)ZM_Base::StateType::PAUSE << ")";

  PGres pgr(PQexec(_pg, ss.str().c_str()));
  if (PQresultStatus(pgr.res) != PGRES_TUPLES_OK){
    errorMess(string("getTasksOfWorker: ") + PQerrorMessage(_pg));
    return false;
  }
  int tsz = PQntuples(pgr.res);
  for (int i = 0; i < tsz; ++i){
    outTasksId.push_back(stoull(PQgetvalue(pgr.res, i, 0)));
  }
  return true;
}
bool DbProvider::getWorkersOfSchedr(uint64_t sId, std::vector<ZM_Base::Worker>& out){
  lock_guard<mutex> lk(m_impl->m_mtx);
  stringstream ss;
  ss << "SELECT w.id, w.state, w.capacityTask, w.activeTask, cp.ipAddr, cp.port, w.name, w.description "
        "FROM tblWorker w "
        "JOIN tblConnectPnt cp ON cp.id = w.connPnt "
        "WHERE w.schedr = " << sId << " AND w.isDelete = 0;";

  PGres pgr(PQexec(_pg, ss.str().c_str()));
  if (PQresultStatus(pgr.res) != PGRES_TUPLES_OK){
    errorMess(string("getWorkersOfSchedr: ") + PQerrorMessage(_pg));
    return false;
  }
  int wsz = PQntuples(pgr.res);
  for(int i =0; i < wsz; ++i){
    out.push_back(ZM_Base::Worker{ stoull(PQgetvalue(pgr.res, i, 0)),
                                   sId,
                                   (ZM_Base::StateType)atoi(PQgetvalue(pgr.res, i, 1)),
                                   atoi(PQgetvalue(pgr.res, i, 2)),
                                   atoi(PQgetvalue(pgr.res, i, 3)),
                                   ZM_Base::Worker::RATING_MAX,
                                   0,
                                   PQgetvalue(pgr.res, i, 4) + string(":") + PQgetvalue(pgr.res, i, 5),
                                   PQgetvalue(pgr.res, i, 6),
                                   PQgetvalue(pgr.res, i, 7)});
  }
  return true;
}
bool DbProvider::getNewTasksForSchedr(uint64_t sId, int maxTaskCnt, std::vector<ZM_Base::Task>& out){
  lock_guard<mutex> lk(m_impl->m_mtx);  
  
  PQconsumeInput(_pg);
  m_impl->m_notifyAuxCheckTOut.updateCycTime();
  
  bool isNewTask = false;
  PGnotify* notify = PQnotifies(_pg);
  if (notify){
    isNewTask = std::string(notify->relname) == m_impl->NOTIFY_NAME_NEW_TASK;
    PQfreemem(notify);
  }
  bool auxCheckTimeout = m_impl->m_notifyAuxCheckTOut.onDelayOncSec(true, 10, 0);
  if (!isNewTask && m_impl->m_firstReqNewTasks && !auxCheckTimeout) return true;
  m_impl->m_firstReqNewTasks = true;
  
  stringstream ss;
  ss << "SELECT * FROM funcNewTasksForSchedr(" << sId << "," << maxTaskCnt << ");";

  PGres pgr(PQexec(_pg, ss.str().c_str()));
  if (PQresultStatus(pgr.res) != PGRES_TUPLES_OK){
    errorMess(string("getNewTasksForSchedr ") + PQerrorMessage(_pg));
    return false;
  }
  int tsz = PQntuples(pgr.res);
  for (int i = 0; i < tsz; ++i){
    out.push_back(ZM_Base::Task {
      stoull(PQgetvalue(pgr.res, i, 0)),
      stoull(PQgetvalue(pgr.res, i, 3)),
      atoi(PQgetvalue(pgr.res, i, 1)),
      atoi(PQgetvalue(pgr.res, i, 2)),
      PQgetvalue(pgr.res, i, 4),
      PQgetvalue(pgr.res, i, 5)
    });
  }
  int cnt = 1;
  while ((cnt < tsz) && ((notify = PQnotifies(_pg)) != nullptr)){
    PQfreemem(notify);
    ++cnt;
  }
  return true;
}
bool DbProvider::sendAllMessFromSchedr(uint64_t sId, std::vector<ZM_DB::MessSchedr>& mess){
  lock_guard<mutex> lk(m_impl->m_mtx);
  
  if(mess.empty()){
    return true;
  }
  stringstream ss;
  for (const auto& m : mess){
    switch (m.type){     
      case ZM_Base::MessType::TASK_ERROR:
      case ZM_Base::MessType::TASK_COMPLETED: 
        ss << "UPDATE tblTaskTime SET "
              "stopTime = current_timestamp "
              "WHERE qtask = " << m.taskId << ";"
              
              "UPDATE tblTaskResult SET "
              "result = '" << m.data << "' "
              "WHERE qtask = " << m.taskId << ";";

        if (m.type == ZM_Base::MessType::TASK_ERROR){ 
          ss << "UPDATE tblTaskState SET "
                "state = " << (int)ZM_Base::StateType::ERRORT << " "
                "WHERE qtask = " << m.taskId << ";";
        }
        else if (m.type == ZM_Base::MessType::TASK_COMPLETED){                      
          ss << "UPDATE tblTaskState SET "
                "state = " << (int)ZM_Base::StateType::COMPLETED << ", "
                "progress = 100 "
                "WHERE qtask = " << m.taskId << ";";
        }
        break;                
      case ZM_Base::MessType::TASK_RUNNING:
        ss << "UPDATE tblTaskState SET "
              "state = " << (int)ZM_Base::StateType::RUNNING << " "
              "WHERE qtask = " << m.taskId << ";"

              "UPDATE tblTaskQueue SET "
              "worker = " << m.workerId << " "
              "WHERE id = " << m.taskId << ";"
          
              "UPDATE tblTaskTime SET "
              "startTime = current_timestamp "
              "WHERE qtask = " << m.taskId << ";";
        break;         
      case ZM_Base::MessType::TASK_PAUSE:
        ss << "UPDATE tblTaskState SET "
              "state = " << (int)ZM_Base::StateType::PAUSE << " "
              "WHERE qtask = " << m.taskId << ";"; 
        break;     
      case ZM_Base::MessType::TASK_CONTINUE: // worker talk, when run task
        ss << "UPDATE tblTaskState SET "
              "state = " << (int)ZM_Base::StateType::RUNNING << " "
              "WHERE qtask = " << m.taskId << ";"; 
        break;    
      case ZM_Base::MessType::TASK_STOP:     
        ss << "UPDATE tblTaskTime SET "
              "stopTime = current_timestamp "
              "WHERE qtask = " << m.taskId << ";"
              
              "UPDATE tblTaskState SET "
              "state = " << (int)ZM_Base::StateType::STOP << " "
              "WHERE qtask = " << m.taskId << ";"; 
        break;
      case ZM_Base::MessType::TASK_PROGRESS:
        ss << "UPDATE tblTaskState SET "
              "progress = " << stoi(m.data) << " "
              "WHERE qtask = " << m.taskId << ";";
        break;
      case ZM_Base::MessType::PAUSE_SCHEDR:
        ss << "UPDATE tblScheduler SET "
              "state = " << (int)ZM_Base::StateType::PAUSE << " "
              "WHERE id = " << sId << ";";
        break;
      case ZM_Base::MessType::START_SCHEDR:
        ss << "UPDATE tblScheduler SET "
              "state = " << (int)ZM_Base::StateType::RUNNING << ", "
              "startTime = current_timestamp "
              "WHERE id = " << sId << ";";
        break;
      case ZM_Base::MessType::STOP_SCHEDR:
        ss << "UPDATE tblScheduler SET "
              "state = " << (int)ZM_Base::StateType::STOP << ", "
              "stopTime = current_timestamp "
              "WHERE id = " << sId << ";";
        break;
      case ZM_Base::MessType::START_AFTER_PAUSE_SCHEDR:
        ss << "UPDATE tblScheduler SET "
              "state = " << (int)ZM_Base::StateType::RUNNING << " "
              "WHERE id = " << sId << ";";
        break;
      case ZM_Base::MessType::PING_SCHEDR:{
        ss << "UPDATE tblScheduler SET "
              "state = " << (int)ZM_Base::StateType::RUNNING << ", "
              "activeTask = " << stoi(m.data) << ", "
              "pingTime = current_timestamp "
              "WHERE id = " << sId << ";";
        break;
      }
      case ZM_Base::MessType::PAUSE_WORKER:
        ss << "UPDATE tblWorker SET "
              "state = " << (int)ZM_Base::StateType::PAUSE << " "
              "WHERE id = " << m.workerId << ";";
        break;
      case ZM_Base::MessType::START_WORKER:
        ss << "UPDATE tblWorker SET "
              "state = " << (int)ZM_Base::StateType::RUNNING << ", "
              "startTime = current_timestamp "
              "WHERE id = " << m.workerId << ";";
        break;
      case ZM_Base::MessType::START_AFTER_PAUSE_WORKER:
        ss << "UPDATE tblWorker SET "
              "state = " << (int)ZM_Base::StateType::RUNNING << " "
              "WHERE id = " << m.workerId << ";";
        break;
      case ZM_Base::MessType::PING_WORKER:{
        auto data = ZM_Aux::split(m.data, '\t');
        ss << "UPDATE tblWorker SET "
              "activeTask = " << stoi(data[0]) << ", "
              "pingTime = current_timestamp, "
              "load = " << stoi(data[1]) << " "
              "WHERE id = " << m.workerId << ";";
        break;
      }        
      case ZM_Base::MessType::WORKER_NOT_RESPONDING:
        ss << "UPDATE tblWorker SET "
              "state = " << (int)ZM_Base::StateType::NOT_RESPONDING << ", "
              "stopTime = current_timestamp "
              "WHERE id = " << m.workerId << ";";
        break;
      case ZM_Base::MessType::JUST_START_WORKER:
        ss << "UPDATE tblWorker SET "
              "startTime = current_timestamp "
              "WHERE id = " << m.workerId << ";";
        break;
      case ZM_Base::MessType::STOP_WORKER:             
        ss << "UPDATE tblWorker SET "
              "state = " << (int)ZM_Base::StateType::STOP << ", "
              "stopTime = current_timestamp "
              "WHERE id = " << m.workerId << ";";
        break;
      case ZM_Base::MessType::INTERN_ERROR:
        if (m.workerId){
          ss << "INSERT INTO tblInternError (schedr, worker, message) VALUES("
                "'" << sId << "',"
                "'" << m.workerId << "',"
                "'" << m.data << "');";
        }else{
          ss << "INSERT INTO tblInternError (schedr, message) VALUES("
                "'" << sId << "',"
                "'" << m.data << "');";
        }
        break;
    }    
  }
  PGres pgr(PQexec(_pg, ss.str().c_str()));
  if (PQresultStatus(pgr.res) != PGRES_COMMAND_OK){
    errorMess(string("sendAllMessFromSchedr: ") + PQerrorMessage(_pg));
    return false;
  }
  return true;
}
}