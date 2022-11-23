#include "base.h"

#include <cstring>
#include <vector>

namespace mess
{

base::MessType deserialMessType(const std::string& m){
  const int allSz = 4,
            typeSz = 4,
            msz = int(m.size()); 
  char* pData = (char*)m.data();
  if (msz < (allSz + typeSz) || msz != *(int*)(pData)){
    return base::MessType::UNDEFINED;
  }  
  return base::MessType(*(int*)(pData + allSz));
}

class NewTask{
public:  
  NewTask(){
  }
  
  std::string connectPnt;
  std::string params;
  std::string scriptPath;     
  std::string resultPath;     
  int taskId{};
  int averDurationSec{};
  int maxDurationSec{};

  std::string serialn(){
    const int intSz = 4,
              connectPntSz = connectPnt.size(),
              paramsSz = params.size(),
              scriptPathSz = scriptPath.size(),
              resultPathSz = resultPath.size(),
              taskIdSz = intSz,
              averDurationSecSz = intSz,
              maxDurationSecSz = intSz,
              //      allSz   mtype             
              allSz = intSz + intSz + connectPntSz + intSz
                                    + paramsSz + intSz
                                    + scriptPathSz + intSz
                                    + resultPathSz + intSz
                                    + taskIdSz
                                    + averDurationSecSz
                                    + maxDurationSecSz;
    int offs = 0,
        inx = 0;
    std::string out;
    out.resize(allSz);
    char* pOut = (char*)out.data();
    *((int*)pOut) = allSz;                                 offs += intSz;                  
    *((int*)pOut) = int(mtype);                            offs += intSz;                  
    
    *((int*)(pOut + offs)) = connectPntSz;                 offs += intSz;
    memcpy(pOut + offs, connectPnt.data(), connectPntSz);  offs += connectPntSz;

    *((int*)(pOut + offs)) = paramsSz;                     offs += intSz;
    memcpy(pOut + offs, params.data(), paramsSz);          offs += paramsSz;

    *((int*)(pOut + offs)) = scriptPathSz;                 offs += intSz;
    memcpy(pOut + offs, scriptPath.data(), scriptPathSz);  offs += scriptPathSz;

    *((int*)(pOut + offs)) = resultPathSz;                 offs += intSz;
    memcpy(pOut + offs, resultPath.data(), resultPathSz);  offs += resultPathSz;

    *((int*)(pOut + offs)) = taskId;                       offs += intSz;
    *((int*)(pOut + offs)) = averDurationSec;              offs += intSz;
    *((int*)(pOut + offs)) = maxDurationSec;               offs += intSz;

    return out;
  }

  bool deserialn(const std::string& m){

    const int intSz = 4,
              allSz = intSz,
              typeSz = intSz,
              msz = int(m.size()); 
    char* pData = (char*)m.data();
    if (msz < (allSz + typeSz) || 
        msz != *(int*)(pData) || 
        *(int*)(pData + allSz) != int(mtype)){
      return false;
    }  
    
#define CHECK_MSIZE_RETURN(step) \
    if (offs + step > msz) return false;

    int offs = allSz + typeSz;                                     CHECK_MSIZE_RETURN(intSz)
    
    int valSz = *((int*)(pData + offs));            offs += intSz; CHECK_MSIZE_RETURN(valSz)
    connectPnt = std::string(pData + offs, valSz);  offs += valSz; CHECK_MSIZE_RETURN(intSz)

    valSz = *((int*)(pData + offs));                offs += intSz; CHECK_MSIZE_RETURN(valSz)
    params = std::string(pData + offs, valSz);      offs += valSz; CHECK_MSIZE_RETURN(intSz)

    valSz = *((int*)(pData + offs));                offs += intSz; CHECK_MSIZE_RETURN(valSz)
    scriptPath = std::string(pData + offs, valSz);  offs += valSz; CHECK_MSIZE_RETURN(intSz)

    valSz = *((int*)(pData + offs));                offs += intSz; CHECK_MSIZE_RETURN(valSz)
    resultPath = std::string(pData + offs, valSz);  offs += valSz; CHECK_MSIZE_RETURN(intSz)

    taskId = *((int*)(pData + offs));               offs += intSz; CHECK_MSIZE_RETURN(intSz)
    averDurationSec = *((int*)(pData + offs));      offs += intSz; CHECK_MSIZE_RETURN(intSz)
    maxDurationSec = *((int*)(pData + offs));       offs += intSz; 
    
#undef CHECK_MSIZE_RETURN
    
    return true;
  }        

private:
  base::MessType mtype = base::MessType::NEW_TASK;
};

class TaskRunning{
public:  
  TaskRunning(){
  }  
  std::string connectPnt;
  int taskId{};
  int activeTaskCount{};
  int loadCPU{};
  
  std::string serialn(){
    const int intSz = 4,
              connectPntSz = connectPnt.size(),
              taskIdSz = intSz,
              activeTaskCountSz = intSz,
              loadCPUSz = intSz,
              //      allSz   mtype             
              allSz = intSz + intSz + connectPntSz + intSz 
                                    + taskIdSz
                                    + activeTaskCountSz
                                    + loadCPUSz;
    int offs = 0,
        inx = 0;
    std::string out;
    out.resize(allSz);
    char* pOut = (char*)out.data();

    *((int*)pOut) = allSz;                                 offs += intSz;                  
    *((int*)pOut) = int(mtype);                            offs += intSz;                  
    
    *((int*)(pOut + offs)) = connectPntSz;                 offs += intSz;
    memcpy(pOut + offs, connectPnt.data(), connectPntSz);  offs += connectPntSz;

    *((int*)(pOut + offs)) = taskId;                       offs += intSz;
    *((int*)(pOut + offs)) = activeTaskCount;              offs += intSz;
    *((int*)(pOut + offs)) = loadCPU;                      offs += intSz;
  
    return out;
  }

