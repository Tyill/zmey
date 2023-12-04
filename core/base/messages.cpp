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
    SerialReader(const std::string& m, int mtype, Fields&... fields):
      in_(m){        
        const int allSz = intSz_,
                  typeSz = intSz_;
        inSize_ = int(m.size());         
        char* pData = (char*)m.data();
        if (inSize_ < (allSz + typeSz) || 
            inSize_ != *(int*)(pData) || 
            (mtype > 0 && *(int*)(pData + allSz) != mtype)){
            ok_ = false;
            return;
        }
        mtype_ = *(int*)(pData + allSz);
        offs_ = allSz + typeSz;
        (readField(fields), ...);
    } 
    int mtype()const{
        return mtype_;
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
    const int intSz_ = 4;
    int inSize_{};    
    int offs_ = 0;
    int mtype_ = 0;
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
    const int intSz_ = 4;
    int outSize_{};    
    int offs_ = 0;
    std::string out_;        
};


mess::MessType getMessType(const std::string& m){
  int mtype;
  const auto r = SerialReader(m, 0, mtype);
  if (r.ok()){
    return mess::MessType(mtype);
  }
  return mess::MessType::UNDEFINED;
}
std::string getConnectPnt(const std::string& m){
  std::string connectPnt;
  SerialReader(m, 0, connectPnt);
  return connectPnt;
}

NewTask::NewTask(const std::string& connPnt):
  connectPnt(connPnt)
{
}

std::string NewTask::serialn(){
  const auto out = SerialWriter(int(mtype), connectPnt, 
                                params,
                                scriptPath,
                                resultPath,
                                taskId).out();
  return out;
}

bool NewTask::deserialn(const std::string& m){
  const auto ok = SerialReader(m, int(mtype), connectPnt, 
                               params,
                               scriptPath,
                               resultPath,
                               taskId).ok();
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

  return out;
}

bool TaskStatus::deserialn(const std::string& m){
  const auto ok = SerialReader(m, int(mtype), connectPnt, 
                               taskId,
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
  const auto ok = SerialReader(m, int(mtype), connectPnt).ok();
  return ok;
}       

InternError::InternError(const std::string& connPnt, const std::string& mess):
  connectPnt(connPnt),
  message(mess)
{
}
  
std::string InternError::serialn(){
  const auto out = SerialWriter(int(mtype), connectPnt, 
                                message).out();
  return out;
}

bool InternError::deserialn(const std::string& m){
  const auto ok = SerialReader(m, int(mtype), connectPnt, 
                               message).ok();
  return ok;
}
}