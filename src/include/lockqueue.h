#pragma once

#include <condition_variable>
#include <mutex>
#include <queue>
#include <thread>

// 异步日志队列
template <typename T>
class LockQueue
{
public:
  void Push(const T &data)
  {
    std::lock_guard<std::mutex> lock(mutex_);
    queue_.push(data);
    condVariable_.notify_one();
  }

  T Pop()
  {
    std::unique_lock<std::mutex> lock(mutex_);
    while (queue_.empty()) {
      // 线程进入wait状态
      condVariable_.wait(lock);
    }
    T data = queue_.front();
    queue_.pop();
    return data;
  }

private:
  std::queue<T> queue_;
  std::mutex mutex_;
  std::condition_variable condVariable_;
};