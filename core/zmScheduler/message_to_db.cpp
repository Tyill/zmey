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
#include "zmDbProvider/db_provider.h"
#include "zmCommon/queue.h"
#include "zmCommon/aux_func.h"
#include "structurs.h"

using namespace std;

static ZM_Aux::CounterTick m_ctickAD;
extern ZM_Aux::Queue<ZM_DB::MessSchedr> g_messToDB;
extern ZM_Base::Scheduler g_schedr;

void sendAllMessToDB(ZM_DB::DbProvider& db){

  vector<ZM_DB::MessSchedr> mess;
  ZM_DB::MessSchedr m;
  while(g_messToDB.tryPop(m)){
    mess.push_back(m);
  }
  if (!db.sendAllMessFromSchedr(g_schedr.id, mess)){
    for (auto& m : mess){
      g_messToDB.push(move(m));
    }
    if (m_ctickAD(100)){ // every 100 cycle
      statusMess("sendAllMessToDB db error: " + db.getLastError());
    }
  }else{
    m_ctickAD.reset();
  }
}