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

bool DbProvider::addGroup(const ZM_Base::UGroup& cng, uint64_t& outGId){
  lock_guard<mutex> lk(m_impl->m_mtx);
  stringstream ss;
  ss << "INSERT INTO tblUTaskGroup (pipeline, name, description) VALUES("
        "'" << cng.pplId << "',"
        "'" << cng.name << "',"
        "'" << cng.description << "') RETURNING id;";

  PGres pgr(PQexec(_pg, ss.str().c_str()));
  if (PQresultStatus(pgr.res) != PGRES_TUPLES_OK){
    errorMess(string("addGroup error: ") + PQerrorMessage(_pg));
    return false;
  }
  outGId = stoull(PQgetvalue(pgr.res, 0, 0));
  return true;
}
bool DbProvider::getGroup(uint64_t gId, ZM_Base::UGroup& cng){
  lock_guard<mutex> lk(m_impl->m_mtx);
  stringstream ss;
  ss << "SELECT pipeline, name, description FROM tblUTaskGroup "
        "WHERE id = " << gId << " AND isDelete = 0;";

  PGres pgr(PQexec(_pg, ss.str().c_str()));
  if (PQresultStatus(pgr.res) != PGRES_TUPLES_OK){
    errorMess(string("getGroup error: ") + PQerrorMessage(_pg));
    return false;
  }
  if (PQntuples(pgr.res) != 1){
    errorMess(string("getGroup error: such group does not exist"));
    return false;
  }
  cng.pplId = stoull(PQgetvalue(pgr.res, 0, 0));
  cng.name = PQgetvalue(pgr.res, 0, 1);
  cng.description = PQgetvalue(pgr.res, 0, 2);
  return true;
}
bool DbProvider::changeGroup(uint64_t gId, const ZM_Base::UGroup& newCng){
  lock_guard<mutex> lk(m_impl->m_mtx);
  stringstream ss;
  ss << "UPDATE tblUTaskGroup SET "
        "pipeline = '" << newCng.pplId << "',"
        "name = '" << newCng.name << "',"
        "description = '" << newCng.description << "' "
        "WHERE id = " << gId << " AND isDelete = 0;";

  PGres pgr(PQexec(_pg, ss.str().c_str()));
  if (PQresultStatus(pgr.res) != PGRES_COMMAND_OK){
    errorMess(string("changeGroup error: ") + PQerrorMessage(_pg));
    return false;
  }  
  return true;
}
bool DbProvider::delGroup(uint64_t gId){
  lock_guard<mutex> lk(m_impl->m_mtx);
  stringstream ss;
  ss << "UPDATE tblUTaskGroup SET "  
        "isDelete = 1 "
        "WHERE id = " << gId << ";";

  PGres pgr(PQexec(_pg, ss.str().c_str()));
  if (PQresultStatus(pgr.res) != PGRES_COMMAND_OK){
    errorMess(string("delGroup error: ") + PQerrorMessage(_pg));
    return false;
  }  
  return true;
}
std::vector<uint64_t> DbProvider::getAllGroups(uint64_t pplId){
  lock_guard<mutex> lk(m_impl->m_mtx);
  stringstream ss;
  ss << "SELECT id FROM tblUTaskGroup "
        "WHERE pipeline = " << pplId << " AND isDelete = 0;";

  PGres pgr(PQexec(_pg, ss.str().c_str()));
  if (PQresultStatus(pgr.res) != PGRES_TUPLES_OK){
    errorMess(string("getAllGroups error: ") + PQerrorMessage(_pg));
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