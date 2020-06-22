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
  // tblExecutor
  enum class executorType{
    cmd       = 0,
    bash      = 1,
    python    = 2,
  };  
  // tblState
  enum class stateType{
    undefined           = -1,
    ready               = 0,  // for task: can be taken to work
    start               = 1,  // for task: taken to work
    running             = 2,  // for task: running
    pause               = 3,
    stop                = 4,    
    completed           = 5,
    error               = 6,
    notResponding       = 7,
  };
  // tblUser
  struct user{
    uint64_t id;              // id tblUser
    std::string name;         // unique name
    std::string passw;        // optional password
    std::string description;
  };
  // tblUPipeline
  struct uPipeline{
    uint64_t id;              // id tblUPipeline
    uint64_t uId;             // id tblUser
    int isShared;             // may be shared [0..1]   
    std::string name;         // unique name
    std::string description;
  };
  // tblTask
  struct task{
    uint64_t id;              // id tblTask
    executorType exr;
    int averDurationSec;      // estimated lead time
    int maxDurationSec;       // maximum lead time
    std::string script;       // script on bash, python or cmd
  };  
  // tblTaskQueue
  struct queueTask{
    uint64_t id;              // id tblTaskQueue
    uint64_t tId;             // id tblTask
    uint64_t uId;             // launcher id tblUser
    stateType state;
    int priority;             // [1..3]
    int progress;             // [0..100] 
    std::string params;       // CLI params for script: {{key, sep, val},{..}..}
    std::string result;       // result of script: {key, sep, val}
  };  
  // tblUTaskTemplate
  struct uTaskTemplate{
    uint64_t uId;             // parent id tblUser
    int isShared;             // may be shared [0..1]  
    std::string name;
    std::string description;
    task base;
  };
  // tblUPipelineTask
  struct uTask{
    uint64_t id;              // id tblUPipelineTask 
    uint64_t pplId;           // id tblUPipeline
    std::string prevTasks;    // queue task id tblUTask of previous tasks to be completed: {tId,..}
    std::string nextTasks;    // queue task id tblUTask of next tasks: {tId,..}
    std::string screenRect;   // rect on screen: x y w h
    queueTask base; 
  };
  // tblScheduler
  struct scheduler{
    uint64_t id;              // id tblScheduler
    stateType state;
    int capacityTask;         // the number of tasks that can be performed simultaneously  
    int activeTask;           // number of running tasks
    std::string connectPnt;   // connection point: IP or DNS ':' port
  };
  // tblWorker
  struct worker{
    uint64_t id;              // id tblWorker
    uint64_t sId;             // id tblScheduler
    stateType state;
    executorType exr;     
    int capacityTask;         // the number of tasks that can be performed simultaneously  
    int activeTask;           // number of running tasks
    int rating;               // manager is assigned a rating to the worker[1..10]
    std::string connectPnt;   // connection point: IP or DNS ':' port
  }; 
}