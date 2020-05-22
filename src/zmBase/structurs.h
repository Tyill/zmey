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

namespace ZM_Base{

  enum class messType{
    newTask = 0,
    taskRunning = 1,
    taskError = 2,
    taskSuccess = 3,
    taskPause = 4,
    taskStart = 5,
    taskStop = 6,
    progress = 7,
    pauseWorker = 8,
    pauseSchedr = 9, 
    startWorker = 10,
    startSchedr = 11, 
    pingWorker = 12,
    pingSchedr = 13,
    justStartWorker = 14,
    workerNotResponding = 15
  };

  enum class executorType{
    cmd = 0,
    bash = 1,
    python = 2,
  };  
   
  enum class state{
    run = 0,
    pause = 1,
    ready = 2,
    completed = 4,
    notResponding = 5,
    taskTakeByScheduler = 6,
    taskSendToWorker = 7,
  };

  struct task{
    uint64_t id = 0;          // id tblTask in DB
    state ste;
    executorType exrType;     // executor for task
    int averDurationSec;      // estimated lead time 
    int maxDurationSec;       // maximum lead time
    std::string params;       // params for script
    std::string script;       // script on bash, python or cmd    
  };

  struct manager{
    uint64_t id = 0;          // id tblManager in DB
    state ste;
    std::string connectPnt;   // connection point: IP or DNS ':' port
  };

  struct scheduler{
    uint64_t id = 0;          // id tblScheduler in DB
    state ste;
    int capasityTask = 10000; // the number of tasks that can be performed simultaneously  
    int activeTask = 0;       // number of running tasks
    bool isActive;
    std::string connectPnt;   // connection point: IP or DNS ':' port
  };

  struct worker{
    uint64_t id = 0;          // id tblWorker in DB
    state ste;
    executorType exrType;     // executor for task
    int capasityTask = 10;    // the number of tasks that can be performed simultaneously  
    int activeTask = 0;       // number of running tasks
    int rating = 10;          // manager is assigned a rating to the worker[1..10]
    bool isActive;
    std::string connectPnt;   // connection point: IP or DNS ':' port
  }; 
}