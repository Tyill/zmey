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

#pragma once

#include "db_provider/db_provider.h"
#include "common/signal_connector.h"

class Application{
public:
 
  struct Config{
    int checkWorkerTOutSec = 60;
    int pingToDBSec = 20;
    std::string localConnPnt;
    std::string remoteConnPnt;
    DB::ConnectCng dbConnCng;
  };

  enum Signals{
    SIGNAL_LOOP_NOTIFY = 0,
    SIGNAL_LOOP_STOP,
  };
  static Aux::SignalConnector SignalConnector;

  static void loopNotify();
  static void loopStop();
 
  void statusMess(const std::string& mess);

  bool parseArgs(int argc, char* argv[], Config& outCng); 
      
private:
  std::mutex m_mtxStatusMess;
  
  
};