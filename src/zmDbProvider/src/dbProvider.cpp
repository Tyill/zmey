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

#include "../dbProvider.h"
#include "filesJSON.h"
#include "PostgreSQL.h"
#include "SQLite.h"

namespace ZM_DB{

DbProvider* makeDbProvider(dbType dbt, const std::string& dbServer, const std::string& dbName, errCBack ecb){
  DbProvider* ret = nullptr;
  switch (dbt){
    case dbType::filesJSON: 
      ret = reinterpret_cast<DbProvider*>(new DbJSONProvider(dbServer, dbName, ecb));
      break;
    case dbType::SQLite: 
      ret = reinterpret_cast<DbProvider*>(new DbSQLiteProvider(dbServer, dbName, ecb));
      break;
#ifdef USE_PostgreSQL
    case dbType::PostgreSQL:
      ret = reinterpret_cast<DbProvider*>(DbPGProvider(dbServer, dbName, ecb));
      break;
#endif
    default:
    break;
  }
  return ret;
}
}