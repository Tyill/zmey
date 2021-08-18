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

#include "zmCommon/aux_func.h"
#include "zmDbProvider/db_provider.h"

#include <libpq-fe.h>

#include <vector>
#include <numeric>
#include <sstream>  
#include <mutex>
#include <thread>
#include <condition_variable>

namespace ZM_DB{

class DbProvider::Impl{
public:
  PGconn* m_db = nullptr; 
  std::mutex m_mtx, m_mtxNotifyTask;
  std::condition_variable m_cvNotifyTask;
  std::thread m_thrEndTask;

  struct NotifyTaskStateCBack{
    ZM_Base::StateType state;
    ChangeTaskStateCBack cback;
    UData ud;
  };

  std::map<uint64_t, NotifyTaskStateCBack> m_notifyTaskStateCBack;
  bool m_fClose = false;
};

class PGres{
public:
  PGresult* res = nullptr;
  PGres(PGresult* _res):res(_res){}
  ~PGres(){
    if (res){
      PQclear(res);
    }
  }  
  PGres& operator=(PGres& npgr){
    if (res){
      PQclear(res);
    }
    res = npgr.res;
    npgr.res = nullptr;
    return *this;
  }
  PGres& operator=(PGres&& npgr){
    if (res){
      PQclear(res);
    }
    res = npgr.res;
    npgr.res = nullptr;
    return *this;
  }
};
}
#define _pg m_impl->m_db
