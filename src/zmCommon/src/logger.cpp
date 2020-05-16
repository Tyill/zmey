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

#include <fstream>
#include "../logger.h"
#include "../auxFunc.h"

using namespace std;

namespace ZM_Aux {
	Logger::Logger(const string &nameFile, const string &pathFile)
    : _nameFile(nameFile),_pathSave(pathFile){			
			_deqMess.resize(MAX_CNT_MESS);
			_readMessCnt = 0;
			_writeMessCnt = 0;            			
			_thrWriteMess = thread([this]{ writeCyc(); });
		}
	Logger::~Logger() {
			_fStop = true;
			_cval.notify_one();
			if (_thrWriteMess.joinable())
        _thrWriteMess.join();
		}
  void Logger::writeMess(const string &mess){
    lock_guard<mutex> lock(_mtxWr);			
		_deqMess[_writeMessCnt] = message(true, currDateTimeMs(), mess);
		++_writeMessCnt;
  	if (_writeMessCnt == MAX_CNT_MESS){
      _writeMessCnt = 0;
    }
  	_cval.notify_one();
	}
	void Logger::writeCyc() {
    while(!_fStop){
      std::unique_lock<std::mutex> lck(_mtxRd);
      _cval.wait(lck);

      std::ofstream slg(_pathSave + _nameFile + ".log", std::ios::app);
      while (_deqMess[_readMessCnt].activ) {
        slg << "[" << _deqMess[_readMessCnt].cTime << "] " << _deqMess[_readMessCnt].mess << std::endl;
        _deqMess[_readMessCnt].activ = false;
        ++_readMessCnt;
        if (_readMessCnt == MAX_CNT_MESS)
          _readMessCnt = 0;
      }
      slg.close();
    }
  }
}