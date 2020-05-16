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
#include <map>
#include <functional>
#include "zmBase/structurs.h"

namespace ZM_DB{

enum class messType{
  workerNotResponding = 0,
  workerRunning = 1,
  taskRunning = 2,
  taskEndSuccess = 3,
  taskEndWithError = 4,
  taskPause = 5,
  taskStop = 6,
};

struct message{
  messType type;
  std::map<std::string, std::string> params;
};

class DbProvider{  
public:
  typedef std::function<void(const std::string& stsMess)> errCBack;
  DbProvider(errCBack);    
  ~DbProvider(); 
  DbProvider(const DbProvider& other) = delete;
  DbProvider& operator=(const DbProvider& other) = delete;
  std::string getLastError();
  bool createTables();
  bool connect(const std::string& dbServer, const std::string& dbName);
  void disconnect();
  bool addSchedr(ZM_Base::scheduler& ioSchedl);
  bool getSchedr(std::string& connPnt, ZM_Base::scheduler& outSchedl);
  bool getPrevTasksForSchedr(uint64_t schedrId, std::vector<ZM_Base::task>&);
  bool getPrevWorkersForSchedr(uint64_t schedrId, std::vector<ZM_Base::worker>&);
  bool getNewTasks(std::vector<ZM_Base::task>&);
  bool getAllWorkers(std::vector<ZM_Base::worker>&);
  bool getAllManagers(std::vector<ZM_Base::worker>&);
  bool sendAllMessFromSchedr(uint64_t schedrId, std::vector<message>&);
private:
  std::string _lastErr;
  errCBack _errCBack = nullptr;
  bool query(const std::string& query, std::vector<std::vector<std::string>>& results) const;
};
}