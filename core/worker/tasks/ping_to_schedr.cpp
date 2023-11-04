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
#include "common/tcp.h"
#include "base/messages.h"

using namespace std;

void Executor::pingToSchedr(const std::string& schedrConnPnt){
  m_worker.activeTask = (int)m_newTasks.size() + (int)m_procs.size();
  map<string, string> data{
    {Link::command, to_string((int)mess::MessType::PING_WORKER)},
    {Link::connectPnt, m_worker.connectPnt},
    {Link::activeTask, to_string(m_worker.activeTask)},
    {Link::load, to_string(m_worker.load)}
  };      
  misc::asyncSendData(schedrConnPnt, misc::serialn(data));
}

