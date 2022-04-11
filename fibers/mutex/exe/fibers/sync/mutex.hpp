#pragma once

#include <exe/fibers/sync/futex.hpp>

#include <twist/stdlike/atomic.hpp>

namespace exe::fibers {

class Mutex {
 public:
  void Lock() {
    count_contentions_.fetch_add(1);
    while (is_locked_.exchange(1) == 1) {
      futex_.ParkIfEqual(1);
    }
  }

  void Unlock() {
    is_locked_.store(0);
    if (count_contentions_.fetch_sub(1) > 1) {
      futex_.WakeOne();
    }
  }

  // BasicLockable

  void lock() {  // NOLINT
    Lock();
  }

  void unlock() {  // NOLINT
    Unlock();
  }

 private:
  twist::stdlike::atomic<uint32_t> is_locked_{0};
  twist::stdlike::atomic<uint32_t> count_contentions_{0};
  FutexLike<uint32_t> futex_{is_locked_};
};

}  // namespace exe::fibers
