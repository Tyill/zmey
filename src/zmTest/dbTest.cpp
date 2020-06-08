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
#include "prepareTest.h"
#include "zmDbProvider/dbProvider.h"
#include "zmCommon/auxFunc.h"

using namespace std;

class DBTest : public ::testing::Test {
public:
  DBTest() { 
    ZM_DB::connectCng cng;
    cng.selType = ZM_DB::dbType::PostgreSQL;
    cng.connectStr = "host=localhost port=5432 password=123 dbname=zmeyDb connect_timeout=10";

    _pDb = ZM_DB::makeDbProvider(cng);
    
    auto err = _pDb->getLastError();
    if (!err.empty()){    
      TEST_COUT << err << endl;     
    }    
  }
  ~DBTest() {
    if (_pDb){
      delete _pDb;
    }
  }
protected:
  ZM_DB::DbProvider* _pDb = nullptr; 
};

TEST_F(DBTest, addUser){
  EXPECT_TRUE(_pDb->delAllUsers()) << _pDb->getLastError();
  
  ZM_Base::user usr;
  usr.name = "";
  usr.passw = "";  
  uint64_t uId = 0;  
  EXPECT_TRUE(!_pDb->addUser(usr, uId) && (uId == 0)) << _pDb->getLastError();

  usr.name = "alm1";
  usr.passw = "";  
  uId = 0;
  EXPECT_TRUE(_pDb->addUser(usr, uId) && (uId > 0)) << _pDb->getLastError();

  usr.name = "alm1";
  usr.passw = "";
  uId = 0;
  EXPECT_TRUE(!_pDb->addUser(usr, uId) && (uId == 0)) << _pDb->getLastError();

  usr.name = "alm2";
  usr.passw = "123";
  uint64_t uId2 = 0;
  EXPECT_TRUE(_pDb->addUser(usr, uId2) && (uId2 > 0)) << _pDb->getLastError();
  
  EXPECT_TRUE(uId2 > uId) << _pDb->getLastError();
}

TEST_F(DBTest, delUser){
 
}
