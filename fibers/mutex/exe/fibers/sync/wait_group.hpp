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
    if (counter_.fetch_sub(1) == 1) {
      counter_.FutexWakeAll();
    }
  }

  void Wait() {
    uint32_t count = counter_.load();
    while (counter_.load() > 0) {
      counter_.FutexWait(count);
    }
  }

 private:
  twist::stdlike::atomic<uint32_t> counter_;
  FutexLike<uint32_t> futex_{counter_};
};

}  // namespace exe::fibers
