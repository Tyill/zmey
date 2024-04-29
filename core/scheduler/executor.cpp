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
  
Executor::Executor(Application& app, db::DbProvider& db):
  m_app(app),
  m_db(db)
{
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

void Executor::addMessToDB(db::MessSchedr mess)
{
  m_messToDB.push(std::move(mess));
}

bool Executor::appendNewTaskAvailable()
{
  const auto schedr = getScheduler();
  return schedr.sState != base::StateType::PAUSE;
}

bool Executor::isTasksEmpty()
{
  return m_tasks.empty();
}

bool Executor::isMessToDBEmpty()
{
  return m_messToDB.empty();
}

bool Executor::getSchedrFromDB(const std::string& connPnt, db::DbProvider& db)
{
  return db.getSchedr(connPnt, m_schedr);
}

base::Scheduler Executor::getScheduler()
{
  std::lock_guard<std::mutex> lk(m_mtxSchedl);
  const auto schedr = m_schedr;
  return schedr;
}

void Executor::updateScheduler(const base::Scheduler& schedr)
{
  std::lock_guard<std::mutex> lk(m_mtxSchedl);
  m_schedr = schedr;
}

bool Executor::listenNewTask(db::DbProvider& db, bool on)
{
  return db.setListenNewTaskNotify(on);
}

void Executor::errorMessage(const std::string& mess, int wId)
{
  m_messToDB.push(db::MessSchedr::errorMess(wId, mess));
  m_app.statusMess(mess);
}