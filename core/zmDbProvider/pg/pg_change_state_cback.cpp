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

bool DbProvider::setChangeTaskStateCBack(uint64_t tId, ChangeTaskStateCBack cback, UData ud){
  if (!cback)
    return false;
  {
    lock_guard<mutex> lk(m_impl->m_mtxNotifyTask);  
    m_impl->m_notifyTaskStateCBack[tId] = {ZM_Base::StateType::UNDEFINED, cback, ud };
    if (m_impl->m_notifyTaskStateCBack.size() == 1)
      m_impl->m_cvNotifyTask.notify_one();
  }  
  if (!m_impl->m_thrEndTask.joinable()){
    m_impl->m_thrEndTask = thread([this](){
      while (!m_impl->m_fClose){
        if (m_impl->m_notifyTaskStateCBack.empty()){
          std::unique_lock<std::mutex> lk(m_impl->m_mtxNotifyTask);
          if (m_impl->m_notifyTaskStateCBack.empty())
            m_impl->m_cvNotifyTask.wait(lk); 
        }
        std::map<uint64_t, DbProvider::Impl::NotifyTaskStateCBack> notifyTasks;
        {
          lock_guard<mutex> lk(m_impl->m_mtxNotifyTask); 
          notifyTasks = m_impl->m_notifyTaskStateCBack;
        } 
        string stId;
        stId = accumulate(notifyTasks.begin(), notifyTasks.end(), stId,
                  [](string& s, pair<uint64_t, DbProvider::Impl::NotifyTaskStateCBack> v){
                    return s.empty() ? to_string(v.first) : s + "," + to_string(v.first);
                  });       
        stringstream ss;
        ss << "SELECT qt.id, ts.state "
              "FROM tblTaskState ts "
              "JOIN tblTaskQueue qt ON qt.id = ts.qtask "
              "WHERE qt.id IN (" << stId << ");"; 
        
        vector<pair<uint64_t, ZM_Base::StateType>> notifyRes;
        { 
          lock_guard<mutex> lk(m_impl->m_mtx);
          PGres pgr(PQexec(_pg, ss.str().c_str()));
          if (PQresultStatus(pgr.res) == PGRES_TUPLES_OK){
            size_t tsz = PQntuples(pgr.res);
            for (size_t i = 0; i < tsz; ++i){
              uint64_t tId = stoull(PQgetvalue(pgr.res, (int)i, 0));
              ZM_Base::StateType state = (ZM_Base::StateType)atoi(PQgetvalue(pgr.res, (int)i, 1));
              if (state != notifyTasks[tId].state){
                notifyRes.push_back(make_pair(tId, state));
              }
            }
          }else{
            errorMess(string("endTaskCBack error: ") + PQerrorMessage(_pg));
          } 
        }
        if (!notifyRes.empty()){
          for (auto& t : notifyRes){
            uint64_t tId = t.first;
            ZM_Base::StateType prevState = notifyTasks[tId].state,
                               newState = t.second;
            notifyTasks[tId].cback(tId, prevState, newState, notifyTasks[tId].ud);
          }
          { 
            lock_guard<mutex> lk(m_impl->m_mtxNotifyTask);  
            for (auto& t : notifyRes){
              uint64_t tId = t.first;
              ZM_Base::StateType newState = t.second;
              if ((newState == ZM_Base::StateType::COMPLETED) || (newState == ZM_Base::StateType::ERROR) || (newState == ZM_Base::StateType::CANCEL)){
                m_impl->m_notifyTaskStateCBack.erase(tId);
              }else{
                m_impl->m_notifyTaskStateCBack[tId].state = newState;
              }
            }    
          }
        }
      }      
    });
  }
  return true;
}
}