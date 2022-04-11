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
    counter_.fetch_add(count);
  }

  void Done() {
    doing_.fetch_add(1);
    counter_.fetch_sub(1);
    futex_.WakeAll();
    doing_.fetch_sub(1);
  }

  void Wait() {
    uint32_t current;
    while ((current = counter_.load()) > 0) {
      futex_.ParkIfEqual(current);
    }
    while (doing_.load() != 0) {
    }
  }

 private:
  twist::stdlike::atomic<uint32_t> counter_;
  twist::stdlike::atomic<uint32_t> doing_{0};
  FutexLike<uint32_t> futex_{counter_};
};

}  // namespace exe::fibers
