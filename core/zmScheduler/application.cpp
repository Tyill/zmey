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
#include "application.h"
#include "zmCommon/aux_func.h"

#include <iostream>

using namespace std;

ZM_Aux::SignalConnector Application::SignalConnector;

void Application::statusMess(const string& mess){
  lock_guard<mutex> lock(m_mtxStatusMess);
  cout << ZM_Aux::currDateTimeMs() << " " << mess << std::endl;
}

bool Application::parseArgs(int argc, char* argv[], Config& outCng){ 
  
  map<string, string> sprms = ZM_Aux::parseCMDArgs(argc, argv);

  if (sprms.empty() || (sprms.cbegin()->first == "help")){
    cout << "Usage: --localAddr[-la] schedr local connection point: IP or DNS:port. Required\n"
         << "       --remoteAddr[-ra] schedr remote connection point (if from NAT): IP or DNS:port. Optional\n"
         << "       --dbConnStr[-db] database connection string\n"
         << "       --checkWorkerTOut[-cw] check ping from workers, sec. Default 120 sec\n";
    return false; 
  }
  
#define SET_PARAM(shortName, longName, prm)        \
  if (sprms.find(#longName) != sprms.end()){       \
    outCng.prm = sprms[#longName];                 \
  }                                                \
  else if (sprms.find(#shortName) != sprms.end()){ \
    outCng.prm = sprms[#shortName];                \
  }

  SET_PARAM(la, localAddr, localConnPnt); 
  SET_PARAM(ra, remoteAddr, remoteConnPnt);
  SET_PARAM(db, dbConnStr, dbConnCng.connectStr);
 
#define SET_PARAM_NUM(shortName, longName, prm)                                           \
  if (sprms.find(#longName) != sprms.end() && ZM_Aux::isNumber(sprms[#longName])){        \
    outCng.prm = stoi(sprms[#longName]);                                                  \
  }                                                                                       \
  else if (sprms.find(#shortName) != sprms.end() && ZM_Aux::isNumber(sprms[#shortName])){ \
    outCng.prm = stoi(sprms[#shortName]);                                                 \
  }

  SET_PARAM_NUM(cw, checkWorkerTOut, checkWorkerTOutSec);

  return true;
}

void Application::loopNotify()
{
  Application::SignalConnector.emitSignal(Signals::SIGNAL_LOOP_NOTIFY);
}

void Application::loopStop()
{
  Application::SignalConnector.emitSignalWithoutBlocking(Signals::SIGNAL_LOOP_STOP);
}