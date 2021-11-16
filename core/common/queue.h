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

#include <vector>
#include <mutex>
#include <algorithm>
#include <utility>
#include <memory>

namespace Aux{

template<typename T>
class Queue{
  struct node{
    std::shared_ptr<T> data;
    std::unique_ptr<node> next;
  };
  std::mutex _headMtx, _tailMtx;
  std::unique_ptr<node> _head;
  node* _tail;  
  int _sz = 0;
      
  std::unique_ptr<node> tryPopHead(T& value){
    std::lock_guard<std::mutex> lock(_headMtx);
    if(_head.get() == getTailAndModifySize()){
      return std::unique_ptr<node>();
    }
    value = std::move(*_head->data);
    std::unique_ptr<node> oldHead = std::move(_head);
    _head = std::move(oldHead->next);   
    return oldHead;
  }      
  node* getTail(){
    std::lock_guard<std::mutex> lock(_tailMtx);
    return _tail;
  }
  node* getTailAndModifySize(){
    std::lock_guard<std::mutex> lock(_tailMtx);
    _sz = std::max(0, --_sz);
    return _tail;
  }
public:
  Queue() : _head(new node), _tail(_head.get()), _sz(0){};
  Queue(const Queue& other) = delete;
  Queue& operator=(const Queue& other) = delete;

  void push(T&& newValue){
    std::shared_ptr<T> newData(std::make_shared<T>(std::move(newValue)));
    std::unique_ptr<node> p(new node);
    {
      std::lock_guard<std::mutex> lock(_tailMtx);
      _tail->data = newData;
      node* const newTail = p.get();
      _tail->next = std::move(p);
      _tail = newTail;
      ++_sz;
    }
  }
  bool tryPop(T& value){
    std::unique_ptr<node> const oldHead = tryPopHead(value);    
    return oldHead.get() != nullptr;
  }
  bool front(T& value){
    std::lock_guard<std::mutex> lock(_headMtx);
    bool isExist = (_head.get() != getTail());
    if (isExist){
      value = *_head->data;
    }
    return isExist;
  }
  int size(){
    std::lock_guard<std::mutex> lock(_tailMtx);
    return _sz;
  }
  bool empty(){
    std::lock_guard<std::mutex> lock(_headMtx);
    return (_head.get() == getTail());
  }
};   
}