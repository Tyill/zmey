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

namespace db{
  
bool DbProvider::addWorker(const base::Worker& worker, int& outWkrId){
  lock_guard<mutex> lk(m_impl->m_mtx);
  auto connPnt = misc::split(worker.wConnectPnt, ':');
  if (connPnt.size() != 2){
    errorMess("addWorker error: connectPnt not correct");
    return false;
  }
  stringstream ss;
  ss << "INSERT INTO tblWorker (connPnt, schedr, state, capacityTask) VALUES("
        "'" << worker.wConnectPnt << "',"
        "'" << (int)worker.sId << "',"
        "'" << (int)worker.wState << "',"
        "'" << worker.wCapacityTaskCount << "') RETURNING id;";

  PGres pgr(PQexec(pg_, ss.str().c_str()));
  if (PQresultStatus(pgr.res) != PGRES_TUPLES_OK){
      errorMess(string("addWorker: ") + PQerrorMessage(pg_));
      return false;
  }
  outWkrId = stoi(PQgetvalue(pgr.res, 0, 0));
  return true;
}
bool DbProvider::getWorker(int wId, base::Worker& cng){
  lock_guard<mutex> lk(m_impl->m_mtx);
  stringstream ss;
  ss << "SELECT connPnt, schedr, state, capacityTask "
        "FROM tblWorker "
        "WHERE id = " << wId << " AND isDeleted = FALSE;";

  PGres pgr(PQexec(pg_, ss.str().c_str()));
  if (PQresultStatus(pgr.res) != PGRES_TUPLES_OK){
    errorMess(string("getWorker: ") + PQerrorMessage(pg_));
    return false;
  }
  if (PQntuples(pgr.res) != 1){
    errorMess(string("getWorker error: such worker does not exist"));
    return false;
  }
  cng.wConnectPnt = PQgetvalue(pgr.res, 0, 0);
  cng.sId = stoi(PQgetvalue(pgr.res, 0, 1));
  cng.wState = base::StateType(atoi(PQgetvalue(pgr.res, 0, 2)));
  cng.wCapacityTaskCount = atoi(PQgetvalue(pgr.res, 0, 3));
  return true;
}
bool DbProvider::changeWorker(int wId, const base::Worker& newCng){
  lock_guard<mutex> lk(m_impl->m_mtx);
  auto connPnt = misc::split(newCng.wConnectPnt, ':');
  if (connPnt.size() != 2){
    errorMess("changeWorker error: connectPnt not correct");
    return false;
  }
  stringstream ss;
  ss << "UPDATE tblWorker SET "
        "schedr = '" << (int)newCng.sId << "',"
        "capacityTask = '" << newCng.wCapacityTaskCount << "',"
        "connPnt = '" << newCng.wConnectPnt << "' "
        "WHERE id = " << wId << " AND isDeleted = FALSE;";

  PGres pgr(PQexec(pg_, ss.str().c_str()));
  if (PQresultStatus(pgr.res) != PGRES_COMMAND_OK){
    errorMess(string("changeWorker: ") + PQerrorMessage(pg_));
    return false;
  }  
  return true;
}
bool DbProvider::delWorker(int wId){
  lock_guard<mutex> lk(m_impl->m_mtx);
  stringstream ss;
  ss << "UPDATE tblWorker SET "
        "isDeleted = TRUE "
        "WHERE id = " << wId << ";";

  PGres pgr(PQexec(pg_, ss.str().c_str()));
  if (PQresultStatus(pgr.res) != PGRES_COMMAND_OK){
    errorMess(string("delWorker: ") + PQerrorMessage(pg_));
    return false;
  }  
  return true;
}
 
bool DbProvider::workerState(const std::vector<int>& wId, std::vector<WorkerState>& out){
  lock_guard<mutex> lk(m_impl->m_mtx);
  string swId;
  swId = accumulate(wId.begin(), wId.end(), swId,
                [](string& s, int v){
                  return s.empty() ? to_string(v) : s + "," + to_string(v);
                }); 
  stringstream ss;
  ss << "SELECT state, activeTask, load, startTime, stopTime, pingTime "
        "FROM tblWorker "
        "WHERE id IN (" << swId << ")  AND isDeleted = FALSE ORDER BY id;";

  PGres pgr(PQexec(pg_, ss.str().c_str()));
  if (PQresultStatus(pgr.res) != PGRES_TUPLES_OK){
    errorMess(string("workerState: ") + PQerrorMessage(pg_));
    return false;
  }
  size_t wsz = wId.size();
  if (PQntuples(pgr.res) != wsz){
    errorMess("workerState error: PQntuples(pgr.res) != wsz");
    return false;
  }
  out.resize(wsz);
  for (size_t i = 0; i < wsz; ++i){
    out[i].state = (base::StateType)atoi(PQgetvalue(pgr.res, (int)i, 0));
    out[i].activeTaskCount = atoi(PQgetvalue(pgr.res, (int)i, 1));
    out[i].load = atoi(PQgetvalue(pgr.res, (int)i, 2));
    out[i].startTime = PQgetvalue(pgr.res, (int)i, 3);
    out[i].stopTime = PQgetvalue(pgr.res, (int)i, 4);
    out[i].pingTime = PQgetvalue(pgr.res, (int)i, 5);
  }
  return true;
}
std::vector<int> DbProvider::getAllWorkers(int sId, base::StateType state){
  lock_guard<mutex> lk(m_impl->m_mtx);
  stringstream ss;
  ss << "SELECT id FROM tblWorker "
        "WHERE (state = " << (int)state << " OR " << (int)state << " = 0) "
        "AND schedr = " << sId << " "
        "AND isDeleted = FALSE;";

  PGres pgr(PQexec(pg_, ss.str().c_str()));
  if (PQresultStatus(pgr.res) != PGRES_TUPLES_OK){
    errorMess(string("getAllWorkers: ") + PQerrorMessage(pg_));
    return std::vector<int>();
  }  
  int rows = PQntuples(pgr.res);
  std::vector<int> ret(rows);
  for (int i = 0; i < rows; ++i){
    ret[i] = stoi(PQgetvalue(pgr.res, i, 0));
  }
  return ret;
}
}