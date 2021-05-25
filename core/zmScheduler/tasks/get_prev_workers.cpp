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
#include "zmScheduler/executor.h"
#include "zmCommon/tcp.h"
#include "zmCommon/json.h"

using namespace std;

void Executor::getPrevWorkersFromDB(ZM_DB::DbProvider& db)
{   
  vector<ZM_Base::Worker> workers; 
  if (db.getWorkersOfSchedr(m_schedr.id, workers)){
    for(auto& w : workers){
      m_workers[w.connectPnt] = SWorker{w, w.state, vector<uint64_t>(int(w.capacityTask * 1.5), 0), 
                                        w.state != ZM_Base::StateType::NOT_RESPONDING};
    }

    Json::Reader readerJs;
    Json::Value obj;
    readerJs.parse(m_schedr.internalData, obj); 

    map<string, vector<uint64_t>> workersCng;
    if (obj.isObject() && obj.isMember("Workers") && obj["Workers"].isArray()){
      Json::Value workersJs = obj["Workers"];
      for (const auto& w : workersJs){
        if (w.isMember("ConnPnt") && w.isMember("Tasks") && w["Tasks"].isArray()){
          string connPnt = w["ConnPnt"].asString();
          workersCng[connPnt];
          for (auto& t : w["Tasks"]){
            if (t.isUInt64())
              workersCng[connPnt].push_back(t.asUInt64());
          }
        }
      }
    }

    for (const auto& wcng : workersCng){
      if (m_workers.count(wcng.first)){
        for (size_t i = 0; (i < m_workers[wcng.first].taskList.size()) && (i < wcng.second.size()) ; ++i)
          m_workers[wcng.first].taskList[i] = wcng.second[i];
      }
    }
  }
  else{
    m_app.statusMess("getPrevWorkersFromDB db error: " + db.getLastError());
  }  
}