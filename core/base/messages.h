#include "base.h"

#include <cstring>
#include <vector>

namespace mess
{
  base::MessType getMessType(const std::string& m);
  std::string getConnectPnt(const std::string& m);

class NewTask{
public:  
  NewTask(const std::string& connPnt);
  
  std::string connectPnt;
  std::string params;
  std::string scriptPath;     
  std::string resultPath;     
  int taskId{};
  int averDurationSec{};
  int maxDurationSec{};

  std::string serialn();
  bool deserialn(const std::string& m);

private:
  base::MessType mtype = base::MessType::NEW_TASK;
};

class TaskStatus{
public:  
  TaskStatus(base::MessType _mtype, const std::string& connPnt);

  base::MessType mtype = base::MessType::UNDEFINED;
  std::string connectPnt;
  int taskId{};
  int activeTaskCount{};
  int loadCPU{};
  
  std::string serialn();
  bool deserialn(const std::string& m); 
};

class TaskProgress{
public:  
  TaskProgress(const std::string& connPnt, const std::vector<std::pair<int,int>>& progress = {});

  std::string connectPnt;
  std::vector<std::pair<int,int>> taskProgress; // first - tId, second - progress
  
  std::string serialn();
  bool deserialn(const std::string& m);

private:
  base::MessType mtype = base::MessType::TASK_PROGRESS;
};
    
class InfoMess{
public:  
  InfoMess(base::MessType _mtype, const std::string& connPnt);

  base::MessType mtype = base::MessType::UNDEFINED;    
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
  base::MessType mtype = base::MessType::INTERN_ERROR;
};

} // namespace mess
      
