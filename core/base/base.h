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
#include <atomic>

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
  std::atomic_int sState{0};
  std::atomic_int sActiveTaskCount{0}; // number of running tasks (approximate quantity)
  std::string sConnectPnt{0};          // connection point: IP or DNS ':' port

  Scheduler(){}

  Scheduler(const Scheduler& other){
    init(other);
  }
  Scheduler(Scheduler&& other){
    init(other);
  }

  Scheduler& operator=(const Scheduler& other){
    init(other);
    return *this;
  }
  Scheduler& operator=(Scheduler&& other){
    init(other);
    return *this;
  }
private:
  void init(const Scheduler& other){
    sId = other.sId;                          
    sCapacityTaskCount = other.sCapacityTaskCount;           
    sState = +other.sState;
    sActiveTaskCount = +other.sActiveTaskCount;
    sConnectPnt = other.sConnectPnt;         
  }
};
// tblWorker
struct Worker{
  int wId{};                          // id tblWorker
  int sId{};                          // id tblScheduler
  int wCapacityTaskCount{};           // the number of tasks that can be performed simultaneously  
  std::atomic_int wState{0};  
  std::atomic_int wStateMem{0};
  std::atomic_int wActiveTaskCount{0}; // number of running tasks (approximate quantity)
  std::atomic_int wLoadCPU{0};
  std::atomic_bool wIsActive{0};  
  std::string wConnectPnt;            // connection point: IP or DNS ':' port

  Worker(){}

  Worker(const Worker& other){
    init(other);
  }
  Worker(Worker&& other){
    init(other);
  }

  Worker& operator=(const Worker& other){
    init(other);
    return *this;
  }
  Worker& operator=(Worker&& other){
    init(other);
    return *this;
  }
private:
  void init(const Worker& other){
    wId = other.wId;                          
    sId = other.sId;                          
    wCapacityTaskCount = other.wCapacityTaskCount; 
    wState = +other.wState;  
    wStateMem = +other.wStateMem;
    wActiveTaskCount = +other.wActiveTaskCount; 
    wLoadCPU = +other.wLoadCPU;
    wIsActive = +other.wIsActive; 
    wConnectPnt = other.wConnectPnt;
  }
};
}