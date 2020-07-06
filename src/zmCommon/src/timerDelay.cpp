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
  _secOnc = _minOnc = _hourOnc = false;
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
  time_t t = time(nullptr);
  tm* lct = localtime(&t);
  _hourOnc = (lct->tm_hour != _prevTm.tm_hour);
  _minOnc = (lct->tm_min != _prevTm.tm_min);
  _secOnc = (lct->tm_sec != _prevTm.tm_sec);
  _prevTm = *lct;
}
uint64_t TimerDelay::getDeltaTimeMS(){
  return currDateTimeSinceEpochMs() - _prevCycTm;
}
bool TimerDelay::onDelTmSec(bool start, int delay, int id){
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
bool TimerDelay::offDelTmSec(bool start, int delay, int id){
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
bool TimerDelay::onDelTmMS(bool start, int delay, int id){
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
bool TimerDelay::offDelTmMS(bool start, int delay, int id){    
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
bool TimerDelay::secOnc(){
  return _secOnc;
}
bool TimerDelay::minOnc(){
  return _minOnc;
}
bool TimerDelay::hourOnc(){
  return _hourOnc;
}  
}