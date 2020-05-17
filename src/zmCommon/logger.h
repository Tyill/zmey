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
#include <condition_variable>
#include <mutex>
#include <thread>

namespace ZM_Aux {
  class Logger{
    const int MAX_CNT_MESS = 1000;
    std::string _nameFile,
                _pathSave;
    int _readMessCnt = 0,
        _writeMessCnt = 0;
    struct message {
      bool activ;
      std::string cTime;
      std::string mess;
      message(bool activ_ = false, const std::string& cTime_= "", const std::string& mess_ = ""):
        activ(activ_), cTime(cTime_), mess(mess_){}
    };
    std::vector<message> _deqMess;
    std::mutex _mtxWr,
               _mtxRd;
    std::thread _thrWriteMess;
    std::condition_variable _cval;
    bool _fStop = false;
    void writeCyc();
    
    public:
      Logger(const std::string &nameFile, const std::string &pathFile);			
      ~Logger();
      void writeMess(const std::string &mess);
  };
}