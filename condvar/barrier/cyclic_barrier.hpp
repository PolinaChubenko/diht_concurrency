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
    waiters_ = 0;
    is_new_portion_ = false;
  }

  // Blocks until all participants have invoked Arrive()
  void Arrive() {
    std::unique_lock<twist::stdlike::mutex> unique_lock(mutex_);
    // new threads need to wait for the barrier to process the previous batch
    while (is_new_portion_) {
      go_through_.wait(unique_lock);
    }
    // blocking as not all participants have invoked Arrive()
    if (++waiters_ < participants_) {
      while (!is_new_portion_) {
        go_through_.wait(unique_lock);
      }
    }
    // the last thread from this batch has arrived,
    // we separate the portions of threads and wake up neighbors-participants
    if (waiters_ == participants_) {
      is_new_portion_ = true;
      go_through_.notify_all();
    }
    // we count the number of released threads,
    // when the last one leaves, we will allow a new batch of threads
    // to move to the next stage in the barrier
    if (--waiters_ == 0) {
      is_new_portion_ = false;
      go_through_.notify_all();
    }
  }

 private:
  twist::stdlike::condition_variable go_through_;
  size_t participants_;
  size_t waiters_;
  bool is_new_portion_;
  twist::stdlike::mutex mutex_;
};

}  // namespace solutions
