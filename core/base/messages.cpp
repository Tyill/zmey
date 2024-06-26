#include "base.h"
#include "messages.h"

#include <cstring>
#include <vector>
#include <tuple>
#include <type_traits>

namespace mess
{
template<typename... Fields>
class SerialReader{
public: 
    SerialReader(const std::string& m, Fields&... fields):
      in_(m){        
        const int allType = intSz_;
        inSize_ = int(m.size());         
        char* pData = (char*)m.data();
        if (inSize_ < allType || inSize_ != *(int*)(pData)){
            ok_ = false;
            return;
        }
        offs_ = allType;
        (readField(fields), ...);
    } 
    bool ok()const{
        return ok_;
    }
private:
    bool checkFieldSize(int fieldSize){
        if (offs_ + fieldSize > inSize_){
            ok_ = false;
        }
        return ok_;        
    }
    void readField(std::string& s){ 
        if (ok_){
            if (!checkFieldSize(intSz_)) return;
            const char* pData = in_.data();
            int strSz = *((int*)(pData + offs_));  offs_ += intSz_;
            
            if (!checkFieldSize(strSz)) return;
            s = std::string(pData + offs_, strSz);  offs_ += strSz;
        }        
    }
    void readField(int& v){
        if (ok_){
            if (!checkFieldSize(intSz_)) return;
            const char* pData = in_.data();
            v = *((int*)(pData + offs_));  offs_ += intSz_;
        }
    }
    void readField(std::vector<int>& out){
        if (ok_){
            if (!checkFieldSize(intSz_)) return;
            const char* pData = in_.data();
            int vsz = *((int*)(pData + offs_));  offs_ += intSz_;
            if (!checkFieldSize(vsz * intSz_)) return;
            out.reserve(vsz);
            memcpy(out.data(), pData + offs_, vsz * intSz_);
            offs_ += vsz * intSz_;
        }
    }
    const int intSz_ = 4;
    int inSize_{};    
    int offs_ = 0;
    bool ok_ = true;
    const std::string& in_;
};

template<typename... Fields>
class SerialWriter{
public: 
    SerialWriter(const Fields&... fields){
        (fieldSize(fields), ...);

        outSize_ += intSz_;
        out_.resize(outSize_);
        writeField(outSize_);

        (writeField(fields), ...);
    } 
    std::string out(){
        return out_;
    }
private:
    void fieldSize(const std::string& s){
        outSize_ += intSz_ + s.size();
    }
    void fieldSize(int){
        outSize_ += intSz_;
    }
    void fieldSize(const std::vector<int>& v){
        outSize_ += intSz_ + int(v.size()) * intSz_;
    }
    void writeField(const std::string& s){ 
        char* pOut = out_.data();
        const auto ssz = s.size();
        *((int*)(pOut + offs_)) = ssz;        offs_ += intSz_;
        memcpy(pOut + offs_, s.data(), ssz);  offs_ += ssz;
    }
    void writeField(int v){
        char* pOut = out_.data();
        *((int*)(pOut + offs_)) = v; offs_ += intSz_;
    }
     void writeField(const std::vector<int>& arr){
        char* pOut = out_.data();
        const int asz = int(arr.size());
        *((int*)(pOut + offs_)) = asz; offs_ += intSz_;
        memcpy(pOut + offs_, arr.data(), asz * intSz_);
        offs_ += asz * intSz_;
    }
    const int intSz_ = 4;
    int outSize_{};    
    int offs_ = 0;
    std::string out_;        
};


mess::MessType getMessType(const std::string& m){
  int mtype{};
  SerialReader r(m, mtype);
  if (r.ok()){
    return (mess::MessType)mtype;
  }
  return mess::MessType::UNDEFINED;
}
std::string getConnectPnt(const std::string& m){
  int mtype{};
  std::string connectPnt;
  SerialReader(m, mtype, connectPnt);
  return connectPnt;
}

NewTask::NewTask(const std::string& connPnt):
  connectPnt(connPnt)
{
}

std::string NewTask::serialn(){
  const auto out = SerialWriter(int(MessType::NEW_TASK), connectPnt, 
                                params,
                                scriptPath,
                                resultPath,
                                taskId,
                                averDurationSec,
                                maxDurationSec).out();
  return out;
}

bool NewTask::deserialn(const std::string& m){
  int mtype{};
  const auto ok = SerialReader(m, mtype, connectPnt, 
                               params,
                               scriptPath,
                               resultPath,
                               taskId,
                               averDurationSec,
                               maxDurationSec).ok();
  return ok;
}        

TaskStatus::TaskStatus(mess::MessType _mtype, const std::string& connPnt):
  mtype(_mtype),
  connectPnt(connPnt)
{
}  

TaskStatus::TaskStatus(int tId, mess::MessType _mtype):
  taskId(tId),
  mtype(_mtype)
{
}

std::string TaskStatus::serialn(){
  const auto out = SerialWriter(int(mtype), connectPnt, 
                                taskId,
                                activeTaskCount,
                                loadCPU).out();
  return out;
}

bool TaskStatus::deserialn(const std::string& m){
  int _mtype{};
  const auto ok = SerialReader(m, _mtype, connectPnt, 
                               taskId,
                               activeTaskCount,
                               loadCPU).ok();
  mtype = MessType(_mtype);
  return ok;
}   

TaskProgress::TaskProgress()
{
}  

TaskProgress::TaskProgress(const std::vector<int>& _taskIds, const std::vector<int>& _taskProgress):
  taskIds(_taskIds),
  taskProgress(_taskProgress)
{
}

std::string TaskProgress::serialn(){
  const auto out = SerialWriter(int(mess::MessType::TASK_PROGRESS), connectPnt, 
                                taskIds,
                                taskProgress,
                                activeTaskCount,
                                loadCPU).out();
  return out;
}

bool TaskProgress::deserialn(const std::string& m){
  int mtype{};
  const auto ok = SerialReader(m, mtype, connectPnt, 
                               taskIds,
                               taskProgress,
                               activeTaskCount,
                               loadCPU).ok();
  return ok;
}   

InfoMess::InfoMess(mess::MessType _mtype, const std::string& connPnt):
  mtype(_mtype),
  connectPnt(connPnt)
{
} 

std::string InfoMess::serialn(){
  const auto out = SerialWriter(int(mtype), connectPnt).out();
  return out;
}

bool InfoMess::deserialn(const std::string& m){
  int _mtype{};
  const auto ok = SerialReader(m, _mtype, connectPnt).ok();
  mtype = MessType(_mtype);
  return ok;
}       

InternError::InternError(const std::string& connPnt, const std::string& mess):
  connectPnt(connPnt),
  message(mess)
{
}
  
std::string InternError::serialn(){
  const auto out = SerialWriter(int(MessType::INTERN_ERROR), connectPnt, 
                                message).out();
  return out;
}

bool InternError::deserialn(const std::string& m){
  int mtype{};
  const auto ok = SerialReader(m, mtype, connectPnt, 
                               message).ok();
  return ok;
}
}