  bool deserialn(const std::string& m){

    const int intSz = 4,
              allSz = intSz,
              typeSz = intSz,
              msz = int(m.size()); 
    char* pData = (char*)m.data();
    if (msz < (allSz + typeSz) || 
        msz != *(int*)(pData) || 
        *(int*)(pData + allSz) != int(mtype)){
      return false;
    }  
    
#define CHECK_MSIZE_RETURN(step) \
    if (offs + step > msz) return false;

    int offs = allSz + typeSz;                                     CHECK_MSIZE_RETURN(intSz)
    
    int valSz = *((int*)(pData + offs));            offs += intSz; CHECK_MSIZE_RETURN(valSz)
    connectPnt = std::string(pData + offs, valSz);  offs += valSz; CHECK_MSIZE_RETURN(intSz)
  
    taskId = *((int*)(pData + offs));               offs += intSz; CHECK_MSIZE_RETURN(intSz)
    activeTaskCount = *((int*)(pData + offs));      offs += intSz; CHECK_MSIZE_RETURN(intSz)
    loadCPU = *((int*)(pData + offs));            
        
#undef CHECK_MSIZE_RETURN
    
    return true;
  }     

private:
  base::MessType mtype = base::MessType::TASK_RUNNING;
};

class TaskError{
public:  
  TaskError(){
  }  
  std::string connectPnt;
  int taskId{};
  int activeTaskCount{};
  int loadCPU{};
  
  std::string serialn(){
    const int intSz = 4,
              connectPntSz = connectPnt.size(),
              taskIdSz = intSz,
              activeTaskCountSz = intSz,
              loadCPUSz = intSz,
              //      allSz   mtype             
              allSz = intSz + intSz + connectPntSz + intSz 
                                    + taskIdSz
                                    + activeTaskCountSz
                                    + loadCPUSz;
    int offs = 0,
        inx = 0;
    std::string out;
    out.resize(allSz);
    char* pOut = (char*)out.data();

    *((int*)pOut) = allSz;                                 offs += intSz;                  
    *((int*)pOut) = int(mtype);                            offs += intSz;                  
    
    *((int*)(pOut + offs)) = connectPntSz;                 offs += intSz;
    memcpy(pOut + offs, connectPnt.data(), connectPntSz);  offs += connectPntSz;

    *((int*)(pOut + offs)) = taskId;                       offs += intSz;
    *((int*)(pOut + offs)) = activeTaskCount;              offs += intSz;
    *((int*)(pOut + offs)) = loadCPU;                      offs += intSz;
  
    return out;
  }

  bool deserialn(const std::string& m){

    const int intSz = 4,
              allSz = intSz,
              typeSz = intSz,
              msz = int(m.size()); 
    char* pData = (char*)m.data();
    if (msz < (allSz + typeSz) || 
        msz != *(int*)(pData) || 
        *(int*)(pData + allSz) != int(mtype)){
      return false;
    }  
    
#define CHECK_MSIZE_RETURN(step) \
    if (offs + step > msz) return false;

    int offs = allSz + typeSz;                                     CHECK_MSIZE_RETURN(intSz)
    
    int valSz = *((int*)(pData + offs));            offs += intSz; CHECK_MSIZE_RETURN(valSz)
    connectPnt = std::string(pData + offs, valSz);  offs += valSz; CHECK_MSIZE_RETURN(intSz)
  
    taskId = *((int*)(pData + offs));               offs += intSz; CHECK_MSIZE_RETURN(intSz)
    activeTaskCount = *((int*)(pData + offs));      offs += intSz; CHECK_MSIZE_RETURN(intSz)
    loadCPU = *((int*)(pData + offs));            
        
#undef CHECK_MSIZE_RETURN
    
    return true;
  }     

private:
  base::MessType mtype = base::MessType::TASK_ERROR;
};

class TaskCompleted{
public:  
  TaskCompleted(){
  }  
  std::string connectPnt;
  int taskId{};
  int activeTaskCount{};
  int loadCPU{};
  
  std::string serialn(){
    const int intSz = 4,
              connectPntSz = connectPnt.size(),
              taskIdSz = intSz,
              activeTaskCountSz = intSz,
              loadCPUSz = intSz,
              //      allSz   mtype             
              allSz = intSz + intSz + connectPntSz + intSz 
                                    + taskIdSz
                                    + activeTaskCountSz
                                    + loadCPUSz;
    int offs = 0,
        inx = 0;
    std::string out;
    out.resize(allSz);
    char* pOut = (char*)out.data();

    *((int*)pOut) = allSz;                                 offs += intSz;                  
    *((int*)pOut) = int(mtype);                            offs += intSz;                  
    
    *((int*)(pOut + offs)) = connectPntSz;                 offs += intSz;
    memcpy(pOut + offs, connectPnt.data(), connectPntSz);  offs += connectPntSz;

    *((int*)(pOut + offs)) = taskId;                       offs += intSz;
    *((int*)(pOut + offs)) = activeTaskCount;              offs += intSz;
    *((int*)(pOut + offs)) = loadCPU;                      offs += intSz;
  
    return out;
  }

  bool deserialn(const std::string& m){

    const int intSz = 4,
              allSz = intSz,
              typeSz = intSz,
              msz = int(m.size()); 
    char* pData = (char*)m.data();
    if (msz < (allSz + typeSz) || 
        msz != *(int*)(pData) || 
        *(int*)(pData + allSz) != int(mtype)){
      return false;
    }  
    
#define CHECK_MSIZE_RETURN(step) \
    if (offs + step > msz) return false;

    int offs = allSz + typeSz;                                     CHECK_MSIZE_RETURN(intSz)
    
    int valSz = *((int*)(pData + offs));            offs += intSz; CHECK_MSIZE_RETURN(valSz)
    connectPnt = std::string(pData + offs, valSz);  offs += valSz; CHECK_MSIZE_RETURN(intSz)
  
    taskId = *((int*)(pData + offs));               offs += intSz; CHECK_MSIZE_RETURN(intSz)
    activeTaskCount = *((int*)(pData + offs));      offs += intSz; CHECK_MSIZE_RETURN(intSz)
    loadCPU = *((int*)(pData + offs));            
        
#undef CHECK_MSIZE_RETURN
    
    return true;
  }     

private:
  base::MessType mtype = base::MessType::TASK_COMPLETED;
};

class TaskPause{
public:  
  TaskPause(){
  }  
  std::string connectPnt;
  int taskId{};
  int activeTaskCount{};
  int loadCPU{};
  
