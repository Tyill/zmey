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

  enum class MessType{
    UNDEFINED = -1,
    NEW_TASK,
    TASK_RUNNING,
    TASK_ERROR,
    TASK_COMPLETED,
    TASK_PAUSE,
    TASK_START,
    TASK_STOP,
    TASK_CONTINUE,
    TASK_CANCEL, // when not yet taken to work
    TASK_PROGRESS,    
    START_WORKER,
    START_SCHEDR, 
    STOP_WORKER,
    STOP_SCHEDR,
    PAUSE_WORKER,
    PAUSE_SCHEDR, 
    START_AFTER_PAUSE_WORKER,
    START_AFTER_PAUSE_SCHEDR,  
    PING_WORKER,
    PING_SCHEDR,
    JUST_START_WORKER,
    WORKER_NOT_RESPONDING,
    INTERN_ERROR,      
  };
  // tblState
  enum class StateType{
    UNDEFINED = -1,
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
    uint64_t id;              // id tblTaskQueue
    uint64_t wId;             // preset worker id tblWorker. Default 0 - not set
    int averDurationSec;      // estimated lead time
    int maxDurationSec;       // maximum lead time
    std::string script;       // script on bash, python or cmd
    std::string params;
    StateType state;
  };  

  // tblTaskTemplate
  struct TaskTemplate{
    uint64_t uId;             // parent id
    uint64_t sId;             // preset schedr id tblScheduler. Default 0 - not set
    uint64_t wId;             // preset worker id tblWorker. Default 0 - not set
    int averDurationSec;      // estimated lead time
    int maxDurationSec;       // maximum lead time
    std::string name;
    std::string description;
    std::string script;       // script on bash, python or cmd
  };

  // tblScheduler
  struct Scheduler{
    uint64_t id;              // id tblScheduler
    StateType state;
    int capacityTask;         // the number of tasks that can be performed simultaneously  
    int activeTask;           // number of running tasks (approximate quantity)
    std::string connectPnt;   // connection point: IP or DNS ':' port
    std::string internalData; // config data in json
    std::string name;
    std::string description;
  };
  // tblWorker
  struct Worker{
    uint64_t id;              // id tblWorker
    uint64_t sId;             // id tblScheduler
    StateType state;  
    int capacityTask;         // the number of tasks that can be performed simultaneously  
    int activeTask;           // number of running tasks (approximate quantity)
    int rating;               // assigned a rating to the worker[1..10]
    int load;                 // current load [0..100]
    std::string connectPnt;   // connection point: IP or DNS ':' port
    std::string name;
    std::string description;
    static const int RATING_MAX = 10;
  };
}