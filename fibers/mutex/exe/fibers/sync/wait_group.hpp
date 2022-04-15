#pragma once

#include <exe/fibers/sync/futex.hpp>
#include <exe/fibers/sync/condvar.hpp>
#include <exe/fibers/sync/mutex.hpp>

#include <twist/stdlike/atomic.hpp>

namespace exe::fibers {

// https://gobyexample.com/waitgroups

class WaitGroup {
 public:
  void Add(size_t count) {
    wake_up_.store(0);
    is_all_done_.store(0);
    counter_.fetch_add(count);
  }

  void Done() {
    if (counter_.fetch_sub(1) == 1) {
      wake_up_.store(1);
      futex_.WakeAll();
      is_all_done_.store(1);
    }
  }

  void Wait() {
    while (is_all_done_.load() == 0) {
      futex_.ParkIfEqual(0);
    }
  }

 private:
  twist::stdlike::atomic<uint32_t> counter_{0};
  twist::stdlike::atomic<uint32_t> is_all_done_{0};
  twist::stdlike::atomic<uint32_t> wake_up_{0};
  FutexLike<uint32_t> futex_{wake_up_};
};

}  // namespace exe::fibers
