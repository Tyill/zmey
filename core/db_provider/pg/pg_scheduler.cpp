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

bool DbProvider::addSchedr(const Base::Scheduler& schedl, int& outSchId){    
  lock_guard<mutex> lk(m_impl->m_mtx);
  
  stringstream ss;
  ss << "INSERT INTO tblScheduler (connPnt, state, capacityTask, name, description) VALUES("
        "'" << schedl.connectPnt << "',"
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
bool DbProvider::getSchedr(int sId, Base::Scheduler& cng){
  lock_guard<mutex> lk(m_impl->m_mtx);
  stringstream ss;
  ss << "SELECT connPnt, state, capacityTask, name, description "
        "FROM tblScheduler "
        "WHERE id = " << sId << " AND isDelete = 0;";

  PGres pgr(PQexec(_pg, ss.str().c_str()));
  if (PQresultStatus(pgr.res) != PGRES_TUPLES_OK){
    errorMess(string("getSchedr: ") + PQerrorMessage(_pg));
    return false;
  }
  if (PQntuples(pgr.res) != 1){
    errorMess(string("getSchedr error: such schedr does not exist"));
    return false;
  }
  cng.connectPnt = PQgetvalue(pgr.res, 0, 0);
  cng.state = (Base::StateType)atoi(PQgetvalue(pgr.res, 0, 1));
  cng.capacityTask = atoi(PQgetvalue(pgr.res, 0, 2));
  cng.name = PQgetvalue(pgr.res, 0, 3);
  cng.description = PQgetvalue(pgr.res, 0, 4);
  return true;
}
bool DbProvider::changeSchedr(int sId, const Base::Scheduler& newCng){  
  lock_guard<mutex> lk(m_impl->m_mtx);
 
  stringstream ss;
  ss << "UPDATE tblScheduler SET "
        "capacityTask = '" << newCng.capacityTask << "', "
        "connPnt = '" << newCng.connectPnt << "', "
        "name = '" << newCng.name << "', "
        "description = '" << newCng.description << "' "
        "WHERE id = " << sId << " AND isDelete = 0;";
      
  PGres pgr(PQexec(_pg, ss.str().c_str()));
  if (PQresultStatus(pgr.res) != PGRES_COMMAND_OK){
    errorMess(string("changeSchedr: ") + PQerrorMessage(_pg));
    return false;
  }  
  return true;
}
bool DbProvider::delSchedr(int sId){  
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

bool DbProvider::schedrState(int sId, SchedulerState& out){
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
  out.state = (Base::StateType)atoi(PQgetvalue(pgr.res, 0, 0));
  out.activeTask = atoi(PQgetvalue(pgr.res, 0, 1));
  out.startTime = PQgetvalue(pgr.res, 0, 2);
  out.stopTime = PQgetvalue(pgr.res, 0, 3);
  out.pingTime = PQgetvalue(pgr.res, 0, 4);
  return true;
}
std::vector<int> DbProvider::getAllSchedrs(Base::StateType state){  
  lock_guard<mutex> lk(m_impl->m_mtx);
  stringstream ss;
  ss << "SELECT id FROM tblScheduler "
        "WHERE (state = " << (int)state << " OR " << (int)state << " = -1) AND isDelete = 0;";

  PGres pgr(PQexec(_pg, ss.str().c_str()));
  if (PQresultStatus(pgr.res) != PGRES_TUPLES_OK){
    errorMess(string("getAllSchedrs: ") + PQerrorMessage(_pg));
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