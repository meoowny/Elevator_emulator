#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#include <mutex>
#include <condition_variable>

class Semaphore
{
public:
  explicit Semaphore(int count = 0)
  : count_(count)
  { }

  void signal()
  {
    std::unique_lock<std::mutex> lock(mutex_);
    ++count_;
    cv_.notify_one();
  }

  void wait()
  {
    std::unique_lock<std::mutex> lock(mutex_);
    cv_.wait(lock, [=] { return count_ > 0; });
    --count_;
  }

private:
  int count_;
  std::mutex mutex_;
  std::condition_variable cv_;
};

#endif // SEMAPHORE_H
