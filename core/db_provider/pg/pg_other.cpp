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
  
bool DbProvider::getWorkerByTask(int tId, base::Worker& wcng){
  lock_guard<mutex> lk(m_impl->m_mtx);
  stringstream ss;
  ss << "SELECT wkr.id, wkr.connPnt "
        "FROM tblTaskQueue tq "
        "JOIN tblWorker wkr ON wkr.id = tq.worker "      
        "WHERE tq.id = " << tId << ";";

  PGres pgr(PQexec(pg_, ss.str().c_str()));
  if (PQresultStatus(pgr.res) != PGRES_TUPLES_OK){
    errorMess(string("getWorkerByTask: ") + PQerrorMessage(pg_));
    return false;
  }
  if (PQntuples(pgr.res) != 1){
    errorMess(string("getWorkerByTask error: task delete OR not taken to work"));
    return false;
  }
  wcng.wId = stoi(PQgetvalue(pgr.res, 0, 0));
  wcng.wConnectPnt = PQgetvalue(pgr.res, 0, 1);

  return true;
}

vector<db::MessError> DbProvider::getInternErrors(int sId, int wId, int mCnt){
  lock_guard<mutex> lk(m_impl->m_mtx);
  if (mCnt == 0){
    mCnt = INT32_MAX;
  }
  stringstream ss;
  ss << "SELECT schedr, worker, message, createTime "
        "FROM tblInternError "
        "WHERE (schedr = " << sId << " OR " << sId << " = 0)" << " AND "
        "      (worker = " << wId << " OR " << wId << " = 0) ORDER BY createTime LIMIT " << mCnt << ";";

  PGres pgr(PQexec(pg_, ss.str().c_str()));
  if (PQresultStatus(pgr.res) != PGRES_TUPLES_OK){
    errorMess(string("getInternErrors: ") + PQerrorMessage(pg_));
    return vector<db::MessError>();
  }  
  int rows = PQntuples(pgr.res);
  std::vector<db::MessError> ret(rows);
  for (int i = 0; i < rows; ++i){
    ret[i].schedrId = stoi(PQgetvalue(pgr.res, i, 0));
    ret[i].workerId = stoi(PQgetvalue(pgr.res, i, 1));
    ret[i].message = PQgetvalue(pgr.res, i, 2);
    ret[i].createTime = PQgetvalue(pgr.res, i, 3);
  }
  return ret;
}

}