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

bool DbProvider::startTask(uint64_t ttlId, const std::string& inparams, uint64_t& tId){
  lock_guard<mutex> lk(m_impl->m_mtx);  
  
  stringstream ss;
  ss << "SELECT * FROM funcStartTask(" << ttlId << ",'" << inparams << "');";

  PGres pgr(PQexec(_pg, ss.str().c_str()));
  if (PQresultStatus(pgr.res) != PGRES_TUPLES_OK){
    errorMess(string("startTask error: ") + PQerrorMessage(_pg));
    return false;
  }
  tId = stoull(PQgetvalue(pgr.res, 0, 0));
  if (tId == 0){
    errorMess("startTask error: such task does not exist");
    return false;
  }  
  return true;
}
bool DbProvider::cancelTask(uint64_t tId){
  lock_guard<mutex> lk(m_impl->m_mtx);
  stringstream ss;
  ss << "UPDATE tblTaskState ts SET "
        "state = " << int(ZM_Base::StateType::CANCEL) << " "
        "FROM tblTaskQueue tq "
        "WHERE tq.id = " << tId << " AND "
        "      ts.qtask = tq.id AND "
        "      ts.state = " << int(ZM_Base::StateType::READY) << " "
        "RETURNING ts.qtask;";
        
  PGres pgr(PQexec(_pg, ss.str().c_str()));
  if (PQresultStatus(pgr.res) != PGRES_TUPLES_OK){
    errorMess(string("cancelTask error: ") + PQerrorMessage(_pg));
    return false;
  }  
  if (PQntuples(pgr.res) != 1){
    errorMess(string("cancelTask error: task already take in work") + PQerrorMessage(_pg));
    return false;
  }
  return true;
}
bool DbProvider::taskState(const std::vector<uint64_t>& tId, std::vector<ZM_DB::TaskState>& outState){
  lock_guard<mutex> lk(m_impl->m_mtx);
  string stId;
  stId = accumulate(tId.begin(), tId.end(), stId,
                [](string& s, uint64_t v){
                  return s.empty() ? to_string(v) : s + "," + to_string(v);
                }); 
  stringstream ss;
  ss << "SELECT ts.state, ts.progress "
        "FROM tblTaskState ts "
        "WHERE ts.qtask IN (" << stId << ") ORDER BY ts.qtask;";

  PGres pgr(PQexec(_pg, ss.str().c_str()));
  if (PQresultStatus(pgr.res) != PGRES_TUPLES_OK){
    errorMess(string("taskState error: ") + PQerrorMessage(_pg));
    return false;
  }
  size_t tsz = tId.size();
  if (PQntuples(pgr.res) != tsz){      
    errorMess("taskState error: PQntuples(pgr.res) != tsz");
    return false;
  }
  outState.resize(tsz);
  for (size_t i = 0; i < tsz; ++i){
    outState[i].state = (ZM_Base::StateType)atoi(PQgetvalue(pgr.res, (int)i, 0));
    outState[i].progress = atoi(PQgetvalue(pgr.res, (int)i, 1));
  }
  return true;
}
bool DbProvider::taskResult(uint64_t tId, std::string& out){
  lock_guard<mutex> lk(m_impl->m_mtx);
  stringstream ss;
  ss << "SELECT result FROM tblTaskResult "
        "WHERE qtask = " << tId << ";";

  PGres pgr(PQexec(_pg, ss.str().c_str()));
  if (PQresultStatus(pgr.res) != PGRES_TUPLES_OK){      
    errorMess(string("taskResult error: ") + PQerrorMessage(_pg));
    return false;
  } 
  if (PQntuples(pgr.res) != 1){
    errorMess("taskResult error: task delete OR not taken to work");
    return false;
  } 
  out = PQgetvalue(pgr.res, 0, 0);
  return true;
}
bool DbProvider::taskTime(uint64_t tId, ZM_DB::TaskTime& out){
  lock_guard<mutex> lk(m_impl->m_mtx);
  stringstream ss;
  ss << "SELECT createTime, takeInWorkTime, startTime, stopTime "
        "FROM tblTaskTime "
        "WHERE qtask = " << tId << ";";

  PGres pgr(PQexec(_pg, ss.str().c_str()));
  if (PQresultStatus(pgr.res) != PGRES_TUPLES_OK){      
    errorMess(string("taskTime error: ") + PQerrorMessage(_pg));
    return false;
  } 
  if (PQntuples(pgr.res) != 1){
    errorMess("taskTime error: task delete OR not taken to work");
    return false;
  } 
  out.createTime = PQgetvalue(pgr.res, 0, 0);
  out.takeInWorkTime = PQgetvalue(pgr.res, 0, 1);
  out.startTime = PQgetvalue(pgr.res, 0, 2);
  out.stopTime = PQgetvalue(pgr.res, 0, 3);
  return true;
}
}