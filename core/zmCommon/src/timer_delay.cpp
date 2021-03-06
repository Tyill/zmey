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
#include "../aux_func.h"
#include "../timer_delay.h"

namespace ZM_Aux{

TimerDelay::TimerDelay(){
  _prevTm = currDateTimeSinceEpochMs();
  _deltaTm = 0;
  _tmCnt = 0;
}
void TimerDelay::updateCycTime(){
  uint64_t ct = currDateTimeSinceEpochMs();
  _deltaTm = ct - _prevTm;
  _prevTm = ct;
  for (int i = 0; i < _tmCnt; ++i) {
    if (!_tmrs[i].isActive){
      _tmrs[i].cDelay = 0;
    }
    _tmrs[i].isActive = false;
  }  
}
uint64_t TimerDelay::getDeltaTimeMS(){
  return currDateTimeSinceEpochMs() - _prevTm;
}
bool TimerDelay::onDelaySec(bool start, int delay, int id){
  if (id >= _tmCnt){
    _tmrs.resize(id + 1, tmBase{0, false});
    _tmCnt = id + 1;
  }
  bool res = false;
  if (start) {
    _tmrs[id].cDelay += (int)_deltaTm;
    if (_tmrs[id].cDelay >= delay * 1000){
      res = true;
    } 
  } else{
    _tmrs[id].cDelay = 0;
  }
  _tmrs[id].isActive = true;
  return res;
}
bool TimerDelay::offDelaySec(bool start, int delay, int id){
  if (id >= _tmCnt){
    _tmrs.resize(id + 1, tmBase{0, false});
    _tmCnt = id + 1;
  }
  bool res = false;
  if (start){ 
    _tmrs[id].cDelay = delay * 1000;
  }
  else if (_tmrs[id].cDelay > 0){
    res = true;
    _tmrs[id].cDelay -= (int)_deltaTm;
  }
  _tmrs[id].isActive = true;
  return (start || res);
}
bool TimerDelay::onDelayMS(bool start, int delay, int id){
  if (id >= _tmCnt){
    _tmrs.resize(id + 1, tmBase{0, false});
    _tmCnt = id + 1;
  }
  bool res = false;
  if (start) {
    _tmrs[id].cDelay += (int)_deltaTm;
    if (_tmrs[id].cDelay >= delay){
      res = true;
    }
  } else{
    _tmrs[id].cDelay = 0;
  }
  _tmrs[id].isActive = true;
  return res;
}
bool TimerDelay::offDelayMS(bool start, int delay, int id){    
  if (id >= _tmCnt){
    _tmrs.resize(id + 1, tmBase{0, false});
    _tmCnt = id + 1;
  }
  bool res = false;
  if (start){
    _tmrs[id].cDelay = delay;
  }
  else if (_tmrs[id].cDelay > 0){      
    res = true;
    _tmrs[id].cDelay -= (int)_deltaTm;
  }
  _tmrs[id].isActive = true;
  return (start || res);
}
bool TimerDelay::onDelayOncSec(bool start, int delay, int id){
  if (id >= _tmCnt){
    _tmrs.resize(id + 1, tmBase{0, false});
    _tmCnt = id + 1;
  }
  bool res = false;
  if (start) {
    _tmrs[id].cDelay += (int)_deltaTm;
    if (_tmrs[id].cDelay >= delay * 1000){
      _tmrs[id].cDelay = 0;
      res = true;
    } 
  } else{
    _tmrs[id].cDelay = 0;
  }
  _tmrs[id].isActive = true;
  return res;
}
bool TimerDelay::onDelayOncMS(bool start, int delay, int id){
  if (id >= _tmCnt){
    _tmrs.resize(id + 1, tmBase{0, false});
    _tmCnt = id + 1;
  }
  bool res = false;
  if (start) {
    _tmrs[id].cDelay += (int)_deltaTm;
    if (_tmrs[id].cDelay >= delay){
      _tmrs[id].cDelay = 0;
      res = true;
    }
  } else{
    _tmrs[id].cDelay = 0;
  }
  _tmrs[id].isActive = true;
  return res;
}
}