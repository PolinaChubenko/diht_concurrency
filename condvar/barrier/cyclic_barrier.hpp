#pragma once

#include <twist/stdlike/mutex.hpp>
#include <twist/stdlike/condition_variable.hpp>

// std::lock_guard, std::unique_lock
#include <mutex>
#include <cstdint>
#include <iostream>

namespace solutions {

// CyclicBarrier allows a set of threads to all wait for each other
// to reach a common barrier point

// The barrier is called cyclic because
// it can be re-used after the waiting threads are released.

class CyclicBarrier {
 public:
  explicit CyclicBarrier(size_t participants) : participants_(participants) {
  }

  // Blocks until all participants have invoked Arrive()
  void Arrive() {
    std::unique_lock<twist::stdlike::mutex> unique_lock(mutex_);
    if (++waiters_ < participants_) {
      size_t current_opening = opened_cnt_;
      do {
        go_through_.wait(unique_lock);
      } while (current_opening == opened_cnt_);
    } else {
      ++opened_cnt_;
      waiters_ = 0;
      go_through_.notify_all();
    }
  }

 private:
  twist::stdlike::mutex mutex_;
  twist::stdlike::condition_variable go_through_;
  size_t participants_;
  size_t waiters_ = 0;
  size_t opened_cnt_ = 0;
};

}  // namespace solutions
