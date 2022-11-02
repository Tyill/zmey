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

namespace DB{
  
bool DbProvider::addWorker(const Base::Worker& worker, int& outWkrId){
  lock_guard<mutex> lk(m_impl->m_mtx);
  auto connPnt = Aux::split(worker.connectPnt, ':');
  if (connPnt.size() != 2){
    errorMess("addWorker error: connectPnt not correct");
    return false;
  }
  stringstream ss;
  ss << "INSERT INTO tblWorker (connPnt, schedr, state, capacityTask, name, description) VALUES("
        "'" << worker.connectPnt << "',"
        "'" << (int)worker.sId << "',"
        "'" << (int)worker.state << "',"
        "'" << worker.capacityTask << "',"
        "'" << worker.name << "',"
        "'" << worker.description << "') RETURNING id;";

  PGres pgr(PQexec(_pg, ss.str().c_str()));
  if (PQresultStatus(pgr.res) != PGRES_TUPLES_OK){
      errorMess(string("addWorker: ") + PQerrorMessage(_pg));
      return false;
  }
  outWkrId = stoull(PQgetvalue(pgr.res, 0, 0));
  return true;
}
bool DbProvider::getWorker(int wId, Base::Worker& cng){
  lock_guard<mutex> lk(m_impl->m_mtx);
  stringstream ss;
  ss << "SELECT connPnt, schedr, state, capacityTask, name, description "
        "FROM tblWorker "
        "WHERE id = " << wId << " AND isDelete = 0;";

  PGres pgr(PQexec(_pg, ss.str().c_str()));
  if (PQresultStatus(pgr.res) != PGRES_TUPLES_OK){
    errorMess(string("getWorker: ") + PQerrorMessage(_pg));
    return false;
  }
  if (PQntuples(pgr.res) != 1){
    errorMess(string("getWorker error: such worker does not exist"));
    return false;
  }
  cng.connectPnt = PQgetvalue(pgr.res, 0, 0);
  cng.sId = stoull(PQgetvalue(pgr.res, 0, 1));
  cng.state = (Base::StateType)atoi(PQgetvalue(pgr.res, 0, 2));
  cng.capacityTask = atoi(PQgetvalue(pgr.res, 0, 3));
  cng.name = PQgetvalue(pgr.res, 0, 4);
  cng.description = PQgetvalue(pgr.res, 0, 5);
  return true;
}
bool DbProvider::changeWorker(int wId, const Base::Worker& newCng){
  lock_guard<mutex> lk(m_impl->m_mtx);
  auto connPnt = Aux::split(newCng.connectPnt, ':');
  if (connPnt.size() != 2){
    errorMess("changeWorker error: connectPnt not correct");
    return false;
  }
  stringstream ss;
  ss << "UPDATE tblWorker SET "
        "schedr = '" << (int)newCng.sId << "',"
        "capacityTask = '" << newCng.capacityTask << "',"
        "connPnt = '" << newCng.connectPnt << "',"
        "name = '" << newCng.name << "',"
        "description = '" << newCng.description << "' "
        "WHERE id = " << wId << " AND isDelete = 0;";

  PGres pgr(PQexec(_pg, ss.str().c_str()));
  if (PQresultStatus(pgr.res) != PGRES_COMMAND_OK){
    errorMess(string("changeWorker: ") + PQerrorMessage(_pg));
    return false;
  }  
  return true;
}
bool DbProvider::delWorker(int wId){
  lock_guard<mutex> lk(m_impl->m_mtx);
  stringstream ss;
  ss << "UPDATE tblWorker SET "
        "isDelete = 1 "
        "WHERE id = " << wId << ";";

  PGres pgr(PQexec(_pg, ss.str().c_str()));
  if (PQresultStatus(pgr.res) != PGRES_COMMAND_OK){
    errorMess(string("delWorker: ") + PQerrorMessage(_pg));
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
        "WHERE id IN (" << swId << ")  AND isDelete = 0 ORDER BY id;";

  PGres pgr(PQexec(_pg, ss.str().c_str()));
  if (PQresultStatus(pgr.res) != PGRES_TUPLES_OK){
    errorMess(string("workerState: ") + PQerrorMessage(_pg));
    return false;
  }
  size_t wsz = wId.size();
  if (PQntuples(pgr.res) != wsz){
    errorMess("workerState error: PQntuples(pgr.res) != wsz");
    return false;
  }
  out.resize(wsz);
  for (size_t i = 0; i < wsz; ++i){
    out[i].state = (Base::StateType)atoi(PQgetvalue(pgr.res, (int)i, 0));
    out[i].activeTask = atoi(PQgetvalue(pgr.res, (int)i, 1));
    out[i].load = atoi(PQgetvalue(pgr.res, (int)i, 2));
    out[i].startTime = PQgetvalue(pgr.res, (int)i, 3);
    out[i].stopTime = PQgetvalue(pgr.res, (int)i, 4);
    out[i].pingTime = PQgetvalue(pgr.res, (int)i, 5);
  }
  return true;
}
std::vector<int> DbProvider::getAllWorkers(int sId, Base::StateType state){
  lock_guard<mutex> lk(m_impl->m_mtx);
  stringstream ss;
  ss << "SELECT id FROM tblWorker "
        "WHERE (state = " << (int)state << " OR " << (int)state << " = -1) "
        "AND schedr = " << sId << " "
        "AND isDelete = 0;";

  PGres pgr(PQexec(_pg, ss.str().c_str()));
  if (PQresultStatus(pgr.res) != PGRES_TUPLES_OK){
    errorMess(string("getAllWorkers: ") + PQerrorMessage(_pg));
    return std::vector<int>();
  }  
  int rows = PQntuples(pgr.res);
  std::vector<int> ret(rows);
  for (int i = 0; i < rows; ++i){
    ret[i] = stoull(PQgetvalue(pgr.res, i, 0));
  }
  return ret;
}
}