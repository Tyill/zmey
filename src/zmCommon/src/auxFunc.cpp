
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

#include "zmCommon/auxFunc.h"
#include <cstring>
#include <thread>
#include <chrono>
#include <algorithm>

using namespace std;

namespace ZM_Aux {
    
 // %Y-%m-%d %H:%M:%S
string currDateTime() {
  
  time_t ct;
  time (&ct);
  tm* lct = localtime (&ct);
  char curDate[24];
  strftime(curDate, 24, "%Y-%m-%d %H:%M:%S", lct);
  return curDate;
}

    // %Y-%m-%d %H:%M:%S:%MS
string currDateTimeMs() {

  time_t ct;
  time (&ct);
  tm* lct = localtime (&ct);

  uint64_t ms = chrono::time_point_cast<chrono::milliseconds>(chrono::system_clock::now()).time_since_epoch().count();
  uint64_t mspr = ms / 1000;
  ms -= mspr * 1000;

  char curDate[24];
  strftime(curDate, 24, "%Y-%m-%d %H:%M:%S:", lct);

  (sprintf)(curDate, "%s%03d", curDate, int(ms));

  return curDate;
}
 
vector<string> split(const string& str, const char *sep) {
  char *cstr = (char*)str.c_str();
  vector<string> res;
  char *pch = strtok(cstr, sep);
  while (pch != NULL) {
    res.push_back(string(pch));
    pch = strtok(NULL, sep);
  }
  return res;
}

uint64_t currDateTimeSinceEpochMs(){
  auto now = std::chrono::system_clock::now();
  auto now_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(now);
  return now_ms.time_since_epoch().count();
}

bool isNumber(const std::string& s){
  return !s.empty() && std::find_if(s.begin(), s.end(),
    [](char c) {
      return !std::isdigit(c);
    }) == s.end();
}

void sleepMs(uint64_t ms){
  std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}    
}