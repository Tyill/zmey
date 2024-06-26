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

#include "executor.h"
#include "application.h"
#include "loop.h"
  
Executor::Executor(Application& app, const std::string& connPnt):
  m_app(app)  
{
  m_worker.wConnectPnt = connPnt;
  m_worker.wLoadCPU = 0;
}

void Executor::setLoop(Loop* l)
{
  m_loop = l;
}
void Executor::loopNotify()
{
  if (m_loop) m_loop->standUpNotify();
}
void Executor::loopStop()
{
  if (m_loop) m_loop->stop();
}

void Executor::addMessForSchedr(mess::TaskStatus mess)
{
  m_messForSchedr.push(std::move(mess));
}

void Executor::addErrMess(std::string mess)
{
  m_errMess.push(std::move(mess));
}

void Executor::setLoadCPU(int load)
{
  m_worker.wLoadCPU = load;
}

bool Executor::isErrMessEmpty()
{
  return m_errMess.empty();
}

bool Executor::isNewTasksEmpty()
{
  return m_newTasks.empty();
}

bool Executor::isMessForSchedrEmpty()
{
  return m_messForSchedr.empty();
}

void Executor::errorMessage(std::string mess)
{
  m_app.statusMess(mess);
  m_errMess.push(move(mess)); 
}