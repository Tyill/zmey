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

bool DbProvider::addSchedr(const ZM_Base::Scheduler& schedl, uint64_t& outSchId){    
  lock_guard<mutex> lk(m_impl->m_mtx);
  auto connPnt = ZM_Aux::split(schedl.connectPnt, ':');
  if (connPnt.size() != 2){
    errorMess("addSchedr error: connectPnt not correct");
    return false;
  }
  stringstream ss;
  ss << "WITH ncp AS (INSERT INTO tblConnectPnt (ipAddr, port) VALUES("
        " '" << connPnt[0] << "',"
        " '" << connPnt[1] << "') RETURNING id)"
        "INSERT INTO tblScheduler (connPnt, state, capacityTask, name, description) VALUES("
        "(SELECT id FROM ncp),"
        "'" << (int)schedl.state << "',"
        "'" << schedl.capacityTask << "',"
        "'" << schedl.name << "',"
        "'" << schedl.description << "') RETURNING id;";

  PGres pgr(PQexec(_pg, ss.str().c_str()));
  if (PQresultStatus(pgr.res) != PGRES_TUPLES_OK){
    errorMess(string("addSchedr: ") + PQerrorMessage(_pg));
    return false;
  }
  outSchId = stoull(PQgetvalue(pgr.res, 0, 0));
  return true;
}
bool DbProvider::getSchedr(uint64_t sId, ZM_Base::Scheduler& cng){
  lock_guard<mutex> lk(m_impl->m_mtx);
  stringstream ss;
  ss << "SELECT cp.ipAddr, cp.port, s.state, s.capacityTask, s.name, s.description FROM tblScheduler s "
        "JOIN tblConnectPnt cp ON cp.id = s.connPnt "
        "WHERE s.id = " << sId << " AND s.isDelete = 0;";

  PGres pgr(PQexec(_pg, ss.str().c_str()));
  if (PQresultStatus(pgr.res) != PGRES_TUPLES_OK){
    errorMess(string("getSchedr: ") + PQerrorMessage(_pg));
    return false;
  }
  if (PQntuples(pgr.res) != 1){
    errorMess(string("getSchedr error: such schedr does not exist"));
    return false;
  }
  cng.connectPnt = PQgetvalue(pgr.res, 0, 0) + string(":") + PQgetvalue(pgr.res, 0, 1);
  cng.state = (ZM_Base::StateType)atoi(PQgetvalue(pgr.res, 0, 2));
  cng.capacityTask = atoi(PQgetvalue(pgr.res, 0, 3));
  cng.name = PQgetvalue(pgr.res, 0, 4);
  cng.description = PQgetvalue(pgr.res, 0, 5);
  return true;
}
bool DbProvider::changeSchedr(uint64_t sId, const ZM_Base::Scheduler& newCng){  
  lock_guard<mutex> lk(m_impl->m_mtx);
  auto connPnt = ZM_Aux::split(newCng.connectPnt, ':');
  if (connPnt.size() != 2){
    errorMess("changeSchedr error: connectPnt not correct");
    return false;
  }
  stringstream ss;
  ss << "UPDATE tblScheduler SET "
        "capacityTask = '" << newCng.capacityTask << "', "
        "name = '" << newCng.name << "', "
        "description = '" << newCng.description << "' "
        "WHERE id = " << sId << " AND isDelete = 0; "

        "UPDATE tblConnectPnt SET "
        "ipAddr = '" << connPnt[0] << "',"
        "port = '" << connPnt[1] << "' "
        "WHERE id = (SELECT connPnt FROM tblScheduler WHERE id = " << sId << " AND isDelete = 0);";

  PGres pgr(PQexec(_pg, ss.str().c_str()));
  if (PQresultStatus(pgr.res) != PGRES_COMMAND_OK){
    errorMess(string("changeSchedr: ") + PQerrorMessage(_pg));
    return false;
  }  
  return true;
}
bool DbProvider::delSchedr(uint64_t sId){  
  lock_guard<mutex> lk(m_impl->m_mtx);
  stringstream ss;
  ss << "UPDATE tblScheduler SET "
        "isDelete = 1 "
        "WHERE id = " << sId << ";";

  PGres pgr(PQexec(_pg, ss.str().c_str()));
  if (PQresultStatus(pgr.res) != PGRES_COMMAND_OK){
    errorMess(string("delSchedr: ") + PQerrorMessage(_pg));
    return false;
  }  
  return true;
}

bool DbProvider::schedrState(uint64_t sId, SchedulerState& out){
  lock_guard<mutex> lk(m_impl->m_mtx);
  stringstream ss;
  ss << "SELECT state, activeTask, startTime, stopTime, pingTime "
        "FROM tblScheduler "
        "WHERE id = " << sId << " AND isDelete = 0;";

  PGres pgr(PQexec(_pg, ss.str().c_str()));
  if ((PQresultStatus(pgr.res) != PGRES_TUPLES_OK) || (PQntuples(pgr.res) != 1)){
    errorMess(string("schedrState: ") + PQerrorMessage(_pg));
    return false;
  }
  out.state = (ZM_Base::StateType)atoi(PQgetvalue(pgr.res, 0, 0));
  out.activeTask = atoi(PQgetvalue(pgr.res, 0, 1));
  out.startTime = PQgetvalue(pgr.res, 0, 2);
  out.stopTime = PQgetvalue(pgr.res, 0, 3);
  out.pingTime = PQgetvalue(pgr.res, 0, 4);
  return true;
}
std::vector<uint64_t> DbProvider::getAllSchedrs(ZM_Base::StateType state){  
  lock_guard<mutex> lk(m_impl->m_mtx);
  stringstream ss;
  ss << "SELECT id FROM tblScheduler "
        "WHERE (state = " << (int)state << " OR " << (int)state << " = -1) AND isDelete = 0;";

  PGres pgr(PQexec(_pg, ss.str().c_str()));
  if (PQresultStatus(pgr.res) != PGRES_TUPLES_OK){
    errorMess(string("getAllSchedrs: ") + PQerrorMessage(_pg));
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