  std::string serialn(){
    const int intSz = 4,
              connectPntSz = connectPnt.size(),
              taskIdSz = intSz,
              activeTaskCountSz = intSz,
              loadCPUSz = intSz,
              //      allSz   mtype             
              allSz = intSz + intSz + connectPntSz + intSz 
                                    + taskIdSz
                                    + activeTaskCountSz
                                    + loadCPUSz;
    int offs = 0,
        inx = 0;
    std::string out;
    out.resize(allSz);
    char* pOut = (char*)out.data();

    *((int*)pOut) = allSz;                                 offs += intSz;                  
    *((int*)pOut) = int(mtype);                            offs += intSz;                  
    
    *((int*)(pOut + offs)) = connectPntSz;                 offs += intSz;
    memcpy(pOut + offs, connectPnt.data(), connectPntSz);  offs += connectPntSz;

    *((int*)(pOut + offs)) = taskId;                       offs += intSz;
    *((int*)(pOut + offs)) = activeTaskCount;              offs += intSz;
    *((int*)(pOut + offs)) = loadCPU;                      offs += intSz;
  
    return out;
  }

  bool deserialn(const std::string& m){

    const int intSz = 4,
              allSz = intSz,
              typeSz = intSz,
              msz = int(m.size()); 
    char* pData = (char*)m.data();
    if (msz < (allSz + typeSz) || 
        msz != *(int*)(pData) || 
        *(int*)(pData + allSz) != int(mtype)){
      return false;
    }  
    
#define CHECK_MSIZE_RETURN(step) \
    if (offs + step > msz) return false;

    int offs = allSz + typeSz;                                     CHECK_MSIZE_RETURN(intSz)
    
    int valSz = *((int*)(pData + offs));            offs += intSz; CHECK_MSIZE_RETURN(valSz)
    connectPnt = std::string(pData + offs, valSz);  offs += valSz; CHECK_MSIZE_RETURN(intSz)
  
    taskId = *((int*)(pData + offs));               offs += intSz; CHECK_MSIZE_RETURN(intSz)
    activeTaskCount = *((int*)(pData + offs));      offs += intSz; CHECK_MSIZE_RETURN(intSz)
    loadCPU = *((int*)(pData + offs));            
        
#undef CHECK_MSIZE_RETURN
    
    return true;
  }     

private:
  base::MessType mtype = base::MessType::TASK_PAUSE;
};

class TaskStart{
public:  
  TaskStart(){
  }  
  std::string connectPnt;
  int taskId{};
  int activeTaskCount{};
  int loadCPU{};
  
  std::string serialn(){
    const int intSz = 4,
              connectPntSz = connectPnt.size(),
              taskIdSz = intSz,
              activeTaskCountSz = intSz,
              loadCPUSz = intSz,
              //      allSz   mtype             
              allSz = intSz + intSz + connectPntSz + intSz 
                                    + taskIdSz
                                    + activeTaskCountSz
                                    + loadCPUSz;
    int offs = 0,
        inx = 0;
    std::string out;
    out.resize(allSz);
    char* pOut = (char*)out.data();

    *((int*)pOut) = allSz;                                 offs += intSz;                  
    *((int*)pOut) = int(mtype);                            offs += intSz;                  
    
    *((int*)(pOut + offs)) = connectPntSz;                 offs += intSz;
    memcpy(pOut + offs, connectPnt.data(), connectPntSz);  offs += connectPntSz;

    *((int*)(pOut + offs)) = taskId;                       offs += intSz;
    *((int*)(pOut + offs)) = activeTaskCount;              offs += intSz;
    *((int*)(pOut + offs)) = loadCPU;                      offs += intSz;
  
    return out;
  }

  bool deserialn(const std::string& m){

    const int intSz = 4,
              allSz = intSz,
              typeSz = intSz,
              msz = int(m.size()); 
    char* pData = (char*)m.data();
    if (msz < (allSz + typeSz) || 
        msz != *(int*)(pData) || 
        *(int*)(pData + allSz) != int(mtype)){
      return false;
    }  
    
#define CHECK_MSIZE_RETURN(step) \
    if (offs + step > msz) return false;

    int offs = allSz + typeSz;                                     CHECK_MSIZE_RETURN(intSz)
    
    int valSz = *((int*)(pData + offs));            offs += intSz; CHECK_MSIZE_RETURN(valSz)
    connectPnt = std::string(pData + offs, valSz);  offs += valSz; CHECK_MSIZE_RETURN(intSz)
  
    taskId = *((int*)(pData + offs));               offs += intSz; CHECK_MSIZE_RETURN(intSz)
    activeTaskCount = *((int*)(pData + offs));      offs += intSz; CHECK_MSIZE_RETURN(intSz)
    loadCPU = *((int*)(pData + offs));            
        
#undef CHECK_MSIZE_RETURN
    
    return true;
  }     

private:
  base::MessType mtype = base::MessType::TASK_START;
};

class TaskStop{
public:  
  TaskStop(){
  }  
  std::string connectPnt;
  int taskId{};
  int activeTaskCount{};
  int loadCPU{};
  
  std::string serialn(){
    const int intSz = 4,
              connectPntSz = connectPnt.size(),
              taskIdSz = intSz,
              activeTaskCountSz = intSz,
              loadCPUSz = intSz,
              //      allSz   mtype             
              allSz = intSz + intSz + connectPntSz + intSz 
                                    + taskIdSz
                                    + activeTaskCountSz
                                    + loadCPUSz;
    int offs = 0,
        inx = 0;
    std::string out;
    out.resize(allSz);
    char* pOut = (char*)out.data();

    *((int*)pOut) = allSz;                                 offs += intSz;                  
    *((int*)pOut) = int(mtype);                            offs += intSz;                  
    
    *((int*)(pOut + offs)) = connectPntSz;                 offs += intSz;
    memcpy(pOut + offs, connectPnt.data(), connectPntSz);  offs += connectPntSz;

    *((int*)(pOut + offs)) = taskId;                       offs += intSz;
    *((int*)(pOut + offs)) = activeTaskCount;              offs += intSz;
    *((int*)(pOut + offs)) = loadCPU;                      offs += intSz;
  
    return out;
  }

