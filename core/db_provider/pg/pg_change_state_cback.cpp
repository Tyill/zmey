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

namespace DB{

bool DbProvider::setChangeTaskStateCBack(int tId, ChangeTaskStateCBack cback, UData ud){
  if (!cback)
    return false;
  {
    lock_guard<mutex> lk(m_impl->m_mtxNotifyTask);  
    m_impl->m_notifyTaskStateCBack[tId] = {base::StateType::UNDEFINED, cback, ud };
  }  
  if (!m_impl->m_thrEndTask.joinable()){
    m_impl->m_thrEndTask = thread([this](){
      
      auto cmd = "LISTEN " + m_impl->NOTIFY_NAME_CHANGE_TASK;
      PGres pgr(PQexec(pg_, cmd.c_str()));
      if (PQresultStatus(pgr.res) != PGRES_COMMAND_OK){
        errorMess(string("endTaskCBack LISTEN: ") + PQerrorMessage(pg_));
      }

      int maxElapseTimeMS = 10;
      while (!m_impl->m_fClose){
        
        PQconsumeInput(pg_);
        m_impl->m_notifyAuxCheckTOut.updateCycTime();
        
        bool isChangeState = false;
        PGnotify* notify = nullptr;
        while ((notify = PQnotifies(pg_)) != nullptr){
          isChangeState = std::string(notify->relname) == m_impl->NOTIFY_NAME_CHANGE_TASK;
          PQfreemem(notify);
        }
        bool auxCheckTimeout = m_impl->m_notifyAuxCheckTOut.onDelayOncSec(true, 10, 1);
        if (!isChangeState && m_impl->m_firstReqChangeTaskState && !auxCheckTimeout){
          misc::sleepMs(maxElapseTimeMS);
          continue;
        }
        m_impl->m_firstReqChangeTaskState = true;

        std::map<int, DbProvider::Impl::NotifyTaskStateCBack> notifyTasks;
        {
          lock_guard<mutex> lk(m_impl->m_mtxNotifyTask);          
          notifyTasks = m_impl->m_notifyTaskStateCBack;
        }
        if (notifyTasks.empty()){
          misc::sleepMs(maxElapseTimeMS);
          continue;
        }

        string stId;
        stId = accumulate(notifyTasks.begin(), notifyTasks.end(), stId,
                  [](string& s, pair<int, DbProvider::Impl::NotifyTaskStateCBack> v){
                    return s.empty() ? to_string(v.first) : s + "," + to_string(v.first);
                  });       
        stringstream ss;
        ss << "SELECT qtask, state "
              "FROM tblTaskState ts "
              "WHERE qtask IN (" << stId << ");"; 
        
        auto t_start = std::chrono::high_resolution_clock::now();

        struct TState{
          int id;
          base::StateType state;
        };
        vector<TState> notifyRes;
        { 
          lock_guard<mutex> lk(m_impl->m_mtx);
          PGres pgr(PQexec(pg_, ss.str().c_str()));
          if (PQresultStatus(pgr.res) == PGRES_TUPLES_OK){
            size_t tsz = PQntuples(pgr.res);
            for (size_t i = 0; i < tsz; ++i){
              int tId = stoi(PQgetvalue(pgr.res, (int)i, 0));
              base::StateType state = (base::StateType)atoi(PQgetvalue(pgr.res, (int)i, 1));
              if (state != notifyTasks[tId].state){
                notifyRes.push_back(TState{tId, state});
              }
            }
          }else{
            errorMess(string("endTaskCBack: ") + PQerrorMessage(pg_));
          } 
        }
        if (!notifyRes.empty()){
          for (auto& t : notifyRes){
            base::StateType prevState = notifyTasks[t.id].state,
                            newState = t.state;
            notifyTasks[t.id].cback(t.id, prevState, newState, notifyTasks[t.id].ud);
          }
          { 
            lock_guard<mutex> lk(m_impl->m_mtxNotifyTask);  
            for (auto& t : notifyRes){
              base::StateType newState = t.state;
              if ((newState == base::StateType::COMPLETED) || (newState == base::StateType::ERRORT) || (newState == base::StateType::CANCEL)){
                m_impl->m_notifyTaskStateCBack.erase(t.id);
              }else{
                m_impl->m_notifyTaskStateCBack[t.id].state = newState;
              }
            }    
          }
        }
        auto t_end = std::chrono::high_resolution_clock::now();
        int deltaTimeMs = (int)std::chrono::duration<double, std::milli>(t_end - t_start).count();
        if ((maxElapseTimeMS - deltaTimeMs) > 0)
          misc::sleepMs(maxElapseTimeMS - deltaTimeMs);
      }      
    });
  }
  return true;
}
}