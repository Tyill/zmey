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

#include "../dbProvider.h"

class DbSQLiteProvider final : ZM_DB::DbProvider{  
public:
  DbSQLiteProvider(const std::string& dbServer, const std::string& dbName, ZM_DB::errCBack);    
  ~DbSQLiteProvider(); 
  DbSQLiteProvider(const DbProvider& other) = delete;
  DbSQLiteProvider& operator=(const DbProvider& other) = delete;
  // for zmSchedr
  bool addSchedr(const ZM_Base::scheduler& schedl, uint64_t& schId) override;
  bool getSchedr(std::string& connPnt, ZM_Base::scheduler& outSchedl) override;
  // for zmSchedr
  bool getTasksForSchedr(uint64_t schedrId, std::vector<ZM_Base::task>&) override;
  bool getWorkersForSchedr(uint64_t schedrId, std::vector<ZM_Base::worker>&) override;
  bool getNewTasks(std::vector<ZM_Base::task>&, int maxTaskCnt) override;
  bool sendAllMessFromSchedr(uint64_t schedrId, std::vector<ZM_DB::messSchedr>&) override;
private:
  std::string _lastErr;
  ZM_DB::errCBack _errCBack = nullptr;
  bool query(const std::string& query, std::vector<std::vector<std::string>>& results) const;
};