  bool deserialn(const std::string& m){

    const int intSz = 4,
              allSz = intSz,
              typeSz = intSz,
              msz = int(m.size()); 
    char* pData = (char*)m.data();
    if (msz < (allSz + typeSz) || 
        msz != *(int*)(pData) || 
        *(int*)(pData + allSz) != int(mtype)){
      return false;
    }  
    
#define CHECK_MSIZE_RETURN(step) \
    if (offs + step > msz) return false;

    int offs = allSz + typeSz;                                     CHECK_MSIZE_RETURN(intSz)
    
    int valSz = *((int*)(pData + offs));            offs += intSz; CHECK_MSIZE_RETURN(valSz)
    connectPnt = std::string(pData + offs, valSz);  offs += valSz; CHECK_MSIZE_RETURN(intSz)
  
    taskId = *((int*)(pData + offs));               offs += intSz; CHECK_MSIZE_RETURN(intSz)
    activeTaskCount = *((int*)(pData + offs));      offs += intSz; CHECK_MSIZE_RETURN(intSz)
    loadCPU = *((int*)(pData + offs));            
        
#undef CHECK_MSIZE_RETURN
    
    return true;
  }     

private:
  base::MessType mtype = base::MessType::TASK_STOP;
};

class TaskContinue{
public:  
  TaskContinue(){
  }  
  std::string connectPnt;
  int taskId{};
  int activeTaskCount{};
  int loadCPU{};
  
  std::string serialn(){
    const int intSz = 4,
              connectPntSz = connectPnt.size(),
              taskIdSz = intSz,
              activeTaskCountSz = intSz,
              loadCPUSz = intSz,
              //      allSz   mtype             
              allSz = intSz + intSz + connectPntSz + intSz 
                                    + taskIdSz
                                    + activeTaskCountSz
                                    + loadCPUSz;
    int offs = 0,
        inx = 0;
    std::string out;
    out.resize(allSz);
    char* pOut = (char*)out.data();

    *((int*)pOut) = allSz;                                 offs += intSz;                  
    *((int*)pOut) = int(mtype);                            offs += intSz;                  
    
    *((int*)(pOut + offs)) = connectPntSz;                 offs += intSz;
    memcpy(pOut + offs, connectPnt.data(), connectPntSz);  offs += connectPntSz;

    *((int*)(pOut + offs)) = taskId;                       offs += intSz;
    *((int*)(pOut + offs)) = activeTaskCount;              offs += intSz;
    *((int*)(pOut + offs)) = loadCPU;                      offs += intSz;
  
    return out;
  }

  bool deserialn(const std::string& m){

    const int intSz = 4,
              allSz = intSz,
              typeSz = intSz,
              msz = int(m.size()); 
    char* pData = (char*)m.data();
    if (msz < (allSz + typeSz) || 
        msz != *(int*)(pData) || 
        *(int*)(pData + allSz) != int(mtype)){
      return false;
    }  
    
#define CHECK_MSIZE_RETURN(step) \
    if (offs + step > msz) return false;

    int offs = allSz + typeSz;                                     CHECK_MSIZE_RETURN(intSz)
    
    int valSz = *((int*)(pData + offs));            offs += intSz; CHECK_MSIZE_RETURN(valSz)
    connectPnt = std::string(pData + offs, valSz);  offs += valSz; CHECK_MSIZE_RETURN(intSz)
  
    taskId = *((int*)(pData + offs));               offs += intSz; CHECK_MSIZE_RETURN(intSz)
    activeTaskCount = *((int*)(pData + offs));      offs += intSz; CHECK_MSIZE_RETURN(intSz)
    loadCPU = *((int*)(pData + offs));            
        
#undef CHECK_MSIZE_RETURN
    
    return true;
  }     

private:
  base::MessType mtype = base::MessType::TASK_CONTINUE;
};
    
class TaskCancel{
public:  
  TaskCancel(){
  }  
  std::string connectPnt;
  int taskId{};
  int activeTaskCount{};
  int loadCPU{};
  
  std::string serialn(){
    const int intSz = 4,
              connectPntSz = connectPnt.size(),
              taskIdSz = intSz,
              activeTaskCountSz = intSz,
              loadCPUSz = intSz,
              //      allSz   mtype             
              allSz = intSz + intSz + connectPntSz + intSz 
                                    + taskIdSz
                                    + activeTaskCountSz
                                    + loadCPUSz;
    int offs = 0,
        inx = 0;
    std::string out;
    out.resize(allSz);
    char* pOut = (char*)out.data();

    *((int*)pOut) = allSz;                                 offs += intSz;                  
    *((int*)pOut) = int(mtype);                            offs += intSz;                  
    
    *((int*)(pOut + offs)) = connectPntSz;                 offs += intSz;
    memcpy(pOut + offs, connectPnt.data(), connectPntSz);  offs += connectPntSz;

    *((int*)(pOut + offs)) = taskId;                       offs += intSz;
    *((int*)(pOut + offs)) = activeTaskCount;              offs += intSz;
    *((int*)(pOut + offs)) = loadCPU;                      offs += intSz;
  
    return out;
  }

  bool deserialn(const std::string& m){

    const int intSz = 4,
              allSz = intSz,
              typeSz = intSz,
              msz = int(m.size()); 
    char* pData = (char*)m.data();
    if (msz < (allSz + typeSz) || 
        msz != *(int*)(pData) || 
        *(int*)(pData + allSz) != int(mtype)){
      return false;
    }  
    
#define CHECK_MSIZE_RETURN(step) \
    if (offs + step > msz) return false;

    int offs = allSz + typeSz;                                     CHECK_MSIZE_RETURN(intSz)
    
    int valSz = *((int*)(pData + offs));            offs += intSz; CHECK_MSIZE_RETURN(valSz)
    connectPnt = std::string(pData + offs, valSz);  offs += valSz; CHECK_MSIZE_RETURN(intSz)
  
    taskId = *((int*)(pData + offs));               offs += intSz; CHECK_MSIZE_RETURN(intSz)
    activeTaskCount = *((int*)(pData + offs));      offs += intSz; CHECK_MSIZE_RETURN(intSz)
    loadCPU = *((int*)(pData + offs));            
        
#undef CHECK_MSIZE_RETURN
    
    return true;
  }     

private:
  base::MessType mtype = base::MessType::TASK_CANCEL;
};

class TaskProgress{
public:  
  TaskProgress(){
  }  
  std::string connectPnt;
  std::vector<std::pair<int,int>> taskProgress; // first - tId, second - progress
  
