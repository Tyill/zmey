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
        
#include <string>

namespace base{

// tblState
enum class StateType{
  UNDEFINED = 0,
  READY,    // for task: can be taken to work
  START,    // for task: taken to work
  RUNNING,  // for task: running
  PAUSE,
  STOP,    
  COMPLETED,
  ERRORT,
  CANCEL,
  NOT_RESPONDING,
};

// task object
struct Task{
  int tId{};                   // id tblTaskQueue
  int tWId{};                  // preset worker id tblWorker. Default 0 - not set
  int tAverDurationSec{};      // estimated lead time, sec 
  int tMaxDurationSec{};       // maximum lead time, sec
  std::string tParams;
  std::string tScriptPath;  
  std::string tResultPath;  
  StateType tState{};
};  

// tblScheduler
struct Scheduler{
  int sId{};                          // id tblScheduler
  int sCapacityTaskCount{};           // the number of tasks that can be performed simultaneously  
  StateType sState{};
  int sActiveTaskCount{};             // number of running tasks (approximate quantity)
  std::string sConnectPnt{};          // connection point: IP or DNS ':' port
};
// tblWorker
struct Worker{
  int wId{};                          // id tblWorker
  int sId{};                          // id tblScheduler
  int wCapacityTaskCount{};           // the number of tasks that can be performed simultaneously  
  StateType wState{};  
  StateType wStateMem{};
  int wActiveTaskCount{};            // number of running tasks (approximate quantity)
  int wLoadCPU{};
  bool wIsActive{};  
  std::string wConnectPnt;           // connection point: IP or DNS ':' port
};
}