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
    PROGRESS,
    PAUSE_WORKER,
    PAUSE_SCHEDR, 
    START_WORKER,
    START_SCHEDR, 
    PING_WORKER,
    PING_SCHEDR,
    JUST_START_WORKER,
    WORKER_RATING,
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
    ERROR,
    CANCEL,
    NOT_RESPONDING,
  };
  // tblUser
  struct User{
    uint64_t id;              // id tblUser
    std::string name;         // unique name
    std::string passw;        // optional password
    std::string description;
  };
  // tblUPipeline
  struct UPipeline{
    uint64_t id;              // id tblUPipeline
    uint64_t uId;             // id tblUser
    std::string name;         // unique name
    std::string description;
  };
  // tblUGroup
  struct UGroup{
    uint64_t id;              // id tblUGroup
    uint64_t pplId;           // id tblUPipeline
    std::string name;         // unique name
    std::string description;
  };
  // tblTask
  struct Task{
    uint64_t id;              // id tblTask
    int averDurationSec;      // estimated lead time
    int maxDurationSec;       // maximum lead time
    std::string script;       // script on bash, python or cmd
  };  
  // tblTaskQueue
  struct QueueTask{
    uint64_t id;              // id tblTaskQueue
    uint64_t tId;             // id tblTask
    uint64_t uId;             // launcher id tblUser
    StateType state;
    int priority;             // [1..3]
    int progress;             // [0..100] 
    std::string params;       // CLI params for script: {param1,param2..}
    std::string result;       // result of script
  };  
  // tblUTaskTemplate
  struct UTaskTemplate{
    uint64_t uId;             // parent id tblUser
    std::string name;
    std::string description;
    Task base;
  };
  // tblUPipelineTask
  struct UTask{
    uint64_t id;              // id tblUPipelineTask 
    uint64_t pplId;           // id tblUPipeline
    uint64_t gId;             // id tblUGroup
    std::string prevTasks;    // queue task id tblUTask of previous tasks to be completed: {tId,..}
    std::string nextTasks;    // queue task id tblUTask of next tasks: {tId,..}
    QueueTask base; 
  };
  // tblScheduler
  struct Scheduler{
    uint64_t id;              // id tblScheduler
    StateType state;
    int capacityTask;         // the number of tasks that can be performed simultaneously  
    int activeTask;           // number of running tasks (approximate quantity)
    std::string connectPnt;   // connection point: IP or DNS ':' port
  };
  // tblWorker
  struct Worker{
    uint64_t id;              // id tblWorker
    uint64_t sId;             // id tblScheduler
    StateType state;  
    int capacityTask;         // the number of tasks that can be performed simultaneously  
    int activeTask;           // number of running tasks (approximate quantity)
    int rating;               // manager is assigned a rating to the worker[1..10]
    int load;                 // current load [0..100]
    std::string connectPnt;   // connection point: IP or DNS ':' port
    static const int RATING_MAX = 10;
  };
}