  std::string serialn(){
    const int intSz = 4,
              connectPntSz = connectPnt.size(),
              taskIdSz = intSz,
              taskProgressSz = intSz,
              taskCountSz = intSz,
              //      allSz   mtype             
              allSz = intSz + intSz + connectPntSz + intSz 
                                    + taskIdSz * taskCountSz
                                    + taskProgressSz * taskCountSz;
    int offs = 0,
        inx = 0;
    std::string out;
    out.resize(allSz);
    char* pOut = (char*)out.data();

    *((int*)pOut) = allSz;                                 offs += intSz;                  
    *((int*)pOut) = int(mtype);                            offs += intSz;                  
    
    *((int*)(pOut + offs)) = connectPntSz;                 offs += intSz;
    memcpy(pOut + offs, connectPnt.data(), connectPntSz);  offs += connectPntSz;

    *((int*)(pOut + offs)) = taskCountSz;                  offs += intSz;

    for (const auto& t : taskProgress){
      *((int*)(pOut + offs)) = t.first;                    offs += intSz;
      *((int*)(pOut + offs)) = t.second;                   offs += intSz;
    }
    return out;
  }

  bool deserialn(const std::string& m){

    const int intSz = 4,
              allSz = intSz,
              typeSz = intSz,
              msz = int(m.size()); 
    char* pData = (char*)m.data();
    if (msz < (allSz + typeSz) || 
        msz != *(int*)(pData) || 
        *(int*)(pData + allSz) != int(mtype)){
      return false;
    }  
    
#define CHECK_MSIZE_RETURN(step) \
    if (offs + step > msz) return false;

    int offs = allSz + typeSz;                                     CHECK_MSIZE_RETURN(intSz)
    
    int valSz = *((int*)(pData + offs));            offs += intSz; CHECK_MSIZE_RETURN(valSz)
    connectPnt = std::string(pData + offs, valSz);  offs += valSz; CHECK_MSIZE_RETURN(intSz)
  
    int taskCount = *((int*)(pData + offs));        offs += intSz; CHECK_MSIZE_RETURN(intSz)
    
    for (int i = 0; i < taskCount; ++i){
      int taskId = *((int*)(pData + offs));         offs += intSz; CHECK_MSIZE_RETURN(intSz)
      int taskPrss = *((int*)(pData + offs));       offs += intSz; 
      
      taskProgress.push_back({taskId, taskPrss});

      if (i < taskCount - 1){
        CHECK_MSIZE_RETURN(intSz)
      }
    }        
        
#undef CHECK_MSIZE_RETURN
    
    return true;
  }     

private:
  base::MessType mtype = base::MessType::TASK_PROGRESS;
};
    
class StartWorker{
public:  
  StartWorker(){
  }  
    
  std::string connectPnt;
  
  std::string serialn(){
    const int intSz = 4,
              connectPntSz = connectPnt.size(),
              //      allSz   mtype             
              allSz = intSz + intSz + connectPntSz + intSz;

    int offs = 0,
        inx = 0;
    std::string out;
    out.resize(allSz);
    char* pOut = (char*)out.data();

    *((int*)pOut) = allSz;                                 offs += intSz;                  
    *((int*)pOut) = int(mtype);                            offs += intSz;                  
    
    *((int*)(pOut + offs)) = connectPntSz;                 offs += intSz;
    memcpy(pOut + offs, connectPnt.data(), connectPntSz);  offs += connectPntSz;
  
    return out;
  }

  bool deserialn(const std::string& m){

    const int intSz = 4,
              allSz = intSz,
              typeSz = intSz,
              msz = int(m.size()); 
    char* pData = (char*)m.data();
    if (msz < (allSz + typeSz) || 
        msz != *(int*)(pData) || 
        *(int*)(pData + allSz) != int(mtype)){
      return false;
    }  
    
#define CHECK_MSIZE_RETURN(step) \
    if (offs + step > msz) return false;

    int offs = allSz + typeSz;                                     CHECK_MSIZE_RETURN(intSz)
    
    int valSz = *((int*)(pData + offs));            offs += intSz; CHECK_MSIZE_RETURN(valSz)
    connectPnt = std::string(pData + offs, valSz);  offs += valSz;
             
#undef CHECK_MSIZE_RETURN
    
    return true;
  }       

private:
  base::MessType mtype = base::MessType::START_WORKER;
};

class StartScheduler{
public:  
  StartScheduler(){
  }  
    
   std::string connectPnt;
  
  std::string serialn(){
    const int intSz = 4,
              connectPntSz = connectPnt.size(),
              //      allSz   mtype             
              allSz = intSz + intSz + connectPntSz + intSz;

    int offs = 0,
        inx = 0;
    std::string out;
    out.resize(allSz);
    char* pOut = (char*)out.data();

    *((int*)pOut) = allSz;                                 offs += intSz;                  
    *((int*)pOut) = int(mtype);                            offs += intSz;                  
    
    *((int*)(pOut + offs)) = connectPntSz;                 offs += intSz;
    memcpy(pOut + offs, connectPnt.data(), connectPntSz);  offs += connectPntSz;
  
    return out;
  }

  bool deserialn(const std::string& m){

    const int intSz = 4,
              allSz = intSz,
              typeSz = intSz,
              msz = int(m.size()); 
    char* pData = (char*)m.data();
    if (msz < (allSz + typeSz) || 
        msz != *(int*)(pData) || 
        *(int*)(pData + allSz) != int(mtype)){
      return false;
    }  
    
#define CHECK_MSIZE_RETURN(step) \
    if (offs + step > msz) return false;

    int offs = allSz + typeSz;                                     CHECK_MSIZE_RETURN(intSz)
    
    int valSz = *((int*)(pData + offs));            offs += intSz; CHECK_MSIZE_RETURN(valSz)
    connectPnt = std::string(pData + offs, valSz);  offs += valSz;
             
#undef CHECK_MSIZE_RETURN
        
    return true;
  }     

private:
  base::MessType mtype = base::MessType::START_SCHEDR;
};

class StopWorker{
public:  
  StopWorker(){
  }  
    
   std::string connectPnt;
  
  std::string serialn(){
    const int intSz = 4,
              connectPntSz = connectPnt.size(),
              //      allSz   mtype             
              allSz = intSz + intSz + connectPntSz + intSz;

    int offs = 0,
        inx = 0;
    std::string out;
    out.resize(allSz);
    char* pOut = (char*)out.data();

    *((int*)pOut) = allSz;                                 offs += intSz;                  
    *((int*)pOut) = int(mtype);                            offs += intSz;                  
    
    *((int*)(pOut + offs)) = connectPntSz;                 offs += intSz;
    memcpy(pOut + offs, connectPnt.data(), connectPntSz);  offs += connectPntSz;
  
    return out;
  }

