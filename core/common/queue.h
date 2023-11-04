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

namespace misc{

template<typename T>
class Queue{
  struct Node{
    std::shared_ptr<T> data;
    std::unique_ptr<Node> next;
  };
  std::mutex headMtx_, tailMtx_;
  std::unique_ptr<Node> head_;
  Node* tail_{};  
  int sz_ = 0;
      
  std::unique_ptr<Node> tryPopHead(T& value){
    std::lock_guard<std::mutex> lock(headMtx_);
    if(head_.get() == getTailAndModifySize()){
      return std::unique_ptr<Node>();
    }
    value = std::move(*head_->data);
    std::unique_ptr<Node> oldHead = std::move(head_);
    head_ = std::move(oldHead->next);   
    return oldHead;
  }      
  Node* getTail(){
    std::lock_guard<std::mutex> lock(tailMtx_);
    return tail_;
  }
  Node* getTailAndModifySize(){
    std::lock_guard<std::mutex> lock(tailMtx_);
    sz_ = std::max(0, --sz_);
    return tail_;
  }
public:
  Queue() : head_(new Node), tail_(head_.get()), sz_(0){};
  Queue(const Queue& other) = delete;
  Queue& operator=(const Queue& other) = delete;

  void push(T&& newValue){
    std::shared_ptr<T> newData(std::make_shared<T>(std::move(newValue)));
    std::unique_ptr<Node> p(new Node);
    {
      std::lock_guard<std::mutex> lock(tailMtx_);
      tail_->data = newData;
      Node* const newTail = p.get();
      tail_->next = std::move(p);
      tail_ = newTail;
      ++sz_;
    }
  }
  bool tryPop(T& value){
    std::unique_ptr<Node> const oldHead = tryPopHead(value);    
    return oldHead.get() != nullptr;
  }
  bool front(T& value){
    std::lock_guard<std::mutex> lock(headMtx_);
    bool isExist = (head_.get() != getTail());
    if (isExist){
      value = *head_->data;
    }
    return isExist;
  }
  int size(){
    std::lock_guard<std::mutex> lock(tailMtx_);
    return sz_;
  }
  bool empty(){
    std::lock_guard<std::mutex> lock(headMtx_);
    return (head_.get() == getTail());
  }
};   
}