#include "base.h"

#include <cstring>

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

class AbstractMess{
public:
  AbstractMess(base::MessType type): mtype(type){}
  virtual ~AbstractMess(){}
  base::MessType mtype{};
  bool isValid{};

  virtual std::string serialn() = 0;
  virtual bool deserialn() = 0;  
};

class NewTask : public AbstractMess{
public:
  NewTask(const std::string& m):
    AbstractMess(base::MessType::NEW_TASK){
    isValid = deserialn(m);
  }
  std::string connectPnt;
  std::string params;
  std::string scriptPath;     
  std::string resultPath;     
  int taskId{};
  int averDurationSec{};
  int maxDurationSec{};

  std::string serialn() override{
    const int intSz = 4,
              connectPntSz = connectPnt.size(),
              paramsSz = params.size(),
              scriptPathSz = scriptPath.size(),
              resultPathSz = resultPath.size(),
              valsSz = connectPntSz + 
                       paramsSz + 
                       scriptPathSz +
                       resultPathSz +
                       intSz + // taskId
                       intSz + // averDurationSec
                       intSz,  // maxDurationSec
              strValsCount = 4,
              //      allSz   mtype             stringSize
              allSz = intSz + intSz + valsSz + strValsCount * intSz;
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
              allSz = 4,
              typeSz = 4,
              msz = int(m.size()); 
    char* pData = (char*)m.data();
    if (msz < (allSz + typeSz) || 
        msz != *(int*)(pData) || 
        *(int*)(pData + allSz) != int(mtype)){
      return false;
    }  
    
    int offs = allSz + typeSz;  

    int valSz = *((int*)(pData + offs));            offs += intSz; 
    connectPnt = std::string(pData + offs, valSz);  offs += valSz; 

    valSz = *((int*)(pData + offs));                offs += intSz; 
    params = std::string(pData + offs, valSz);      offs += valSz; 

    valSz = *((int*)(pData + offs));                offs += intSz; 
    scriptPath = std::string(pData + offs, valSz);  offs += valSz; 

    valSz = *((int*)(pData + offs));                offs += intSz; 
    resultPath = std::string(pData + offs, valSz);  offs += valSz; 

    taskId = *((int*)(pData + offs));               offs += intSz; 
    averDurationSec = *((int*)(pData + offs));      offs += intSz; 
    maxDurationSec = *((int*)(pData + offs));       offs += intSz; 
    
    return true;
  }        
};

class PingWorker : public AbstractMess{
public:
  PingWorker(const std::string& m):
    AbstractMess(base::MessType::PING_WORKER){
    isValid = deserialn(m);
  }
  std::string connectPnt;
  int activeTaskCount{};
  int cpuLoad{};

  std::string serialn() override{
    const int intSz = 4,
              connectPntSz = connectPnt.size(),
              valsSz = connectPntSz + 
                       intSz + // activeTaskCount
                       intSz,  // cpuLoad
              strValsCount = 1,
              //      allSz   mtype             stringSize
              allSz = intSz + intSz + valsSz + strValsCount * intSz;
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
    *((int*)(pOut + offs)) = cpuLoad;                      offs += intSz;

    return out;
  }

  bool deserialn(const std::string& m){

    const int intSz = 4,
              allSz = 4,
              typeSz = 4,
              msz = int(m.size()); 
    char* pData = (char*)m.data();
    if (msz < (allSz + typeSz) || 
        msz != *(int*)(pData) || 
        *(int*)(pData + allSz) != int(mtype)){
      return false;
    }  
    
    int offs = allSz + typeSz;  

    int valSz = *((int*)(pData + offs));            offs += intSz; 
    connectPnt = std::string(pData + offs, valSz);  offs += valSz; 
   
    activeTaskCount = *((int*)(pData + offs));      offs += intSz; 
    cpuLoad = *((int*)(pData + offs));              offs += intSz; 
    
    return true;
  }        
};
    
} // namespace mess

