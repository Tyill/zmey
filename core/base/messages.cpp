#include "base.h"
#include "messages.h"

#include <cstring>
#include <vector>
#include <tuple>
#include <type_traits>

namespace mess
{

#define CHECK_MSIZE_RETURN(step) \
  if (offs + step > msz) return false;

mess::MessType getMessType(const std::string& m){
  const int intSz = 4,
            allSz = intSz,
            typeSz = intSz,
            msz = int(m.size()); 
  char* pData = (char*)m.data();
  if (msz < (allSz + typeSz) || 
      msz != *(int*)(pData)){
    return mess::MessType::UNDEFINED;
  }
  return mess::MessType(*(int*)(pData + allSz));
}
std::string getConnectPnt(const std::string& m){
  const int intSz = 4,
            allSz = intSz,
            typeSz = intSz,
            msz = int(m.size()); 
  char* pData = (char*)m.data();
  if (msz < (allSz + typeSz) || 
      msz != *(int*)(pData)){
    return std::string();
  }
  int offs = allSz + typeSz;
  if (offs + intSz > msz) return std::string();
  
  int valSz = *((int*)(pData + offs));            
  offs += intSz;
  if (offs + valSz > msz) return std::string();
  
  return std::string(pData + offs, valSz);
}

NewTask::NewTask(const std::string& connPnt):
  connectPnt(connPnt)
{
}



template<typename... Fields>
void writeMessage(Fields... fields){
  auto t = std::make_tuple(std::forward<Args>(fields)...);  
  
  const int intSz = 4;
  int outSz = 0;
  for (int i = 0; i < std::tuple_size_v<t>; ++i){
    auto v = std::get<i>(t);
    if constexpr (std::is_same<std::decltype(v), std::string>){
      outSz += v.size();
    } 
    if constexpr (std::is_same<std::decltype(v), int>){
      outSz += intSz;
    } 
  }
  char out[outSz];  
  
  int offs = 0;
  *((int*)(pOut + offs)) = connectPntSz;                 offs += intSz_;
  memcpy(pOut + offs, connectPnt.data(), connectPntSz);  offs += connectPntSz;

  

 
}

std::string NewTask::serialn(){
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

bool NewTask::deserialn(const std::string& m){

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

  return true;
}        

TaskStatus::TaskStatus(mess::MessType _mtype, const std::string& connPnt):
  mtype(_mtype),
  connectPnt(connPnt)
{
}  

std::string TaskStatus::serialn(){
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

bool TaskStatus::deserialn(const std::string& m){

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

  int offs = allSz + typeSz;                                     CHECK_MSIZE_RETURN(intSz)
  
  int valSz = *((int*)(pData + offs));            offs += intSz; CHECK_MSIZE_RETURN(valSz)
  connectPnt = std::string(pData + offs, valSz);  offs += valSz; CHECK_MSIZE_RETURN(intSz)

  taskId = *((int*)(pData + offs));               offs += intSz; CHECK_MSIZE_RETURN(intSz)
  activeTaskCount = *((int*)(pData + offs));      offs += intSz; CHECK_MSIZE_RETURN(intSz)
  loadCPU = *((int*)(pData + offs));            
      
  return true;
}     

TaskProgress::TaskProgress(const std::string& connPnt, const std::vector<std::pair<int,int>>& progress):
  connectPnt(connPnt),
  taskProgress(progress)
{
}

std::string TaskProgress::serialn(){
  const int intSz = 4,
            connectPntSz = connectPnt.size(),
            taskIdSz = intSz,
            taskProgressSz = intSz,
            taskCount = taskProgress.size(),
            taskCountSz = intSz,
            //      allSz   mtype             
            allSz = intSz + intSz + connectPntSz + taskCountSz 
                                  + taskIdSz * taskCount
                                  + taskProgressSz * taskCount;
  int offs = 0,
      inx = 0;
  std::string out;
  out.resize(allSz);
  char* pOut = (char*)out.data();

  *((int*)pOut) = allSz;                                 offs += intSz;                  
  *((int*)pOut) = int(mtype);                            offs += intSz;                  
  
  *((int*)(pOut + offs)) = connectPntSz;                 offs += intSz;
  memcpy(pOut + offs, connectPnt.data(), connectPntSz);  offs += connectPntSz;

  *((int*)(pOut + offs)) = taskCount;                    offs += intSz;

  for (const auto& t : taskProgress){
    *((int*)(pOut + offs)) = t.first;                    offs += intSz;
    *((int*)(pOut + offs)) = t.second;                   offs += intSz;
  }
  return out;
}

bool TaskProgress::deserialn(const std::string& m){

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
      
  return true;
}     
  
InfoMess::InfoMess(mess::MessType _mtype, const std::string& connPnt):
  mtype(_mtype),
  connectPnt(connPnt)
{
} 

std::string InfoMess::serialn(){
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

bool InfoMess::deserialn(const std::string& m){

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

  int offs = allSz + typeSz;                                     CHECK_MSIZE_RETURN(intSz)
  
  int valSz = *((int*)(pData + offs));            offs += intSz; CHECK_MSIZE_RETURN(valSz)
  connectPnt = std::string(pData + offs, valSz);  offs += valSz;
      
  return true;
}       

InternError::InternError(const std::string& connPnt, const std::string& mess):
  connectPnt(connPnt),
  message(mess)
{
}
  
std::string InternError::serialn(){
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

bool InternError::deserialn(const std::string& m){

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

  int offs = allSz + typeSz;                                     CHECK_MSIZE_RETURN(intSz)
  
  int valSz = *((int*)(pData + offs));            offs += intSz; CHECK_MSIZE_RETURN(valSz)
  connectPnt = std::string(pData + offs, valSz);  offs += valSz; CHECK_MSIZE_RETURN(intSz)

  valSz = *((int*)(pData + offs));                offs += intSz; CHECK_MSIZE_RETURN(valSz)
  message = std::string(pData + offs, valSz); 
      
  return true;
}     

#undef CHECK_MSIZE_RETURN
} // namespace mess
      
