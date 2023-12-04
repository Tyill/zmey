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

#include "pg/pg_impl.h"

using namespace std;

namespace db{

DbProvider::DbProvider(const db::ConnectCng& cng) :
  m_impl(new DbProvider::Impl),
  m_connCng(cng){ 

  m_impl->m_db = (PGconn*)PQconnectdb(cng.connectStr.c_str());
  if (PQstatus(pg_) != CONNECTION_OK){
    errorMess(PQerrorMessage(pg_));
    return;
  }
}
DbProvider::~DbProvider(){  
  if (m_impl->m_thrEndTask.joinable()){
    m_impl->m_fClose = true; 
    m_impl->m_thrEndTask.join();
  }
  if (pg_){
    PQfinish(pg_);
  }
  delete m_impl;
}

void DbProvider::setErrorCBack(ErrCBack ecb, UData ud){
  lock_guard<mutex> lk(m_mtx);
  m_errCBack = ecb;
  m_errUData = ud;
}
std::string DbProvider::getLastError() const{
  lock_guard<mutex> lk(m_mtx);
  return m_err;
}

void DbProvider::errorMess(const std::string& mess){
  lock_guard<mutex> lk(m_mtx);
  m_err = mess;
  if (m_errCBack){
    m_errCBack(mess.c_str(), m_errUData);
  } 
}

}