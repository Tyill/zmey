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

bool DbProvider::addTaskTemplate(const ZM_Base::TaskTemplate& cng, uint64_t& outTId){
  lock_guard<mutex> lk(m_impl->m_mtx);
  stringstream ss;
  ss << "INSERT INTO tblTaskTemplate (usr, schedrPreset, workerPreset, name, description, script, averDurationSec, maxDurationSec) VALUES("
        "'" << cng.uId << "',"
        "NULLIF(" << cng.sId << ", 0),"
        "NULLIF(" << cng.wId << ", 0),"
        "'" << cng.name << "',"
        "'" << cng.description << "',"
        "'" << cng.script << "',"
        "'" << cng.averDurationSec << "',"
        "'" << cng.maxDurationSec << "') RETURNING id;";

  PGres pgr(PQexec(_pg, ss.str().c_str()));
  if (PQresultStatus(pgr.res) != PGRES_TUPLES_OK){
    errorMess(string("addTaskTemplate: ") + PQerrorMessage(_pg));
    return false;
  }
  outTId = stoull(PQgetvalue(pgr.res, 0, 0));
  return true;
}
bool DbProvider::getTaskTemplate(uint64_t tId, ZM_Base::TaskTemplate& outTCng){
  lock_guard<mutex> lk(m_impl->m_mtx);
  stringstream ss;
  ss << "SELECT usr, COALESCE(schedrPreset, 0), COALESCE(workerPreset, 0), name, description, script, averDurationSec, maxDurationSec "
        "FROM tblTaskTemplate "
        "WHERE id = " << tId << " AND isDelete = 0;";

  PGres pgr(PQexec(_pg, ss.str().c_str()));
  if (PQresultStatus(pgr.res) != PGRES_TUPLES_OK){
    errorMess(string("getTaskTemplate: ") + PQerrorMessage(_pg));
    return false;
  }
  if (PQntuples(pgr.res) != 1){
    errorMess(string("getTaskTemplate error: such taskTemplate does not exist"));
    return false;
  }
  outTCng.uId = stoull(PQgetvalue(pgr.res, 0, 0));
  outTCng.sId = stoull(PQgetvalue(pgr.res, 0, 1));
  outTCng.wId = stoull(PQgetvalue(pgr.res, 0, 2));
  outTCng.name = PQgetvalue(pgr.res, 0, 3);
  outTCng.description = PQgetvalue(pgr.res, 0, 4);
  outTCng.script = PQgetvalue(pgr.res, 0, 5);  
  outTCng.averDurationSec = atoi(PQgetvalue(pgr.res, 0, 6));
  outTCng.maxDurationSec = atoi(PQgetvalue(pgr.res, 0, 7));
  return true;
};
bool DbProvider::changeTaskTemplate(uint64_t tId, const ZM_Base::TaskTemplate& newTCng){
  lock_guard<mutex> lk(m_impl->m_mtx);
  stringstream ss;
  ss << "UPDATE tblTaskTemplate SET "
        "usr = '" << newTCng.uId << "',"
        "schedrPreset = NULLIF(" << newTCng.sId << ", 0),"
        "workerPreset = NULLIF(" << newTCng.wId << ", 0),"
        "name = '" << newTCng.name << "',"
        "description = '" << newTCng.description << "', "
        "script = '" << newTCng.script << "',"
        "averDurationSec = '" << newTCng.averDurationSec << "',"
        "maxDurationSec = '" << newTCng.maxDurationSec << "' "
        "WHERE id = " << tId << ";";
          
  PGres pgr(PQexec(_pg, ss.str().c_str()));
  if (PQresultStatus(pgr.res) != PGRES_COMMAND_OK){
    errorMess(string("changeTaskTemplate: ") + PQerrorMessage(_pg));
    return false;
  } 
  return true;
}
bool DbProvider::delTaskTemplate(uint64_t tId){
  lock_guard<mutex> lk(m_impl->m_mtx);
  stringstream ss;
  ss << "UPDATE tblTaskTemplate SET "
        "isDelete = 1 "
        "WHERE id = " << tId << ";";

  PGres pgr(PQexec(_pg, ss.str().c_str()));
  if (PQresultStatus(pgr.res) != PGRES_COMMAND_OK){
    errorMess(string("delTaskTemplate: ") + PQerrorMessage(_pg));
    return false;
  }  
  return true;
}
std::vector<uint64_t> DbProvider::getAllTaskTemplates(uint64_t usr){
  lock_guard<mutex> lk(m_impl->m_mtx);
  stringstream ss;
  ss << "SELECT id FROM tblTaskTemplate "
        "WHERE usr = " << usr << " AND isDelete = 0;";

  PGres pgr(PQexec(_pg, ss.str().c_str()));
  if (PQresultStatus(pgr.res) != PGRES_TUPLES_OK){
    errorMess(string("getAllTaskTemplates: ") + PQerrorMessage(_pg));
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