  bool deserialn(const std::string& m){

    const int intSz = 4,
              allSz = intSz,
              typeSz = intSz,
              msz = int(m.size()); 
    char* pData = (char*)m.data();
    if (msz < (allSz + typeSz) || 
        msz != *(int*)(pData) || 
        *(int*)(pData + allSz) != int(mtype)){
      return false;
    }  
    
#define CHECK_MSIZE_RETURN(step) \
    if (offs + step > msz) return false;

    int offs = allSz + typeSz;                                     CHECK_MSIZE_RETURN(intSz)
    
    int valSz = *((int*)(pData + offs));            offs += intSz; CHECK_MSIZE_RETURN(valSz)
    connectPnt = std::string(pData + offs, valSz);  offs += valSz;
             
#undef CHECK_MSIZE_RETURN
        
    return true;
  }     

private:
  base::MessType mtype = base::MessType::START_SCHEDR;
};

class StopScheduler{
public:  
  StopScheduler(){
  }  
    
  std::string connectPnt;
  
  std::string serialn(){
    const int intSz = 4,
              connectPntSz = connectPnt.size(),
              //      allSz   mtype             
              allSz = intSz + intSz + connectPntSz + intSz;

    int offs = 0,
        inx = 0;
    std::string out;
    out.resize(allSz);
    char* pOut = (char*)out.data();

    *((int*)pOut) = allSz;                                 offs += intSz;                  
    *((int*)pOut) = int(mtype);                            offs += intSz;                  
    
    *((int*)(pOut + offs)) = connectPntSz;                 offs += intSz;
    memcpy(pOut + offs, connectPnt.data(), connectPntSz);  offs += connectPntSz;
  
    return out;
  }

  bool deserialn(const std::string& m){

    const int intSz = 4,
              allSz = intSz,
              typeSz = intSz,
              msz = int(m.size()); 
    char* pData = (char*)m.data();
    if (msz < (allSz + typeSz) || 
        msz != *(int*)(pData) || 
        *(int*)(pData + allSz) != int(mtype)){
      return false;
    }  
    
#define CHECK_MSIZE_RETURN(step) \
    if (offs + step > msz) return false;

    int offs = allSz + typeSz;                                     CHECK_MSIZE_RETURN(intSz)
    
    int valSz = *((int*)(pData + offs));            offs += intSz; CHECK_MSIZE_RETURN(valSz)
    connectPnt = std::string(pData + offs, valSz);  offs += valSz;
             
#undef CHECK_MSIZE_RETURN
        
    return true;
  }     

private:
  base::MessType mtype = base::MessType::STOP_SCHEDR;
};

class PauseWorker{
public:  
  PauseWorker(){
  }  
    
  std::string connectPnt;
  
  std::string serialn(){
    const int intSz = 4,
              connectPntSz = connectPnt.size(),
              //      allSz   mtype             
              allSz = intSz + intSz + connectPntSz + intSz;

    int offs = 0,
        inx = 0;
    std::string out;
    out.resize(allSz);
    char* pOut = (char*)out.data();

    *((int*)pOut) = allSz;                                 offs += intSz;                  
    *((int*)pOut) = int(mtype);                            offs += intSz;                  
    
    *((int*)(pOut + offs)) = connectPntSz;                 offs += intSz;
    memcpy(pOut + offs, connectPnt.data(), connectPntSz);  offs += connectPntSz;
  
    return out;
  }

  bool deserialn(const std::string& m){

    const int intSz = 4,
              allSz = intSz,
              typeSz = intSz,
              msz = int(m.size()); 
    char* pData = (char*)m.data();
    if (msz < (allSz + typeSz) || 
        msz != *(int*)(pData) || 
        *(int*)(pData + allSz) != int(mtype)){
      return false;
    }  
    
#define CHECK_MSIZE_RETURN(step) \
    if (offs + step > msz) return false;

    int offs = allSz + typeSz;                                     CHECK_MSIZE_RETURN(intSz)
    
    int valSz = *((int*)(pData + offs));            offs += intSz; CHECK_MSIZE_RETURN(valSz)
    connectPnt = std::string(pData + offs, valSz);  offs += valSz;
             
#undef CHECK_MSIZE_RETURN
        
    return true;
  }     

private:
  base::MessType mtype = base::MessType::PAUSE_WORKER;
};

class PauseScheduler{
public:  
  PauseScheduler(){
  }  
    
  std::string connectPnt;
  
  std::string serialn(){
    const int intSz = 4,
              connectPntSz = connectPnt.size(),
              //      allSz   mtype             
              allSz = intSz + intSz + connectPntSz + intSz;

    int offs = 0,
        inx = 0;
    std::string out;
    out.resize(allSz);
    char* pOut = (char*)out.data();

    *((int*)pOut) = allSz;                                 offs += intSz;                  
    *((int*)pOut) = int(mtype);                            offs += intSz;                  
    
    *((int*)(pOut + offs)) = connectPntSz;                 offs += intSz;
    memcpy(pOut + offs, connectPnt.data(), connectPntSz);  offs += connectPntSz;
  
    return out;
  }

  bool deserialn(const std::string& m){

    const int intSz = 4,
              allSz = intSz,
              typeSz = intSz,
              msz = int(m.size()); 
    char* pData = (char*)m.data();
    if (msz < (allSz + typeSz) || 
        msz != *(int*)(pData) || 
        *(int*)(pData + allSz) != int(mtype)){
      return false;
    }  
    
#define CHECK_MSIZE_RETURN(step) \
    if (offs + step > msz) return false;

    int offs = allSz + typeSz;                                     CHECK_MSIZE_RETURN(intSz)
    
    int valSz = *((int*)(pData + offs));            offs += intSz; CHECK_MSIZE_RETURN(valSz)
    connectPnt = std::string(pData + offs, valSz);  offs += valSz;
             
#undef CHECK_MSIZE_RETURN
        
    return true;
  }     

private:
  base::MessType mtype = base::MessType::PAUSE_SCHEDR;
};

class StartAfterPauseWorker{
public:  
  StartAfterPauseWorker(){
  }  
    
  std::string connectPnt;
  
