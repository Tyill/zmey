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
#include "common/aux_func.h"

#include <iostream>

using namespace std;

misc::SignalConnector Application::SignalConnector;

void Application::statusMess(const string& mess){
  lock_guard<mutex> lock(m_mtxStatusMess);
  cout << misc::currDateTimeMs() << " " << mess << std::endl;
}

bool Application::parseArgs(int argc, char* argv[], Config& outCng){
  
  map<string, string> sprms = misc::parseCMDArgs(argc, argv);
  
  if (sprms.empty() || (sprms.cbegin()->first == "help")){
    cout << "Usage: --localAddr[-la] worker local connection point: IP or DNS:port. Required\n"
         << "       --remoteAddr[-ra] worker remote connection point (if from NAT): IP or DNS:port. Optional\n"
         << "       --schedrAddr[-sa] schedr remote connection point: IP or DNS:port. Required\n"
         << "       --progressTasksTOutSec[-pt] send progress of tasks to schedr, sec. Default 10 sec\n"
         << "       --pingSchedrTOutSec[-st] send ping to schedr, sec. Default 20 sec\n";
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
  SET_PARAM(sa, schedrAddr, schedrConnPnt);

#define SET_PARAM_NUM(shortName, longName, prm)                                           \
  if (sprms.find(#longName) != sprms.end() && misc::isNumber(sprms[#longName])){           \
    outCng.prm = stoi(sprms[#longName]);                                                  \
  }                                                                                       \
  else if (sprms.find(#shortName) != sprms.end() && misc::isNumber(sprms[#shortName])){    \
    outCng.prm = stoi(sprms[#shortName]);                                                 \
  }

  SET_PARAM_NUM(pt, progressTasksTOutSec, progressTasksTOutSec);
  SET_PARAM_NUM(st, pingSchedrTOutSec, pingSchedrTOutSec);

  m_cng = outCng;

  return true;
}

void Application::loopNotify()
{
  Application::SignalConnector.emitSignalWithoutBlocking(Signals::SIGNAL_LOOP_NOTIFY);
}

void Application::loopStop()
{
  Application::SignalConnector.emitSignalWithoutBlocking(Signals::SIGNAL_LOOP_STOP);
}