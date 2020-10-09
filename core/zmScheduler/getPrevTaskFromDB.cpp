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
#include "zmDbProvider/dbProvider.h"
#include "zmCommon/queue.h"
#include "structurs.h"

using namespace std;

void getPrevTaskFromDB(ZM_DB::DbProvider& db, 
                       ZM_Base::Scheduler& schedr,
                       ZM_Aux::QueueThrSave<STask>& outTasks){
  vector<ZM_DB::SchedrTask> tasks;
  if (db.getTasksOfSchedr(schedr.id, tasks)){
    for(auto& t : tasks){
      outTasks.push(STask{t.qTaskId, t.base, t.params});
    }
  }else{
    statusMess("getPrevTaskFromDB db error: " + db.getLastError());
  }
};