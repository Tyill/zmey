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

bool DbProvider::addSchedr(const base::Scheduler& schedl, int& outSchId){    
  lock_guard<mutex> lk(m_impl->m_mtx);
  
  stringstream ss;
  ss << "INSERT INTO tblScheduler (connPnt, state, capacityTask) VALUES("
        "'" << schedl.sConnectPnt << "',"
        "'" << (int)schedl.sState << "',"
        "'" << schedl.sCapacityTaskCount << "') RETURNING id;";

  PGres pgr(PQexec(pg_, ss.str().c_str()));
  if (PQresultStatus(pgr.res) != PGRES_TUPLES_OK){
    errorMess(string("addSchedr: ") + PQerrorMessage(pg_));
    return false;
  }
  outSchId = stoi(PQgetvalue(pgr.res, 0, 0));
  return true;
}
bool DbProvider::getSchedr(int sId, base::Scheduler& cng){
  lock_guard<mutex> lk(m_impl->m_mtx);
  stringstream ss;
  ss << "SELECT connPnt, state, capacityTask "
        "FROM tblScheduler "
        "WHERE id = " << sId << " AND isDeleted = FALSE;";

  PGres pgr(PQexec(pg_, ss.str().c_str()));
  if (PQresultStatus(pgr.res) != PGRES_TUPLES_OK){
    errorMess(string("getSchedr: ") + PQerrorMessage(pg_));
    return false;
  }
  if (PQntuples(pgr.res) != 1){
    errorMess(string("getSchedr error: such schedr does not exist"));
    return false;
  }
  cng.sConnectPnt = PQgetvalue(pgr.res, 0, 0);
  cng.sState = base::StateType(atoi(PQgetvalue(pgr.res, 0, 1)));
  cng.sCapacityTaskCount = atoi(PQgetvalue(pgr.res, 0, 2));
  return true;
}
bool DbProvider::changeSchedr(int sId, const base::Scheduler& newCng){  
  lock_guard<mutex> lk(m_impl->m_mtx);
 
  stringstream ss;
  ss << "UPDATE tblScheduler SET "
        "capacityTask = '" << newCng.sCapacityTaskCount << "', "
        "connPnt = '" << newCng.sConnectPnt << "' "
        "WHERE id = " << sId << " AND isDeleted = FALSE;";
      
  PGres pgr(PQexec(pg_, ss.str().c_str()));
  if (PQresultStatus(pgr.res) != PGRES_COMMAND_OK){
    errorMess(string("changeSchedr: ") + PQerrorMessage(pg_));
    return false;
  }  
  return true;
}
bool DbProvider::delSchedr(int sId){  
  lock_guard<mutex> lk(m_impl->m_mtx);
  stringstream ss;
  ss << "UPDATE tblScheduler SET "
        "isDeleted = TRUE "
        "WHERE id = " << sId << ";";

  PGres pgr(PQexec(pg_, ss.str().c_str()));
  if (PQresultStatus(pgr.res) != PGRES_COMMAND_OK){
    errorMess(string("delSchedr: ") + PQerrorMessage(pg_));
    return false;
  }  
  return true;
}

bool DbProvider::schedrState(int sId, SchedulerState& out){
  lock_guard<mutex> lk(m_impl->m_mtx);
  stringstream ss;
  ss << "SELECT state, activeTask, startTime, stopTime, pingTime "
        "FROM tblScheduler "
        "WHERE id = " << sId << " AND isDeleted = FALSE;";

  PGres pgr(PQexec(pg_, ss.str().c_str()));
  if ((PQresultStatus(pgr.res) != PGRES_TUPLES_OK) || (PQntuples(pgr.res) != 1)){
    errorMess(string("schedrState: ") + PQerrorMessage(pg_));
    return false;
  }
  out.state = (base::StateType)atoi(PQgetvalue(pgr.res, 0, 0));
  out.activeTaskCount = atoi(PQgetvalue(pgr.res, 0, 1));
  out.startTime = PQgetvalue(pgr.res, 0, 2);
  out.stopTime = PQgetvalue(pgr.res, 0, 3);
  out.pingTime = PQgetvalue(pgr.res, 0, 4);
  return true;
}
std::vector<int> DbProvider::getAllSchedrs(base::StateType state){  
  lock_guard<mutex> lk(m_impl->m_mtx);
  stringstream ss;
  ss << "SELECT id FROM tblScheduler "
        "WHERE (state = " << (int)state << " OR " << (int)state << " = 0) AND isDeleted = FALSE;";

  PGres pgr(PQexec(pg_, ss.str().c_str()));
  if (PQresultStatus(pgr.res) != PGRES_TUPLES_OK){
    errorMess(string("getAllSchedrs: ") + PQerrorMessage(pg_));
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