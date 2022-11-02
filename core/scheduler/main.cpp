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
#include "application.h"
#include "executor.h"
#include "loop.h"

#include <signal.h>

using namespace std;

void closeHandler(int sig);

unique_ptr<DB::DbProvider> 
createDbProvider(const Application::Config& cng, string& err);

#define CHECK_RETURN(fun, mess) \
  if (fun){                     \
    app.statusMess(mess);       \
    return -1;                  \
  }

int main(int argc, char* argv[])
{
  Application app;

  Application::Config cng;
  if (!app.parseArgs(argc, argv, cng)){
    return 0;
  }
  
  if (cng.remoteConnPnt.empty()){  // when without NAT
    cng.remoteConnPnt = cng.localConnPnt;
  } 

  CHECK_RETURN(cng.localConnPnt.empty() || (Aux::split(cng.localConnPnt, ':').size() != 2), "Not set param '--localAddr[-la]' - scheduler local connection point: IP or DNS:port");
  CHECK_RETURN(cng.remoteConnPnt.empty() || (Aux::split(cng.remoteConnPnt, ':').size() != 2), "Not set param '--remoteAddr[-ra]' - scheduler remote connection point: IP or DNS:port");
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
  CHECK_RETURN(!dbNewTask || !dbSendMess, "Schedr DB connect error " + err + ": " + cng.dbConnCng.connectStr); 
    
  Executor executor(app, *dbNewTask);
  
  // schedr from DB
  CHECK_RETURN(!executor.getSchedrFromDB(cng.remoteConnPnt, *dbNewTask), "Schedr not found in DB for connectPnt " + cng.remoteConnPnt);
     
  // prev tasks and workers
  executor.getPrevTaskFromDB(*dbNewTask);
  executor.getPrevWorkersFromDB(*dbNewTask);
  executor.listenNewTask(*dbNewTask, true);
   
  // TCP server
  Tcp::ReceiveDataCBack receiveDataCB = [&executor](const string& cp, const string& data){
    executor.receiveHandler(cp, data);
  };
  Tcp::SendStatusCBack sendStatusCB = [&executor](const string& cp, const string& data, const error_code& ec){
    executor.sendNotifyHandler(cp, data, ec);
  };  
  CHECK_RETURN(!Tcp::startServer(cng.localConnPnt, receiveDataCB, sendStatusCB, 0, err), 
    "Schedr error: " + cng.localConnPnt + " " + err);
  app.statusMess("Schedr running: " + cng.localConnPnt);
  
  // on start
  executor.addMessToDB(DB::MessSchedr{ base::MessType::START_SCHEDR });

  // loop ///////////////////////////////////////////////////////////////////////
  Loop loop(cng, executor, *dbNewTask, *dbSendMess);

  Application::SignalConnector.connectSlot(Application::SIGNAL_LOOP_NOTIFY, 
    std::function<void()>([&loop]() {
      loop.standUpNotify();
  }));

  Application::SignalConnector.connectSlot(Application::SIGNAL_LOOP_STOP, 
    std::function<void()>([&loop]() {
      loop.stop();
  }));

  try{
    loop.run();
  }
  catch(exception& e){
    string mess = "loop exeption: " + string(e.what());
    executor.addMessToDB(DB::MessSchedr::errorMess(0, mess));
    app.statusMess(mess);  
  }

  /////////////////////////////////////////////////////////////////////////
  
  Tcp::stopServer();
  executor.listenNewTask(*dbNewTask, false);
  executor.stopSchedr(*dbSendMess);
}

void closeHandler(int sig)
{
  Application::loopStop();
}

unique_ptr<DB::DbProvider> 
createDbProvider(const Application::Config& cng, string& err)
{
  unique_ptr<DB::DbProvider> db(new DB::DbProvider(cng.dbConnCng));
  err = db->getLastError();
  if (err.empty()){
    return db;
  } else{    
    return nullptr;
  }
}

