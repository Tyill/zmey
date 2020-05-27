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
#include <unordered_map>
#include "zmBase/structurs.h"
#include "zmDbProvider/dbProvider.h"
#include "zmCommon/queue.h"
#include "zmCommon/tcp.h"
#include "zmCommon/serial.h"

using namespace std;

void checkStatusWorkers(const ZM_Base::scheduler& schedr,
                        unordered_map<std::string, ZM_Base::worker>& workers,
                        ZM_Aux::QueueThrSave<ZM_DB::messSchedr>& messToDB){
   
  for(auto& w : workers){
    if (!w.second.isActive && (w.second.ste == ZM_Base::state::running)){            
      messToDB.push(ZM_DB::messSchedr{ZM_Base::messType::workerNotResponding,
                                      w.second.id});
      w.second.ste = ZM_Base::state::notResponding;
      w.second.activeTask = 0;
    }else{
      w.second.isActive = false;
    }
  }

  // all inactive
  vector<ZM_Base::worker> wkrNotResp;
  for(auto& w : workers){
    if (w.second.ste == ZM_Base::state::notResponding){
      wkrNotResp.push_back(w.second);
    }
  }
  if (wkrNotResp.empty())
    return; 

  // check max10% inactive
  int cnt = 0,
      maxCnt = max(1, int(wkrNotResp.size() * 0.1));
  while(true){   
    int i = rand() % wkrNotResp.size(); 
    map<string, string> sendData{
      make_pair("command", to_string((int)ZM_Base::messType::pingWorker))
    };      
    ZM_Tcp::sendData(wkrNotResp[i].connectPnt, ZM_Aux::serialn(sendData));
    ++cnt;
    if (cnt == maxCnt)
      break;
  }
}