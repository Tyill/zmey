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
    int id{};                   // id tblTaskQueue
    int wId{};                  // preset worker id tblWorker. Default 0 - not set
    std::string params;
    std::string scriptPath;  
    std::string resultPath;  
    StateType state;
  };  
 
  // tblScheduler
  struct Scheduler{
    int id{};                   // id tblScheduler
    StateType state{};
    int capacityTask{};         // the number of tasks that can be performed simultaneously  
    int activeTask{};           // number of running tasks (approximate quantity)
    std::string connectPnt;     // connection point: IP or DNS ':' port
  };
  // tblWorker
  struct Worker{
    int id{};                   // id tblWorker
    int sId{};                  // id tblScheduler
    StateType state{};  
    int capacityTask{};         // the number of tasks that can be performed simultaneously  
    int activeTask{};           // number of running tasks (approximate quantity)
    int load{}; 
    std::string connectPnt;     // connection point: IP or DNS ':' port
  };
}