  std::string serialn(){
    const int intSz = 4,
              connectPntSz = connectPnt.size(),
              //      allSz   mtype             
              allSz = intSz + intSz + connectPntSz + intSz;

    int offs = 0,
        inx = 0;
    std::string out;
    out.resize(allSz);
    char* pOut = (char*)out.data();

    *((int*)pOut) = allSz;                                 offs += intSz;                  
    *((int*)pOut) = int(mtype);                            offs += intSz;                  
    
    *((int*)(pOut + offs)) = connectPntSz;                 offs += intSz;
    memcpy(pOut + offs, connectPnt.data(), connectPntSz);  offs += connectPntSz;
  
    return out;
  }

  bool deserialn(const std::string& m){

    const int intSz = 4,
              allSz = intSz,
              typeSz = intSz,
              msz = int(m.size()); 
    char* pData = (char*)m.data();
    if (msz < (allSz + typeSz) || 
        msz != *(int*)(pData) || 
        *(int*)(pData + allSz) != int(mtype)){
      return false;
    }  
    
#define CHECK_MSIZE_RETURN(step) \
    if (offs + step > msz) return false;

    int offs = allSz + typeSz;                                     CHECK_MSIZE_RETURN(intSz)
    
    int valSz = *((int*)(pData + offs));            offs += intSz; CHECK_MSIZE_RETURN(valSz)
    connectPnt = std::string(pData + offs, valSz);  offs += valSz;
             
#undef CHECK_MSIZE_RETURN
        
    return true;
  }     

private:
  base::MessType mtype = base::MessType::START_AFTER_PAUSE_WORKER;
};

class StartAfterPauseScheduler{
public:  
  StartAfterPauseScheduler(){
  }  
    
  std::string connectPnt;
  
  std::string serialn(){
    const int intSz = 4,
              connectPntSz = connectPnt.size(),
              //      allSz   mtype             
              allSz = intSz + intSz + connectPntSz + intSz;

    int offs = 0,
        inx = 0;
    std::string out;
    out.resize(allSz);
    char* pOut = (char*)out.data();

    *((int*)pOut) = allSz;                                 offs += intSz;                  
    *((int*)pOut) = int(mtype);                            offs += intSz;                  
    
    *((int*)(pOut + offs)) = connectPntSz;                 offs += intSz;
    memcpy(pOut + offs, connectPnt.data(), connectPntSz);  offs += connectPntSz;
  
    return out;
  }

  bool deserialn(const std::string& m){

    const int intSz = 4,
              allSz = intSz,
              typeSz = intSz,
              msz = int(m.size()); 
    char* pData = (char*)m.data();
    if (msz < (allSz + typeSz) || 
        msz != *(int*)(pData) || 
        *(int*)(pData + allSz) != int(mtype)){
      return false;
    }  
    
#define CHECK_MSIZE_RETURN(step) \
    if (offs + step > msz) return false;

    int offs = allSz + typeSz;                                     CHECK_MSIZE_RETURN(intSz)
    
    int valSz = *((int*)(pData + offs));            offs += intSz; CHECK_MSIZE_RETURN(valSz)
    connectPnt = std::string(pData + offs, valSz);  offs += valSz;
             
#undef CHECK_MSIZE_RETURN
        
    return true;
  }     

private:
  base::MessType mtype = base::MessType::START_AFTER_PAUSE_SCHEDR;
};

class PingWorker{
public:  
  PingWorker(){
  }  
  std::string connectPnt;
  int activeTaskCount{};
  int loadCPU{};
  
  std::string serialn(){
    const int intSz = 4,
              connectPntSz = connectPnt.size(),
              activeTaskCountSz = intSz,
              loadCPUSz = intSz,
              //      allSz   mtype             
              allSz = intSz + intSz + connectPntSz + intSz 
                                    + activeTaskCountSz
                                    + loadCPUSz;
    int offs = 0,
        inx = 0;
    std::string out;
    out.resize(allSz);
    char* pOut = (char*)out.data();

    *((int*)pOut) = allSz;                                 offs += intSz;                  
    *((int*)pOut) = int(mtype);                            offs += intSz;                  
    
    *((int*)(pOut + offs)) = connectPntSz;                 offs += intSz;
    memcpy(pOut + offs, connectPnt.data(), connectPntSz);  offs += connectPntSz;

    *((int*)(pOut + offs)) = activeTaskCount;              offs += intSz;
    *((int*)(pOut + offs)) = loadCPU;                      offs += intSz;
  
    return out;
  }

  bool deserialn(const std::string& m){

    const int intSz = 4,
              allSz = intSz,
              typeSz = intSz,
              msz = int(m.size()); 
    char* pData = (char*)m.data();
    if (msz < (allSz + typeSz) || 
        msz != *(int*)(pData) || 
        *(int*)(pData + allSz) != int(mtype)){
      return false;
    }  
    
#define CHECK_MSIZE_RETURN(step) \
    if (offs + step > msz) return false;

    int offs = allSz + typeSz;                                     CHECK_MSIZE_RETURN(intSz)
    
    int valSz = *((int*)(pData + offs));            offs += intSz; CHECK_MSIZE_RETURN(valSz)
    connectPnt = std::string(pData + offs, valSz);  offs += valSz; CHECK_MSIZE_RETURN(intSz)
  
    activeTaskCount = *((int*)(pData + offs));      offs += intSz; CHECK_MSIZE_RETURN(intSz)
    loadCPU = *((int*)(pData + offs));            
        
#undef CHECK_MSIZE_RETURN
    
    return true;
  }     

private:
  base::MessType mtype = base::MessType::PING_WORKER;
};

class PingScheduler{
public:  
  PingScheduler(){
  }  
  std::string connectPnt;
    
  std::string serialn(){
    const int intSz = 4,
              connectPntSz = connectPnt.size(),
              //      allSz   mtype             
              allSz = intSz + intSz + connectPntSz + intSz;

    int offs = 0,
        inx = 0;
    std::string out;
    out.resize(allSz);
    char* pOut = (char*)out.data();

    *((int*)pOut) = allSz;                                 offs += intSz;                  
    *((int*)pOut) = int(mtype);                            offs += intSz;                  
    
    *((int*)(pOut + offs)) = connectPntSz;                 offs += intSz;
    memcpy(pOut + offs, connectPnt.data(), connectPntSz);  offs += connectPntSz;
  
    return out;
  }

