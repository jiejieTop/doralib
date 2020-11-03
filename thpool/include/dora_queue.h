/*
 * @Author: jiejie
 * @GitHub: https://github.com/jiejieTop
 * @Date: 2020-10-26 11:13:53
 * @LastEditors: jiejie
 * @LastEditTime: 2020-10-27 13:36:09
 * @Description: the code belongs to jiejie, please keep the author information and source code according to the license.
 */
#ifndef _DORA_QUEUE_H_
#define _DORA_QUEUE_H_

#include <mutex>
#include <queue>

namespace doralib {
    
template <typename T>
class queue {
private:
  std::queue<T> _queue;
  std::mutex _mutex;
public:
  queue() {}

  queue(queue & other) {}

  ~queue() {}


  bool empty() {
    std::unique_lock<std::mutex> lock(_mutex);
    return _queue.empty();
  }
  
  int size() {
    std::unique_lock<std::mutex> lock(_mutex);
    return _queue.size();
  }

  void enqueue(T& t) {
    std::unique_lock<std::mutex> lock(_mutex);
    _queue.push(t);
  }
  
  bool dequeue(T& t) {
    std::unique_lock<std::mutex> lock(_mutex);

    if (_queue.empty()) {
      return false;
    }
    t = std::move(_queue.front());
    
    _queue.pop();
    return true;
  }
};

}

#endif
