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
#include "common/tcp.h"
#include "base/messages.h"
#include "application.h"
#include "executor.h"
#include "loop.h"

#include <signal.h>

using namespace std;

unique_ptr<db::DbProvider> 
static createDbProvider(const Application::Config& cng, string& err);

static Loop* pLoop;
static void closeHandler(int sig)
{
  if (pLoop) pLoop->stop();
}

int main(int argc, char* argv[])
{
  Application app;

  Application::Config cng;
  if (!app.parseArgs(argc, argv, cng)){
    return 1;
  }
  
  if (cng.remoteConnPnt.empty()){  // when without NAT
    cng.remoteConnPnt = cng.localConnPnt;
  } 

#define CHECK_RETURN(fun, mess) \
  if (fun){                     \
    app.statusMess(mess);       \
    return 1;                   \
  }

  CHECK_RETURN(cng.localConnPnt.empty() || (misc::split(cng.localConnPnt, ':').size() != 2), "Not set param '--localAddr[-la]' - scheduler local connection point: IP or DNS:port");
  CHECK_RETURN(cng.remoteConnPnt.empty() || (misc::split(cng.remoteConnPnt, ':').size() != 2), "Not set param '--remoteAddr[-ra]' - scheduler remote connection point: IP or DNS:port");
  CHECK_RETURN(cng.dbConnCng.connectStr.empty(), "Not set param '--dbConnStr[-db]' - database connection string");
   
  signal(SIGINT, closeHandler);
  signal(SIGTERM, closeHandler);
#ifdef __linux__
  signal(SIGHUP, closeHandler);
  signal(SIGQUIT, closeHandler);
  signal(SIGPIPE, SIG_IGN);
#endif
  
  // db providers
  string err;
  auto dbNewTask = createDbProvider(cng, err);
  auto dbSendMess = dbNewTask ? createDbProvider(cng, err) : nullptr;
  CHECK_RETURN(!dbNewTask || !dbSendMess, "Schedr db connect error " + err + ": " + cng.dbConnCng.connectStr); 
    
  Executor executor(app, *dbNewTask);
  
  // schedr from db
  CHECK_RETURN(!executor.getSchedrFromDB(cng.remoteConnPnt, *dbNewTask), "Schedr not found in db for connectPnt " + cng.remoteConnPnt);
     
  // prev tasks and workers
  executor.getPrevTaskFromDB(*dbNewTask);
  executor.getPrevWorkersFromDB(*dbNewTask);
  executor.listenNewTask(*dbNewTask, true);
   
  // TCP server
  misc::ReceiveDataCBack receiveDataCB = [&executor](const string& cp, const string& data){
    executor.receiveHandler(cp, data);
  };
  misc::ErrorStatusCBack errorStatusCB = [&executor](const string& cp, const std::string& data, const error_code& ec){
    executor.errorNotifyHandler(cp, ec);
  };  
  CHECK_RETURN(!misc::startServer(cng.localConnPnt, receiveDataCB, errorStatusCB, 0, err), 
    "Schedr error: " + cng.localConnPnt + " " + err);
  app.statusMess("Schedr running: " + cng.localConnPnt);
  
  // on start
  executor.addMessToDB(db::MessSchedr{ mess::MessType::START_SCHEDR });

  // loop ///////////////////////////////////////////////////////////////////////
  Loop loop(cng, executor, *dbNewTask, *dbSendMess);
  pLoop = &loop;  
  executor.setLoop(&loop);

  loop.run();
  
  /////////////////////////////////////////////////////////////////////////
  
  misc::stopServer();
  executor.listenNewTask(*dbNewTask, false);
  executor.stopSchedr(*dbSendMess);
}

unique_ptr<db::DbProvider> 
createDbProvider(const Application::Config& cng, string& err)
{
  unique_ptr<db::DbProvider> db(new db::DbProvider(cng.dbConnCng));
  err = db->getLastError();
  if (err.empty()){
    return db;
  } else{    
    return nullptr;
  }
}

