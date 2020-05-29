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
#include <vector>

namespace ZM_Base{

  enum class messType{
    undefined           = -1,
    newTask             = 0,
    taskRunning         = 1,
    taskError           = 2,
    taskCompleted       = 3,
    taskPause           = 4,
    taskStart           = 5,
    taskStop            = 6,
    progress            = 7,
    pauseWorker         = 8,
    pauseSchedr         = 9, 
    startWorker         = 10,
    startSchedr         = 11, 
    pingWorker          = 12,
    pingSchedr          = 13,
    justStartWorker     = 14,
    workerNotResponding = 15,
  };

  enum class executorType{
    undefined = -1,
    cmd       = 0,
    bash      = 1,
    python    = 2,
  };  
   
  enum class stateType{
    undefined          = -1,
    ready               = 0,
    start               = 1,
    running             = 2,
    pause               = 3,
    stop                = 4,    
    completed           = 5,
    error               = 6,
    notResponding       = 7,
  };

  struct task{
    uint64_t tId = 0;         // id tblTask
    uint64_t mId = 0;         // id tblManager
    executorType exr = executorType::undefined;           
    int averDurationSec = 0;  // estimated lead time 
    int maxDurationSec = 0;   // maximum lead time      
    std::string script;       // script on bash, python or cmd
  };

  struct queueTask{
    uint64_t qId = 0;         // id tblTaskQueue
    uint64_t tId = 0;         // id tblTask
    uint64_t mId = 0;         // id tblManager
    uint64_t sId = 0;         // id tblScheduler
    uint64_t wId = 0;         // id tblWorker
    stateType state = stateType::undefined; 
    int priority = 0;         // [1..3]
    int progress = 0;         // [0..100]     
    std::string params;
    std::string result;
    std::vector<uint64_t> prevTasks; // queue task id of previous tasks to be completed
  };

  struct manager{
    uint64_t mId = 0;         // id tblManager
    stateType state = stateType::undefined;
    std::string name;         // unique name
    std::string passw;        // optional password 
    std::string connectPnt;   // connection point: IP or DNS ':' port
  };

  struct scheduler{
    uint64_t sId = 0;         // id tblScheduler
    uint64_t mId = 0;         // id tblManager
    stateType state = stateType::undefined;
    int capasityTask = 10000; // the number of tasks that can be performed simultaneously  
    int activeTask = 0;       // number of running tasks
    std::string connectPnt;   // connection point: IP or DNS ':' port
  };

  struct worker{
    uint64_t wId = 0;         // id tblWorker
    uint64_t sId = 0;         // id tblScheduler
    uint64_t mId = 0;         // id tblManager
    stateType state = stateType::undefined;
    executorType exr = executorType::undefined;     
    int capasityTask = 10;    // the number of tasks that can be performed simultaneously  
    int activeTask = 0;       // number of running tasks
    int rating = 10;          // manager is assigned a rating to the worker[1..10]
    std::string connectPnt;   // connection point: IP or DNS ':' port
  }; 
}