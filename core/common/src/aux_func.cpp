
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
#include <cctype>
#include <thread>
#include <chrono>
#include <sstream>
#include <fstream>
#include <algorithm>

#include "common/aux_func.h"

#ifdef _WIN32
#include <windows.h>
#endif

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

  uint64_t ms = chrono::time_point_cast<chrono::milliseconds>(chrono::system_clock::now()).time_since_epoch().count() % 1000;

  char curDate[24];
  strftime(curDate, 24, "%Y-%m-%d %H:%M:%S:", lct);

  (sprintf)(curDate, "%s%03d", curDate, int(ms));

  return curDate;
} 
vector<string> split(const string& str, char sep) {  
  vector<string> res;
  std::istringstream iss(str);
  string token;
  while (getline(iss, token, sep)){
    res.emplace_back(token);
  }
  return res;
}
bool endWith (const std::string& fullString, const std::string& ending) {
  if (fullString.size() >= ending.size()) {
    return (0 == fullString.compare (fullString.size() - ending.size(), ending.size(), ending));
  } else {
    return false;
  }
}
bool startWith (const std::string& fullString, const std::string& starting) {
  if (fullString.size() >= starting.size()) {
    return (0 == fullString.compare (0, starting.size(), starting));
  } else {
    return false;
  }
}
std::string replace(std::string& ioStr, const std::string& targ, const std::string& repl){
  size_t cpos = ioStr.find(targ, 0),
         tsz = targ.size(),
         rsz = repl.size();
  while (cpos != std::string::npos){
    ioStr.replace(cpos, tsz, repl);  
    cpos = ioStr.find(targ, cpos + rsz);  
  }
  return ioStr;
}
std::string trim(std::string str){
  auto itB = std::find_if(str.cbegin(), str.cend(), 
                          [](int32_t ch) -> bool {
                            return !std::isspace(ch);
                          });
  str.erase(str.cbegin(), itB);

  auto itE = std::find_if(str.crbegin(), str.crend(),
                          [](int32_t ch) -> bool {
                            return !std::isspace(ch);
                          });
  str.erase(itE.base(), str.cend());

  return str;
}
uint64_t currDateTimeSinceEpochMs(){
  auto now = std::chrono::system_clock::now();
  auto now_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(now);
  return now_ms.time_since_epoch().count();
}
bool isNumber(const std::string& s){
  for(size_t i = 0; i < s.size(); ++i){
    if (!std::isdigit(s[i])){
      return false; 
    }
  }
  return !s.empty();
}

std::map<std::string, std::string> 
parseCMDArgs(int argc, char* argv[]){
  string sargs;
  for (int i = 1; i < argc; ++i){
    sargs += argv[i];
  }
  map<string, string> sprms;
  auto argPair = ZM_Aux::split(sargs, '-');
  for (auto& arg : argPair){    
    arg = ZM_Aux::trim(arg);
    if (arg.empty()) continue;
    size_t sp = min(arg.find_first_of("="), arg.find_first_of(" "));
    if (sp != std::string::npos){
      sprms[ZM_Aux::trim(arg.substr(0, sp))] = ZM_Aux::trim(arg.substr(sp + 1));
    }else{
      sprms[ZM_Aux::trim(arg)] = "";
    }
  }
  return sprms;
}

void sleepMs(uint64_t ms){
  std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}    

CPUData::CPUData(){
  load();
}
int CPUData::load(){
#ifdef __linux__
  std::ifstream fileStat("/proc/stat");  
  std::string line;
  while(std::getline(fileStat, line)){
    if (startWith(line, "cpu")){
      auto times = split(line, ' ');
      for (auto it = times.begin(); it != times.end();){
        if (it->empty() || !isNumber(*it)){
          it = times.erase(it);
        }else{
          ++it;
        }
      }
      if (times.size() >= (size_t)States::NUM_STATES){
        int activeTime = stoi(times[(int)States::S_USER]) +
                         stoi(times[(int)States::S_NICE]) +
                         stoi(times[(int)States::S_SYSTEM]) +
                         stoi(times[(int)States::S_IRQ]) +
                         stoi(times[(int)States::S_SOFTIRQ]) +
                         stoi(times[(int)States::S_STEAL]) +
                         stoi(times[(int)States::S_GUEST]) +
                         stoi(times[(int)States::S_GUEST_NICE]); 
        int idleTime = stoi(times[(int)States::S_IDLE]) +
                       stoi(times[(int)States::S_IOWAIT]);   

        int activeTimeTotal = activeTime - _prevActiveTime,
            idleTimeTotal = idleTime - _prevIdleTime,
            totalTime = activeTimeTotal + idleTimeTotal;
  
        if (totalTime == 0) totalTime = 1;  

        _prevActiveTime = activeTime;
        _prevIdleTime = idleTime;

        return bound(0, (100 * activeTimeTotal) / totalTime, 100);
      }
    }
  }
#elif _WIN32
  auto calculateCPULoad = [](unsigned long long idleTicks, unsigned long long totalTicks) -> float
  {
    static unsigned long long _previousTotalTicks = 0;
    static unsigned long long _previousIdleTicks = 0;

    unsigned long long totalTicksSinceLastTime = totalTicks-_previousTotalTicks;
    unsigned long long idleTicksSinceLastTime  = idleTicks-_previousIdleTicks;

    float ret = 1.0f-((totalTicksSinceLastTime > 0) ? ((float)idleTicksSinceLastTime)/totalTicksSinceLastTime : 0);

    _previousTotalTicks = totalTicks;
    _previousIdleTicks  = idleTicks;
    return ret;
  };

  auto fileTimeToInt64 = [](const FILETIME & ft) ->unsigned long long {
    return (((unsigned long long)(ft.dwHighDateTime))<<32)|((unsigned long long)ft.dwLowDateTime);
  };
  
  FILETIME idleTime, kernelTime, userTime;
  if (GetSystemTimes(&idleTime, &kernelTime, &userTime)){ 
    auto idle = fileTimeToInt64(idleTime);
    auto kernelAUser = fileTimeToInt64(kernelTime) + fileTimeToInt64(userTime);
    return int(calculateCPULoad(idle, kernelAUser) * 100.0f);
  }
#endif
  return 0;    
}

}