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
#include "../auxFunc.h"
#include "../timerDelay.h"

namespace ZM_Aux{

TimerDelay::TimerDelay(){
  _prevCycTm = currDateTimeSinceEpochMs();
  _cycleTm = 0;
  _tmSz = 0;
}
void TimerDelay::updateCycTime(){
  uint64_t ct = currDateTimeSinceEpochMs();
  _cycleTm = ct - _prevCycTm;
  _prevCycTm = ct;
  for (int i = 0; i < _tmSz; ++i) {
    if (!_tmrs[i].tmActiv){
      _tmrs[i].tmCnt = 0;
    }
    _tmrs[i].tmActiv = false;
  }  
}
uint64_t TimerDelay::getDeltaTimeMS(){
  return currDateTimeSinceEpochMs() - _prevCycTm;
}
bool TimerDelay::onDelaySec(bool start, int delay, int id){
  if (id >= _tmSz){
    _tmrs.resize(id + 1, tmBase{0, false});
    _tmSz = id + 1;
  }
  bool res = false;
  if (start) {
    _tmrs[id].tmCnt += (int)_cycleTm;
    if (_tmrs[id].tmCnt >= delay * 1000){
      res = true;
    } 
  } else{
    _tmrs[id].tmCnt = 0;
  }
  _tmrs[id].tmActiv = true;
  return res;
}
bool TimerDelay::offDelaySec(bool start, int delay, int id){
  if (id >= _tmSz){
    _tmrs.resize(id + 1, tmBase{0, false});
    _tmSz = id + 1;
  }
  bool res = false;
  if (start){ 
    _tmrs[id].tmCnt = delay * 1000;
  }
  else if (_tmrs[id].tmCnt > 0){
    res = true;
    _tmrs[id].tmCnt -= (int)_cycleTm;
  }
  _tmrs[id].tmActiv = true;
  return (start || res);
}
bool TimerDelay::onDelayMS(bool start, int delay, int id){
  if (id >= _tmSz){
    _tmrs.resize(id + 1, tmBase{0, false});
    _tmSz = id + 1;
  }
  bool res = false;
  if (start) {
    _tmrs[id].tmCnt += (int)_cycleTm;
    if (_tmrs[id].tmCnt >= delay){
      res = true;
    }
  } else{
    _tmrs[id].tmCnt = 0;
  }
  _tmrs[id].tmActiv = true;
  return res;
}
bool TimerDelay::offDelayMS(bool start, int delay, int id){    
  if (id >= _tmSz){
    _tmrs.resize(id + 1, tmBase{0, false});
    _tmSz = id + 1;
  }
  bool res = false;
  if (start){
    _tmrs[id].tmCnt = delay;
  }
  else if (_tmrs[id].tmCnt > 0){      
    res = true;
    _tmrs[id].tmCnt -= (int)_cycleTm;
  }
  _tmrs[id].tmActiv = true;
  return (start || res);
}
bool TimerDelay::onDelayOncSec(bool start, int delay, int id){
  if (id >= _tmSz){
    _tmrs.resize(id + 1, tmBase{0, false});
    _tmSz = id + 1;
  }
  bool res = false;
  if (start) {
    _tmrs[id].tmCnt += (int)_cycleTm;
    if (_tmrs[id].tmCnt >= delay * 1000){
      _tmrs[id].tmCnt = 0;
      res = true;
    } 
  } else{
    _tmrs[id].tmCnt = 0;
  }
  _tmrs[id].tmActiv = true;
  return res;
}
bool TimerDelay::onDelayOncMS(bool start, int delay, int id){
  if (id >= _tmSz){
    _tmrs.resize(id + 1, tmBase{0, false});
    _tmSz = id + 1;
  }
  bool res = false;
  if (start) {
    _tmrs[id].tmCnt += (int)_cycleTm;
    if (_tmrs[id].tmCnt >= delay){
      _tmrs[id].tmCnt = 0;
      res = true;
    }
  } else{
    _tmrs[id].tmCnt = 0;
  }
  _tmrs[id].tmActiv = true;
  return res;
}
}