#pragma once

#include <twist/stdlike/mutex.hpp>
#include <twist/stdlike/condition_variable.hpp>

namespace exe::tp {

class Countdown {
 public:
  void Increment() {
    std::lock_guard guard_lock(mutex_);
    ++counter_;
  }

  void Decrement() {
    std::lock_guard guard_lock(mutex_);
    if (--counter_ == 0) {
      is_zero_.notify_all();
    }
  }

  void WaitZero() {
    std::unique_lock unique_lock(mutex_);
    is_zero_.wait(unique_lock, [&]() {
      return counter_ == 0;
    });
  }

 private:
  twist::stdlike::condition_variable is_zero_;
  twist::stdlike::mutex mutex_;
  size_t counter_ = 0;
};

}  // namespace exe::tp