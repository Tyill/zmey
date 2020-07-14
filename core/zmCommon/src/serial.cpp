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
        
#include <map>        
#include <vector>
#include <numeric>
#include <cstring>

using namespace std;

namespace ZM_Aux {

  std::string serialn(const std::map<std::string, std::string>& data){

    int vcnt = data.size() * 2;    
    vector<int> dataSz;
    dataSz.reserve(vcnt); 
    for (auto& v : data){
      dataSz.push_back((int)v.first.size());
      dataSz.push_back((int)v.second.size());
    }
    int vlsSz = std::accumulate(dataSz.begin(), dataSz.end(), 0),        
        intSz = 4,
        offs = 0,
        inx = 0;
    string out;
    out.resize(intSz + vlsSz + vcnt * intSz);
    char* pOut = (char*)out.data();

    *((int*)pOut) = vcnt / 2;                    
    offs += intSz;   
   
    for (auto& v : data){
      *((int*)(pOut + offs)) = dataSz[inx];              offs += intSz;
      memcpy(pOut + offs, v.first.data(), dataSz[inx]);  offs += dataSz[inx]; ++inx;

      *((int*)(pOut + offs)) = dataSz[inx];              offs += intSz;
      memcpy(pOut + offs, v.second.data(), dataSz[inx]); offs += dataSz[inx]; ++inx;
    }

    return out;
  }

  std::map<std::string, std::string> deserialn(const std::string& data){
        
    char* pData = (char*)data.data();    
    int offs = 0,
        intSz = 4,
        allSz = (int)data.size();
    
    if (allSz <= intSz * 3){ // vcnt, key, vsz
      return map<string, string>();
    }
    int vcnt = *((int*)pData);
    offs += intSz;

    map<string, string> out;    
    while(offs < allSz){
      if (offs + intSz >= allSz) break;
      int ksz = *((int*)(pData + offs)); offs += intSz;

      if (offs + ksz >= allSz) break;
      string key(pData + offs, ksz);     offs += ksz;   

      if (offs + intSz > allSz) break;
      int vsz = *((int*)(pData + offs)); offs += intSz; 
      
      if (offs + vsz > allSz) break;
      string val(pData + offs, vsz);     offs += vsz;   

      out[key] = val;
    }
    return vcnt == out.size() ? out : map<string, string>();
  }          
}