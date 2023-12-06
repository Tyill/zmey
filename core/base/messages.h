#pragma once

#include "base.h"

#include <cstring>
#include <vector>

namespace mess
{
enum class MessType{
  UNDEFINED = 0,
  NEW_TASK,
  TASK_RUNNING,
  TASK_ERROR,
  TASK_COMPLETED,
  TASK_PAUSE,
  TASK_START,
  TASK_STOP,
  TASK_CONTINUE,
  TASK_CANCEL, // when not yet taken to work
  TASK_PROGRESS,
  START_WORKER,
  START_SCHEDR, 
  STOP_WORKER,
  STOP_SCHEDR,
  PAUSE_WORKER,
  PAUSE_SCHEDR, 
  START_AFTER_PAUSE_WORKER,
  START_AFTER_PAUSE_SCHEDR,  
  PING_WORKER,
  PING_SCHEDR,
  JUST_START_WORKER,
  WORKER_NOT_RESPONDING,
  INTERN_ERROR,      
};

MessType getMessType(const std::string& m);
std::string getConnectPnt(const std::string& m);

class NewTask{
public:  
  NewTask(const std::string& connPnt);
  
  std::string connectPnt;
  std::string params;
  std::string scriptPath;     
  std::string resultPath;     
  int taskId{};

  std::string serialn();
  bool deserialn(const std::string& m);

private:
  MessType mtype = MessType::NEW_TASK;
};

class TaskStatus{
public:  
  TaskStatus(MessType _mtype, const std::string& connPnt);
  TaskStatus(int taskId = 0, MessType _mtype = MessType::UNDEFINED);

  MessType mtype = MessType::UNDEFINED;
  std::string connectPnt;
  int taskId{};
  int activeTaskCount{};
  int loadCPU{};
  
  std::string serialn();
  bool deserialn(const std::string& m); 
};

class TaskProgress{
public:  
  TaskProgress(MessType _mtype, const std::string& connPnt);
  TaskProgress(const std::vector<int>& taskIds, const std::vector<int>& taskProgress, MessType _mtype = MessType::UNDEFINED);

  MessType mtype = MessType::UNDEFINED;
  std::string connectPnt;
  std::vector<int> taskIds;
  std::vector<int> taskProgress;
  int activeTaskCount{};
  int loadCPU{};
  
  std::string serialn();
  bool deserialn(const std::string& m); 
};
    
class InfoMess{
public:  
  InfoMess(MessType _mtype, const std::string& connPnt);

  MessType mtype = MessType::UNDEFINED;    
  std::string connectPnt;
  
  std::string serialn();
  bool deserialn(const std::string& m);
};

class InternError{
public:  
  InternError(const std::string& connPnt, const std::string& mess = "");
    
  std::string connectPnt;
  std::string message;
  
  std::string serialn();
  bool deserialn(const std::string& m);

private:
  MessType mtype = MessType::INTERN_ERROR;
};

} // namespace mess
      
