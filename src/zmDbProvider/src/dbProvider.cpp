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
#include "dbPGProvider.h"

namespace ZM_DB{

std::string dbTypeToStr(dbType dbt){
  switch (dbt){
    case dbType::PostgreSQL: return "PostgreSQL";
    default:                 return "undefined";
  }
}
dbType dbTypeFromStr(const std::string& dbt){
  if (dbt == "PostgreSQL") return dbType::PostgreSQL;
  else                     return dbType::undefined;
}

DbProvider* makeDbProvider(const ZM_DB::connectCng& connCng){
  DbProvider* ret = nullptr;
  switch (connCng.selType){
    case dbType::PostgreSQL:
      ret = reinterpret_cast<DbProvider*>(new DbPGProvider(connCng));
      break;
    default:
      break;
  }
  return ret;
}
void DbProvider::setErrorCBack(errCBack ecb, udata ud){
  _errCBack = ecb;
  _errUData = ud;
}
void DbProvider::errorMess(const std::string& mess){
  _err = mess;
  if (_errCBack){
    _errCBack(mess.c_str(), _errUData);
  } 
}
std::string DbProvider::getLastError() const{
  return _err;
}  
}