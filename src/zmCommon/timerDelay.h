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

namespace ZM_Aux{

class TimerDelay {
  struct tmBase{
    int tmCnt;						
    bool tmActiv;						
  };
  std::vector<tmBase> _tmrs;
  int _tmSz;	
  uint64_t _prevCycTm;
  uint64_t _cycleTm;
  tm _prevTm;
  bool _secOnc = false, 
       _minOnc = false,
       _hourOnc = false;
public:
  TimerDelay();
  void updateCycTime();
  uint64_t getDeltaTimeMS();
  bool onDelTmSec(bool start, int delay, int id);
  bool offDelTmSec(bool start, int delay, int id);
  bool onDelTmMS(bool start, int delay, int id);
  bool offDelTmMS(bool start, int delay, int id);
  bool secOnc();
  bool minOnc();
  bool hourOnc();
};
}