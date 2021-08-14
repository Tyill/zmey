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

bool DbProvider::addPipelineTask(const ZM_Base::UPipelineTask& cng, uint64_t& outTId){
  lock_guard<mutex> lk(m_impl->m_mtx);
  
  stringstream ss;
  ss << "INSERT INTO tblUPipelineTask (pipeline, taskTempl, taskGroup, name, description) VALUES("
        "'" << cng.pplId << "',"
        "'" << cng.ttId << "',"
        "NULLIF(" << cng.gId << ", 0),"
        "'" << cng.name << "',"
        "'" << cng.description<< "') RETURNING id;";

  PGres pgr(PQexec(_pg, ss.str().c_str()));
  if (PQresultStatus(pgr.res) != PGRES_TUPLES_OK){
    errorMess(string("addPipelineTask error: ") + PQerrorMessage(_pg));
    return false;
  }
  outTId = stoull(PQgetvalue(pgr.res, 0, 0));
  return true;
}
bool DbProvider::getPipelineTask(uint64_t tId, ZM_Base::UPipelineTask& outTCng){
  lock_guard<mutex> lk(m_impl->m_mtx);
  stringstream ss;
  ss << "SELECT pipeline, COALESCE(taskGroup, 0), taskTempl, name, description "
        "FROM tblUPipelineTask "
        "WHERE id = " << tId << " AND isDelete = 0;";

  PGres pgr(PQexec(_pg, ss.str().c_str()));
  if (PQresultStatus(pgr.res) != PGRES_TUPLES_OK){
    errorMess(string("getPipelineTask error: ") + PQerrorMessage(_pg));
    return false;
  }
  if (PQntuples(pgr.res) != 1){
    errorMess(string("getPipelineTask error: such task does not exist"));
    return false;
  }
  outTCng.pplId = stoull(PQgetvalue(pgr.res, 0, 0));
  outTCng.gId = stoull(PQgetvalue(pgr.res, 0, 1));
  outTCng.ttId = stoull(PQgetvalue(pgr.res, 0, 2));
  outTCng.name = PQgetvalue(pgr.res, 0, 3);
  outTCng.description = PQgetvalue(pgr.res, 0, 4);
  return true;
}
bool DbProvider::changePipelineTask(uint64_t tId, const ZM_Base::UPipelineTask& newCng){
  lock_guard<mutex> lk(m_impl->m_mtx);
 
  stringstream ss;
  ss << "UPDATE tblUPipelineTask SET "
        "pipeline = '" << newCng.pplId << "',"
        "taskTempl = '" << newCng.ttId << "',"
        "taskGroup = NULLIF(" << newCng.gId << ", 0),"
        "name = '" << newCng.name << "',"
        "description = '" << newCng.description << "' "   
        "WHERE id = " << tId << " AND isDelete = 0;";
          
  PGres pgr(PQexec(_pg, ss.str().c_str()));
  if (PQresultStatus(pgr.res) != PGRES_COMMAND_OK){
    errorMess(string("changePipelineTask error: ") + PQerrorMessage(_pg));
    return false;
  }  
  return true;
}
bool DbProvider::delPipelineTask(uint64_t tId){
  lock_guard<mutex> lk(m_impl->m_mtx);
  stringstream ss;
  ss << "UPDATE tblUPipelineTask SET "
        "isDelete = 1 "
        "WHERE id = " << tId << ";";

  PGres pgr(PQexec(_pg, ss.str().c_str()));
  if (PQresultStatus(pgr.res) != PGRES_COMMAND_OK){
    errorMess(string("delPipelineTask error: ") + PQerrorMessage(_pg));
    return false;
  }  
  return true;
}
std::vector<uint64_t> DbProvider::getAllPipelineTasks(uint64_t pplId){  
  lock_guard<mutex> lk(m_impl->m_mtx);
  stringstream ss;
  ss << "SELECT ut.id FROM tblUPipelineTask ut "
        "WHERE ut.isDelete = 0 AND ut.pipeline = " << pplId << ";";

  PGres pgr(PQexec(_pg, ss.str().c_str()));
  if (PQresultStatus(pgr.res) != PGRES_TUPLES_OK){
    errorMess(string("getAllPipelineTasks error: ") + PQerrorMessage(_pg));
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