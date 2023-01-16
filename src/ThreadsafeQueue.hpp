#pragma once

#include <mutex>
#include <queue>

template <typename T>
class ThreadsafeQueue {
  std::queue<T> _queue;
  mutable std::mutex _mutex;

public:
  virtual ~ThreadsafeQueue() {
  }

  unsigned long size() const {
    std::lock_guard<std::mutex> lock(_mutex);

    return _queue.size();
  }

  bool pop(T& result) {
    std::lock_guard<std::mutex> lock(_mutex);

    if (_queue.empty()) {
      return false;
    }

    result = std::move(_queue.front());
    _queue.pop();

    return true;
  }

  void push(const T& item) {
    std::lock_guard<std::mutex> lock(_mutex);

    _queue.push(item);
  }
};
