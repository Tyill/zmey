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
    ready               = 0, // for task: can be taken to work
    start               = 1, // for task: taken to work
    running             = 2, // for task: running
    pause               = 3,
    stop                = 4,    
    completed           = 5,
    error               = 6,
    notResponding       = 7,
  };

  struct user{
    uint64_t id;              // id tblUser
    std::string name;         // unique name
    std::string passw;        // optional password
    std::string description;
  };

  struct uPipeline{
    uint64_t id = 0;          // id tblUPipeline
    uint64_t uId = 0;         // id tblUser
    std::string name;         // unique name
    std::string description;
  };
  
  struct task{
    uint64_t id = 0;          // id tblTask
    executorType exr = executorType::undefined;           
    int averDurationSec = 0;  // estimated lead time 
    int maxDurationSec = 0;   // maximum lead time      
    std::string script;       // script on bash, python or cmd    
  };  

  struct queueTask{
    uint64_t id = 0;          // id tblTaskQueue
    uint64_t tId = 0;         // id tblTask
    uint64_t uId = 0;         // launcher id tblUser
    stateType state = stateType::undefined; 
    int priority = 0;         // [1..3]
    int progress = 0;         // [0..100]     
    std::string params;       // params of script: -key=value
    std::string result;
  };  
  
  struct uTaskTemplate{
    uint64_t uId = 0;         // parent id tblUser
    std::string name;
    std::string description;
    task base;
  };
  
  struct uScreenRect{
    int x, y, w, h;
    std::string toString(){
      return std::to_string(x) + " " + std::to_string(y) + " " + std::to_string(w) + " " + std::to_string(h);
    }
  };
  struct uTask{
    uint64_t id = 0;          // id tblUTask 
    uint64_t pplId = 0;       // id tblUPipeline
    std::vector<uint64_t> prevTasks; // queue task id tblUTask of previous tasks to be completed
    std::vector<uint64_t> nextTasks; // queue task id tblUTask of next tasks
    uScreenRect rct;          // rect on screen
    queueTask base; 
  };

  struct scheduler{
    uint64_t id = 0;          // id tblScheduler
    stateType state = stateType::undefined;
    int capacityTask = 10000; // the number of tasks that can be performed simultaneously  
    int activeTask = 0;       // number of running tasks
    std::string connectPnt;   // connection point: IP or DNS ':' port
  };
  using schedrTask = std::pair<ZM_Base::task, ZM_Base::queueTask>;

  struct worker{
    uint64_t id = 0;          // id tblWorker
    uint64_t sId = 0;         // id tblScheduler
    stateType state = stateType::undefined;
    executorType exr = executorType::undefined;     
    int capacityTask = 10;    // the number of tasks that can be performed simultaneously  
    int activeTask = 0;       // number of running tasks
    int rating = 10;          // manager is assigned a rating to the worker[1..10]
    std::string connectPnt;   // connection point: IP or DNS ':' port
  }; 
}