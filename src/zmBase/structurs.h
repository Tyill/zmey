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

  enum class executorType{
    cmd = 0,
    bash = 1,
    python = 2,
  };

  enum class state{
    run = 0,
    pause = 1,
    ready = 2,
    busy = 3,
    completed = 4,
    notResponding = 5,
    taskTakeByScheduler = 6,
    taskSendToWorker = 7,
    workerSilentLongTime = 8,
  };

  struct task{
    uint64_t id;              // id tblTask in DB
    state ste;
    executorType exrType;     // executor for task
    std::string params;       // params for script
    std::string script;       // script on bash, python or cmd
    std::string meanDuration; // estimated lead time 
    std::string maxDuration;  // maximum lead time
  };

  struct worker{
    uint64_t id;              // id tblWorker in DB
    state ste;
    int capasityTask;         // the number of tasks that can be performed simultaneously  
    int activeTask;           // number of running tasks
    int rating;               // manager is assigned a rating to the worker[1..10]
    std::string connectPnt;   // connection point: IP or DNS ':' port
  };

  struct scheduler{
    uint64_t id;              // id tblScheduler in DB
    state ste;
    int capasityTask;         // the number of tasks that can be performed simultaneously  
    int activeTask;           // number of running tasks
    std::string connectPnt;   // connection point: IP or DNS ':' port
  };
}