  bool deserialn(const std::string& m){

    const int intSz = 4,
              allSz = intSz,
              typeSz = intSz,
              msz = int(m.size()); 
    char* pData = (char*)m.data();
    if (msz < (allSz + typeSz) || 
        msz != *(int*)(pData) || 
        *(int*)(pData + allSz) != int(mtype)){
      return false;
    }  
    
#define CHECK_MSIZE_RETURN(step) \
    if (offs + step > msz) return false;

    int offs = allSz + typeSz;                                     CHECK_MSIZE_RETURN(intSz)
    
    int valSz = *((int*)(pData + offs));            offs += intSz; CHECK_MSIZE_RETURN(valSz)
    connectPnt = std::string(pData + offs, valSz);  offs += valSz;         
        
#undef CHECK_MSIZE_RETURN
    
    return true;
  }     

private:
  base::MessType mtype = base::MessType::PING_SCHEDR;
};

class JustStartWorker{
public:  
  JustStartWorker(){
  }  
    
  std::string connectPnt;
  
  std::string serialn(){
    const int intSz = 4,
              connectPntSz = connectPnt.size(),
              //      allSz   mtype             
              allSz = intSz + intSz + connectPntSz + intSz;

    int offs = 0,
        inx = 0;
    std::string out;
    out.resize(allSz);
    char* pOut = (char*)out.data();

    *((int*)pOut) = allSz;                                 offs += intSz;                  
    *((int*)pOut) = int(mtype);                            offs += intSz;                  
    
    *((int*)(pOut + offs)) = connectPntSz;                 offs += intSz;
    memcpy(pOut + offs, connectPnt.data(), connectPntSz);  offs += connectPntSz;
  
    return out;
  }

  bool deserialn(const std::string& m){

    const int intSz = 4,
              allSz = intSz,
              typeSz = intSz,
              msz = int(m.size()); 
    char* pData = (char*)m.data();
    if (msz < (allSz + typeSz) || 
        msz != *(int*)(pData) || 
        *(int*)(pData + allSz) != int(mtype)){
      return false;
    }  
    
#define CHECK_MSIZE_RETURN(step) \
    if (offs + step > msz) return false;

    int offs = allSz + typeSz;                                     CHECK_MSIZE_RETURN(intSz)
    
    int valSz = *((int*)(pData + offs));            offs += intSz; CHECK_MSIZE_RETURN(valSz)
    connectPnt = std::string(pData + offs, valSz);  offs += valSz;
             
#undef CHECK_MSIZE_RETURN
        
    return true;
  }     

private:
  base::MessType mtype = base::MessType::JUST_START_WORKER;
};

class WorkerNotResponding{
public:  
  WorkerNotResponding(){
  }  
    
  std::string connectPnt;
  
  std::string serialn(){
    const int intSz = 4,
              connectPntSz = connectPnt.size(),
              //      allSz   mtype             
              allSz = intSz + intSz + connectPntSz + intSz;

    int offs = 0,
        inx = 0;
    std::string out;
    out.resize(allSz);
    char* pOut = (char*)out.data();

    *((int*)pOut) = allSz;                                 offs += intSz;                  
    *((int*)pOut) = int(mtype);                            offs += intSz;                  
    
    *((int*)(pOut + offs)) = connectPntSz;                 offs += intSz;
    memcpy(pOut + offs, connectPnt.data(), connectPntSz);  offs += connectPntSz;
  
    return out;
  }

  bool deserialn(const std::string& m){

    const int intSz = 4,
              allSz = intSz,
              typeSz = intSz,
              msz = int(m.size()); 
    char* pData = (char*)m.data();
    if (msz < (allSz + typeSz) || 
        msz != *(int*)(pData) || 
        *(int*)(pData + allSz) != int(mtype)){
      return false;
    }  
    
#define CHECK_MSIZE_RETURN(step) \
    if (offs + step > msz) return false;

    int offs = allSz + typeSz;                                     CHECK_MSIZE_RETURN(intSz)
    
    int valSz = *((int*)(pData + offs));            offs += intSz; CHECK_MSIZE_RETURN(valSz)
    connectPnt = std::string(pData + offs, valSz);  offs += valSz;
             
#undef CHECK_MSIZE_RETURN
        
    return true;
  }     

private:
  base::MessType mtype = base::MessType::WORKER_NOT_RESPONDING;
};

class InternError{
public:  
  InternError(){
  }  
    
  std::string connectPnt;
  std::string message;
  
  std::string serialn(){
    const int intSz = 4,
              connectPntSz = connectPnt.size(),
              messageSz = message.size(),
              //      allSz   mtype             
              allSz = intSz + intSz + connectPntSz + intSz
                                    + messageSz + intSz;

    int offs = 0,
        inx = 0;
    std::string out;
    out.resize(allSz);
    char* pOut = (char*)out.data();

    *((int*)pOut) = allSz;                                 offs += intSz;                  
    *((int*)pOut) = int(mtype);                            offs += intSz;                  
    
    *((int*)(pOut + offs)) = connectPntSz;                 offs += intSz;
    memcpy(pOut + offs, connectPnt.data(), connectPntSz);  offs += connectPntSz;

    *((int*)(pOut + offs)) = messageSz;                    offs += intSz;
    memcpy(pOut + offs, message.data(), messageSz);        offs += messageSz;
  
    return out;
  }

  bool deserialn(const std::string& m){

    const int intSz = 4,
              allSz = intSz,
              typeSz = intSz,
              msz = int(m.size()); 
    char* pData = (char*)m.data();
    if (msz < (allSz + typeSz) || 
        msz != *(int*)(pData) || 
        *(int*)(pData + allSz) != int(mtype)){
      return false;
    }  
    
#define CHECK_MSIZE_RETURN(step) \
    if (offs + step > msz) return false;

    int offs = allSz + typeSz;                                     CHECK_MSIZE_RETURN(intSz)
    
    int valSz = *((int*)(pData + offs));            offs += intSz; CHECK_MSIZE_RETURN(valSz)
    connectPnt = std::string(pData + offs, valSz);  offs += valSz; CHECK_MSIZE_RETURN(intSz)

    valSz = *((int*)(pData + offs));                offs += intSz; CHECK_MSIZE_RETURN(valSz)
    message = std::string(pData + offs, valSz); 
             
#undef CHECK_MSIZE_RETURN
        
    return true;
  }     

private:
  base::MessType mtype = base::MessType::INTERN_ERROR;
};

} // namespace mess

