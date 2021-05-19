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

#include "zmCommon/tcp.h"
#include "application.h"
#include "executor.h"
#include "loop.h"
#include "process.h"

#include <signal.h>

using namespace std;

void closeHandler(int sig);
void loopNotify(int sig);

#define CHECK_RETURN(fun, mess) \
  if (fun){                     \
    app.statusMess(mess);       \
    return -1;                  \
  }

int main(int argc, char* argv[]){

  Application app;

  Application::Config cng;
  if (!app.parseArgs(argc, argv, cng)){
    return 0;
  }

  if (cng.remoteConnPnt.empty()){  // when without NAT
    cng.remoteConnPnt = cng.localConnPnt;
  } 

  CHECK_RETURN(cng.localConnPnt.empty() || (ZM_Aux::split(cng.localConnPnt, ':').size() != 2), "Not set param '--localAddr[-la]' - worker local connection point: IP or DNS:port");
  CHECK_RETURN(cng.remoteConnPnt.empty() || (ZM_Aux::split(cng.remoteConnPnt, ':').size() != 2), "Not set param '--remoteAddr[-ra]' - worker remote connection point: IP or DNS:port");
  CHECK_RETURN(cng.schedrConnPnt.empty() || (ZM_Aux::split(cng.schedrConnPnt, ':').size() != 2), "Not set param '--schedrAddr[-sa]' - scheduler connection point: IP or DNS:port");
    
  signal(SIGPIPE, SIG_IGN);
  signal(SIGCHLD, loopNotify);
  signal(SIGTERM, closeHandler);
  signal(SIGHUP, closeHandler);
  signal(SIGQUIT, closeHandler);

  Executor executor(app, cng.remoteConnPnt);
 
  // on start
  executor.addMessForSchedr(Executor::MessForSchedr{0, ZM_Base::MessType::JUST_START_WORKER});

  // TCP server
  ZM_Tcp::setReceiveCBack([&executor](const string& cp, const string& data){
    executor.receiveHandler(cp, data);
  });
  ZM_Tcp::setSendStatusCBack([&executor](const string& cp, const string& data, const error_code& ec){
    executor.sendNotifyHandler(cp, data, ec);
  });

  ZM_Tcp::addPreConnectPnt(cng.schedrConnPnt);
  string err;
  CHECK_RETURN(!ZM_Tcp::startServer(cng.localConnPnt, err, 1), "Worker error: " + cng.localConnPnt + " " + err);
  app.statusMess("Worker running: " + cng.localConnPnt);
  
  // loop ///////////////////////////////////////////////////////////////////////
  Loop loop(cng, executor);

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
    string mess = "worker::loop exeption: " + string(e.what());
    app.statusMess(mess);  
  }

  /////////////////////////////////////////////////////////////////////////
  
  ZM_Tcp::stopServer();
}

void closeHandler(int sig)
{
  Application::loopStop();
}

void loopNotify(int sig)
{
  Application::loopNotify();
}