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

#include "worker/executor.h"
#include "base/messages.h"

using namespace std;

static misc::CounterTick ctickSendNotify;

void Executor::sendNotifyHandler(const string& cp, const string& data, const std::error_code& ec)
{  
  mess::MessType messType = mess::getMessType(data);
  if (ec && (messType != mess::MessType::TASK_PROGRESS) &&
            (messType != mess::MessType::INTERN_ERROR) &&
            (messType != mess::MessType::PING_WORKER)){
    mess::TaskStatus mess;
    mess.deserialn(data);
    m_messForSchedr.push(move(mess));
    if (ctickSendNotify(1000)){
      m_app.statusMess("sendNotifyHandler error send to schedr: " + ec.message());
    }
  }
}