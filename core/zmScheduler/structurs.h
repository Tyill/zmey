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
#pragma once

#include "zmBase/structurs.h"

void statusMess(const std::string& mess);

void mainCycleNotify();

struct SWorker{
  ZM_Base::Worker base;
  ZM_Base::StateType stateMem;
  bool isActive;
};

struct Config{
  int checkWorkerTOutSec = 120;
  string localConnPnt;
  string remoteConnPnt;
  ZM_DB::ConnectCng dbConnCng;
};

class BaseScheduler{
public:
  BaseScheduler();
  
  static void receiveHandler(const string& cp, const string& data);
  static void sendHandler(const string& cp, const string& data, const std::error_code& ec);
  static void getNewTaskFromDB(ZM_DB::DbProvider& db);
  static bool sendTaskToWorker(const ZM_Base::Scheduler&, map<string, SWorker>&, ZM_Aux::Queue<ZM_Base::Task>&, ZM_Aux::Queue<ZM_DB::MessSchedr>& messToDB);
  static void sendAllMessToDB(ZM_DB::DbProvider& db);
  static void checkStatusWorkers(const ZM_Base::Scheduler&, map<string, SWorker>&, ZM_Aux::Queue<ZM_DB::MessSchedr>&);
  static void getPrevTaskFromDB(ZM_DB::DbProvider& db, const ZM_Base::Scheduler&,  ZM_Aux::Queue<ZM_Base::Task>&);
  static void getPrevWorkersFromDB(ZM_DB::DbProvider& db, const ZM_Base::Scheduler&, map<string, SWorker>&);
  
public:
  static std::map<std::string, SWorker> workers;   // key - connectPnt
  static ZM_Aux::Queue<ZM_Base::Task> tasks;
  static ZM_Aux::Queue<ZM_DB::MessSchedr> messToDB;
  static ZM_Base::Scheduler schedr;  
};