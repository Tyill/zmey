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
#include <cstdint>

namespace ZM_Aux {

//%Y-%m-%d %H:%M:%S
std::string currDateTime();

// %Y-%m-%d %H:%M:%S:%MS
std::string currDateTimeMs();

std::vector<std::string> split(std::string str, const char *sep);

bool endWith(const std::string& fullString, const std::string& ending);

bool startWith(const std::string& fullString, const std::string& starting);

std::string replace(std::string& ioStr, const std::string& targ, const std::string& repl);

uint64_t currDateTimeSinceEpochMs();

bool isNumber(const std::string& s);

void sleepMs(uint64_t ms);      

std::string trim(std::string &str);

class CounterTick{
  int _valmem = 0;
public:
  bool operator()(int val){
    if (_valmem >= val){
      _valmem = 0;
      return true;
    }
    ++_valmem;
    return false;
  };
};
}