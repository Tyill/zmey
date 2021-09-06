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

#include <chrono>

using namespace std;

namespace ZM_DB{

bool DbProvider::setChangeTaskStateCBack(uint64_t tId, uint64_t userId, ChangeTaskStateCBack cback, UData ud){
  if (!cback)
    return false;
  {
    lock_guard<mutex> lk(m_impl->m_mtxNotifyTask);  
    m_impl->m_notifyTaskStateCBack[tId] = {ZM_Base::StateType::UNDEFINED, 0, cback, userId, ud };
    if (m_impl->m_notifyTaskStateCBack.size() == 1)
      m_impl->m_cvNotifyTask.notify_one();
  }  
  if (!m_impl->m_thrEndTask.joinable()){
    m_impl->m_thrEndTask = thread([this](){
      while (!m_impl->m_fClose){
        std::map<uint64_t, DbProvider::Impl::NotifyTaskStateCBack> notifyTasks;
        {
          std::unique_lock<std::mutex> lk(m_impl->m_mtxNotifyTask);
          if (m_impl->m_notifyTaskStateCBack.empty())
            m_impl->m_cvNotifyTask.wait(lk); 
          notifyTasks = m_impl->m_notifyTaskStateCBack;
        }
        string stId;
        stId = accumulate(notifyTasks.begin(), notifyTasks.end(), stId,
                  [](string& s, pair<uint64_t, DbProvider::Impl::NotifyTaskStateCBack> v){
                    return s.empty() ? to_string(v.first) : s + "," + to_string(v.first);
                  });       
        stringstream ss;
        ss << "SELECT qtask, state, progress "
              "FROM tblTaskState ts "
              "WHERE qtask IN (" << stId << ");"; 
        
        auto t_start = std::chrono::high_resolution_clock::now();

        struct TState{
          uint64_t id;
          ZM_Base::StateType state;
          int progress;
        };
        vector<TState> notifyRes;
        { 
          lock_guard<mutex> lk(m_impl->m_mtx);
          PGres pgr(PQexec(_pg, ss.str().c_str()));
          if (PQresultStatus(pgr.res) == PGRES_TUPLES_OK){
            size_t tsz = PQntuples(pgr.res);
            for (size_t i = 0; i < tsz; ++i){
              uint64_t tId = stoull(PQgetvalue(pgr.res, (int)i, 0));
              ZM_Base::StateType state = (ZM_Base::StateType)atoi(PQgetvalue(pgr.res, (int)i, 1));
              int progress = atoi(PQgetvalue(pgr.res, (int)i, 2));
              if ((state != notifyTasks[tId].state) || (progress != notifyTasks[tId].progress)){
                notifyRes.push_back(TState{tId, state, progress});
              }
            }
          }else{
            errorMess(string("endTaskCBack error: ") + PQerrorMessage(_pg));
          } 
        }
        if (!notifyRes.empty()){
          for (auto& t : notifyRes){
            ZM_Base::StateType prevState = notifyTasks[t.id].state,
                               newState = t.state;
            notifyTasks[t.id].cback(t.id, notifyTasks[t.id].userId, t.progress, prevState, newState, notifyTasks[t.id].ud);
          }
          { 
            lock_guard<mutex> lk(m_impl->m_mtxNotifyTask);  
            for (auto& t : notifyRes){
              ZM_Base::StateType newState = t.state;
              if ((newState == ZM_Base::StateType::COMPLETED) || (newState == ZM_Base::StateType::ERRORT) || (newState == ZM_Base::StateType::CANCEL)){
                m_impl->m_notifyTaskStateCBack.erase(t.id);
              }else{
                m_impl->m_notifyTaskStateCBack[t.id].state = newState;
                m_impl->m_notifyTaskStateCBack[t.id].progress = t.progress;
              }
            }    
          }
        }
        auto t_end = std::chrono::high_resolution_clock::now();
        int deltaTimeMs = (int)std::chrono::duration<double, std::milli>(t_end - t_start).count();
        int maxElapseTimeMS = 10;
        if ((maxElapseTimeMS - deltaTimeMs) > 0)
          ZM_Aux::sleepMs(maxElapseTimeMS - deltaTimeMs);
      }      
    });
  }
  return true;
}
}
