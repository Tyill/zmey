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
  
Executor::Executor(Application& app):
  m_app(app)
{
}

void Executor::addMessToDB(ZM_DB::MessSchedr mess)
{
  m_messToDB.push(std::move(mess));
}

bool Executor::appendNewTaskAvailable()
{
  return (m_tasks.size() < m_schedr.capacityTask) && (m_schedr.state != ZM_Base::StateType::PAUSE);
}

bool Executor::isTasksEmpty()
{
  return m_tasks.empty();
}

bool Executor::isMessToDBEmpty()
{
  return m_messToDB.empty();
}

bool Executor::getSchedrFromDB(const std::string& connPnt, ZM_DB::DbProvider& db)
{
  return db.getSchedr(connPnt, m_schedr);
}

bool Executor::listenNewTask(ZM_DB::DbProvider& db, bool on)
{
  return db.setListenNewTaskNotify(on);
}