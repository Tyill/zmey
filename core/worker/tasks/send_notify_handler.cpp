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
#include "common/serial.h"
#include "base/link.h"

using namespace std;

void Executor::sendNotifyHandler(const string& cp, const string& data, const std::error_code& ec)
{  
  auto smess = misc::deserialn(data);  
  base::MessType messType = (base::MessType)stoi(smess[Link::command]);
  if (ec && (messType != base::MessType::TASK_PROGRESS) &&
            (messType != base::MessType::INTERN_ERROR) &&
            (messType != base::MessType::PING_WORKER)){
    MessForSchedr mess;
    mess.MessType = messType;
    mess.taskId = stoi(smess[Link::taskId]);
    m_listMessForSchedr.push(move(mess));
    if (m_ctickSendNotify(1000)){
      m_app.statusMess("sendNotifyHandler error send to schedr: " + ec.message());
    }
  }
}