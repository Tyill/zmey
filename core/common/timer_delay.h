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

#include <ctime>
#include <vector>
#include <cstdint>

namespace Aux{

class TimerDelay {
  struct tmBase{
    int cDelay;            
    bool isActive;            
  };
  std::vector<tmBase> _tmrs;
  int _tmCnt;  
  int64_t _prevTm;
  int64_t _deltaTm;  
public:
  TimerDelay();
  void updateCycTime();
  int64_t getDeltaTimeMS();
  bool onDelaySec(bool start, int delay, int id);  
  bool offDelaySec(bool start, int delay, int id);
  bool onDelayMS(bool start, int delay, int id);
  bool offDelayMS(bool start, int delay, int id);
  bool onDelayOncSec(bool start, int delay, int id);
  bool onDelayOncMS(bool start, int delay, int id);
};
}