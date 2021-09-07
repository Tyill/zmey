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
  
bool DbProvider::addWorker(const ZM_Base::Worker& worker, uint64_t& outWkrId){
  lock_guard<mutex> lk(m_impl->m_mtx);
  auto connPnt = ZM_Aux::split(worker.connectPnt, ':');
  if (connPnt.size() != 2){
    errorMess("addWorker error: connectPnt not correct");
    return false;
  }
  stringstream ss;
  ss << "WITH ncp AS (INSERT INTO tblConnectPnt (ipAddr, port) VALUES("
        " '" << connPnt[0] << "',"
        " '" << connPnt[1] << "') RETURNING id)"
        "INSERT INTO tblWorker (connPnt, schedr, state, capacityTask, name, description) VALUES("
        "(SELECT id FROM ncp),"
        "'" << (int)worker.sId << "',"
        "'" << (int)worker.state << "',"
        "'" << worker.capacityTask << "',"
        "'" << worker.name << "',"
        "'" << worker.description << "') RETURNING id;";

  PGres pgr(PQexec(_pg, ss.str().c_str()));
  if (PQresultStatus(pgr.res) != PGRES_TUPLES_OK){
      errorMess(string("addWorker error: ") + PQerrorMessage(_pg));
      return false;
  }
  outWkrId = stoull(PQgetvalue(pgr.res, 0, 0));
  return true;
}
bool DbProvider::getWorker(uint64_t wId, ZM_Base::Worker& cng){
  lock_guard<mutex> lk(m_impl->m_mtx);
  stringstream ss;
  ss << "SELECT cp.ipAddr, cp.port, w.schedr, w.state, w.capacityTask, w.name, w.description FROM tblWorker w "
        "JOIN tblConnectPnt cp ON cp.id = w.connPnt "
        "WHERE w.id = " << wId << " AND w.isDelete = 0;";

  PGres pgr(PQexec(_pg, ss.str().c_str()));
  if (PQresultStatus(pgr.res) != PGRES_TUPLES_OK){
    errorMess(string("getWorker error: ") + PQerrorMessage(_pg));
    return false;
  }
  if (PQntuples(pgr.res) != 1){
    errorMess(string("getWorker error: such worker does not exist"));
    return false;
  }
  cng.connectPnt = PQgetvalue(pgr.res, 0, 0) + string(":") + PQgetvalue(pgr.res, 0, 1);
  cng.sId = stoull(PQgetvalue(pgr.res, 0, 2));
  cng.state = (ZM_Base::StateType)atoi(PQgetvalue(pgr.res, 0, 3));
  cng.capacityTask = atoi(PQgetvalue(pgr.res, 0, 4));
  cng.name = PQgetvalue(pgr.res, 0, 5);
  cng.description = PQgetvalue(pgr.res, 0, 6);
  return true;
}
bool DbProvider::changeWorker(uint64_t wId, const ZM_Base::Worker& newCng){
  lock_guard<mutex> lk(m_impl->m_mtx);
  auto connPnt = ZM_Aux::split(newCng.connectPnt, ':');
  if (connPnt.size() != 2){
    errorMess("changeWorker error: connectPnt not correct");
    return false;
  }
  stringstream ss;
  ss << "UPDATE tblWorker SET "
        "schedr = '" << (int)newCng.sId << "',"
        "capacityTask = '" << newCng.capacityTask << "',"
        "name = '" << newCng.name << "',"
        "description = '" << newCng.description << "' "
        "WHERE id = " << wId << " AND isDelete = 0; "

        "UPDATE tblConnectPnt SET "
        "ipAddr = '" << connPnt[0] << "',"
        "port = '" << connPnt[1] << "' "
        "WHERE id = (SELECT connPnt FROM tblWorker WHERE id = " << wId << " AND isDelete = 0);";

  PGres pgr(PQexec(_pg, ss.str().c_str()));
  if (PQresultStatus(pgr.res) != PGRES_COMMAND_OK){
    errorMess(string("changeWorker error: ") + PQerrorMessage(_pg));
    return false;
  }  
  return true;
}
bool DbProvider::delWorker(uint64_t wId){
  lock_guard<mutex> lk(m_impl->m_mtx);
  stringstream ss;
  ss << "UPDATE tblWorker SET "
        "isDelete = 1 "
        "WHERE id = " << wId << ";";

  PGres pgr(PQexec(_pg, ss.str().c_str()));
  if (PQresultStatus(pgr.res) != PGRES_COMMAND_OK){
    errorMess(string("delWorker error: ") + PQerrorMessage(_pg));
    return false;
  }  
  return true;
}
 
bool DbProvider::workerState(const std::vector<uint64_t>& wId, std::vector<WorkerState>& out){
  lock_guard<mutex> lk(m_impl->m_mtx);
  string swId;
  swId = accumulate(wId.begin(), wId.end(), swId,
                [](string& s, uint64_t v){
                  return s.empty() ? to_string(v) : s + "," + to_string(v);
                }); 
  stringstream ss;
  ss << "SELECT state, activeTask, load, startTime, stopTime "
        "FROM tblWorker "
        "WHERE id IN (" << swId << ")  AND isDelete = 0 ORDER BY id;";

  PGres pgr(PQexec(_pg, ss.str().c_str()));
  if (PQresultStatus(pgr.res) != PGRES_TUPLES_OK){
    errorMess(string("workerState error: ") + PQerrorMessage(_pg));
    return false;
  }
  size_t wsz = wId.size();
  if (PQntuples(pgr.res) != wsz){
    errorMess("workerState error: PQntuples(pgr.res) != wsz");
    return false;
  }
  out.resize(wsz);
  for (size_t i = 0; i < wsz; ++i){
    out[i].state = (ZM_Base::StateType)atoi(PQgetvalue(pgr.res, (int)i, 0));
    out[i].activeTask = atoi(PQgetvalue(pgr.res, (int)i, 1));
    out[i].load = atoi(PQgetvalue(pgr.res, (int)i, 2));
    out[i].startTime = PQgetvalue(pgr.res, (int)i, 3);
    out[i].stopTime = PQgetvalue(pgr.res, (int)i, 4);
  }
  return true;
}
std::vector<uint64_t> DbProvider::getAllWorkers(uint64_t sId, ZM_Base::StateType state){
  lock_guard<mutex> lk(m_impl->m_mtx);
  stringstream ss;
  ss << "SELECT id FROM tblWorker "
        "WHERE (state = " << (int)state << " OR " << (int)state << " = -1) "
        "AND schedr = " << sId << " "
        "AND isDelete = 0;";

  PGres pgr(PQexec(_pg, ss.str().c_str()));
  if (PQresultStatus(pgr.res) != PGRES_TUPLES_OK){
    errorMess(string("getAllWorkers error: ") + PQerrorMessage(_pg));
    return std::vector<uint64_t>();
  }  
  int rows = PQntuples(pgr.res);
  std::vector<uint64_t> ret(rows);
  for (int i = 0; i < rows; ++i){
    ret[i] = stoull(PQgetvalue(pgr.res, i, 0));
  }
  return ret;
}
}