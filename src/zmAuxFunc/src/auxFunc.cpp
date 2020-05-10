
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

#include "zmAuxFunc/auxFunc.h"
#include <cstring>
#include <thread>
#include <chrono>

using namespace std;

namespace ZM_Aux {
    
     // %Y-%m-%d %H:%M:%S
    string currDateTimeSQL() {

        time_t ct = time(nullptr);
        tm* lct = localtime(&ct);

        char curDate[24];
        strftime(curDate, 24, "%Y-%m-%d %H:%M:%S", (const tm *) &lct);

        return curDate;
    }
 
    vector<string> split(const string& str, const char *sep) {
        char *cstr = (char*)str.c_str();
        vector<string> res;
        char *pch = strtok(cstr, sep);
        while (pch != NULL) {
            res.push_back(string(pch));
            pch = strtok(NULL, sep);
        }
        return res;
    }

    void sleepMs(uint64_t ms){
        std::this_thread::sleep_for(std::chrono::milliseconds(ms));
    }    
}