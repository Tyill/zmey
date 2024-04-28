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
#include <cstdint>

namespace misc {

//%Y-%m-%d %H:%M:%S
std::string currDateTime();

// %Y-%m-%d %H:%M:%S:%MS
std::string currDateTimeMs();

std::vector<std::string> split(const std::string& str, char sep);

bool endWith(const std::string& fullString, const std::string& ending);

bool startWith(const std::string& fullString, const std::string& starting);

std::string replace(std::string& ioStr, const std::string& targ, const std::string& repl);

int64_t currDateTimeSinceEpochMs();

bool isNumber(const std::string& s);

bool createSubDirectory(const std::string& strDirs);

void sleepMs(int ms);      

std::string trim(std::string str);

std::map<std::string, std::string> parseCMDArgs(int argc, char* argv[]);

template<typename T> 
T bound(T vmin, T val, T vmax){
  return std::min<T>(vmax, std::max<T>(val, vmin));
}

class CPUData{  
  enum class States{
    S_USER = 0,
    S_NICE,
    S_SYSTEM,
    S_IDLE,
    S_IOWAIT,
    S_IRQ,
    S_SOFTIRQ,
    S_STEAL,
    S_GUEST,
    S_GUEST_NICE,
    NUM_STATES
  };
  int prevActiveTime_ = 0,
      prevIdleTime_ = 0;
public:
  CPUData();
  int load();
};

class CounterTick{
  int tick_ = -1;
public:
  bool operator()(int setTick){
    if (tick_ >= setTick){
      tick_ = 0;
      return true;
    }
    ++tick_;
    return tick_ == 0;
  };
  void reset(){
    tick_ = -1;
  };
};
}