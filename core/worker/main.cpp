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
#include "process.h"

#include <signal.h>

using namespace std;

static Loop* pLoop;
static void closeHandler(int sig)
{
  if (pLoop) pLoop->stop();
}

static void loopNotify(int sig)
{
  if (pLoop) pLoop->standUpNotify();
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

#define CHECK_RETURN(fun, mess) \
  if (fun){                     \
    app.statusMess(mess);       \
    return -1;                  \
  }
  CHECK_RETURN(cng.localConnPnt.empty() || (misc::split(cng.localConnPnt, ':').size() != 2), "Not set param '--localAddr[-la]' - worker local connection point: IP or DNS:port");
  CHECK_RETURN(cng.remoteConnPnt.empty() || (misc::split(cng.remoteConnPnt, ':').size() != 2), "Not set param '--remoteAddr[-ra]' - worker remote connection point: IP or DNS:port");
  CHECK_RETURN(cng.schedrConnPnt.empty() || (misc::split(cng.schedrConnPnt, ':').size() != 2), "Not set param '--schedrAddr[-sa]' - scheduler connection point: IP or DNS:port");
  
  signal(SIGTERM, closeHandler);
#ifdef __linux__
  signal(SIGPIPE, SIG_IGN);
  signal(SIGCHLD, loopNotify);
  signal(SIGHUP, closeHandler);
  signal(SIGQUIT, closeHandler);
#endif
 
  Executor executor(app, cng.remoteConnPnt);
 
  // on start
  executor.addMessForSchedr(Executor::MessForSchedr{0, mess::MessType::JUST_START_WORKER});

  // TCP server
  misc::ReceiveDataCBack receiveDataCB = [&executor](const string& cp, const string& data){
    executor.receiveHandler(cp, data);
  };
  misc::ErrorStatusCBack sendStatusCB = [&executor](const string& cp, const error_code& ec){
    executor.sendNotifyHandler(cp, ec);
  };
  string err;
  CHECK_RETURN(!misc::startServer(cng.localConnPnt, receiveDataCB, sendStatusCB, 1, err),
    "Worker error: " + cng.localConnPnt + " " + err);
  app.statusMess("Worker running: " + cng.localConnPnt);
  
  // loop ///////////////////////////////////////////////////////////////////////
  Loop loop(cng, executor);
  executor.setLoop(&loop);

  loop.run();
  
  /////////////////////////////////////////////////////////////////////////
  
  executor.stopToSchedr(cng.schedrConnPnt);

  misc::stopServer();
}
