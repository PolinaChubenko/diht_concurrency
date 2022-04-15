#pragma once

#include <twist/util/spin_wait.hpp>
#include <twist/stdlike/atomic.hpp>

namespace exe::support {

// Test-and-TAS spinlock

class SpinLock {
 public:
  void Lock() {
    while (locked_.exchange(true, std::memory_order_acquire)) {
      while (locked_.load(std::memory_order_relaxed)) {
        spin_wait_();
      }
    }
  }

  void Unlock() {
    locked_.store(false, std::memory_order_release);
  }

  // BasicLockable

  void lock() {  // NOLINT
    Lock();
  }

  void unlock() {  // NOLINT
    Unlock();
  }

 private:
  twist::util::SpinWait spin_wait_;
  twist::stdlike::atomic<bool> locked_{false};
};

}  // namespace exe::support
