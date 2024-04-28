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
#include "../misc.h"
#include "../timer_delay.h"

namespace misc{

TimerDelay::TimerDelay(){
  prevTm_ = currDateTimeSinceEpochMs();
  deltaTm_ = 0;
  tmCnt_ = 0;
}
void TimerDelay::updateCycTime(){
  int64_t ct = currDateTimeSinceEpochMs();
  deltaTm_ = ct - prevTm_;
  prevTm_ = ct;
  for (int i = 0; i < tmCnt_; ++i) {
    if (!tmrs_[i].isActive){
      tmrs_[i].cDelay = 0;
    }
    tmrs_[i].isActive = false;
  }  
}
int64_t TimerDelay::getDeltaTimeMS(){
  return currDateTimeSinceEpochMs() - prevTm_;
}
bool TimerDelay::onDelaySec(bool start, int delay, int id){
  if (id >= tmCnt_){
    tmrs_.resize(id + 1, TmBase{0, false});
    tmCnt_ = id + 1;
  }
  bool res = false;
  if (start) {
    tmrs_[id].cDelay += (int)deltaTm_;
    if (tmrs_[id].cDelay >= delay * 1000){
      res = true;
    } 
  } else{
    tmrs_[id].cDelay = 0;
  }
  tmrs_[id].isActive = true;
  return res;
}
bool TimerDelay::offDelaySec(bool start, int delay, int id){
  if (id >= tmCnt_){
    tmrs_.resize(id + 1, TmBase{0, false});
    tmCnt_ = id + 1;
  }
  bool res = false;
  if (start){ 
    tmrs_[id].cDelay = delay * 1000;
  }
  else if (tmrs_[id].cDelay > 0){
    res = true;
    tmrs_[id].cDelay -= (int)deltaTm_;
  }
  tmrs_[id].isActive = true;
  return (start || res);
}
bool TimerDelay::onDelayMS(bool start, int delay, int id){
  if (id >= tmCnt_){
    tmrs_.resize(id + 1, TmBase{0, false});
    tmCnt_ = id + 1;
  }
  bool res = false;
  if (start) {
    tmrs_[id].cDelay += (int)deltaTm_;
    if (tmrs_[id].cDelay >= delay){
      res = true;
    }
  } else{
    tmrs_[id].cDelay = 0;
  }
  tmrs_[id].isActive = true;
  return res;
}
bool TimerDelay::offDelayMS(bool start, int delay, int id){    
  if (id >= tmCnt_){
    tmrs_.resize(id + 1, TmBase{0, false});
    tmCnt_ = id + 1;
  }
  bool res = false;
  if (start){
    tmrs_[id].cDelay = delay;
  }
  else if (tmrs_[id].cDelay > 0){      
    res = true;
    tmrs_[id].cDelay -= (int)deltaTm_;
  }
  tmrs_[id].isActive = true;
  return (start || res);
}
bool TimerDelay::onDelayOncSec(bool start, int delay, int id){
  if (id >= tmCnt_){
    tmrs_.resize(id + 1, TmBase{0, false});
    tmCnt_ = id + 1;
  }
  bool res = false;
  if (start) {
    tmrs_[id].cDelay += (int)deltaTm_;
    if (tmrs_[id].cDelay >= delay * 1000){
      tmrs_[id].cDelay = 0;
      res = true;
    } 
  } else{
    tmrs_[id].cDelay = 0;
  }
  tmrs_[id].isActive = true;
  return res;
}
bool TimerDelay::onDelayOncMS(bool start, int delay, int id){
  if (id >= tmCnt_){
    tmrs_.resize(id + 1, TmBase{0, false});
    tmCnt_ = id + 1;
  }
  bool res = false;
  if (start) {
    tmrs_[id].cDelay += (int)deltaTm_;
    if (tmrs_[id].cDelay >= delay){
      tmrs_[id].cDelay = 0;
      res = true;
    }
  } else{
    tmrs_[id].cDelay = 0;
  }
  tmrs_[id].isActive = true;
  return res;
}
}