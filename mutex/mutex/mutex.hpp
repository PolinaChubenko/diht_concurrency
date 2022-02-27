#pragma once

#include <twist/stdlike/atomic.hpp>

#include <cstdlib>

namespace stdlike {

class Mutex {
 public:
  void Lock() {
    uint32_t expected = 0;
    if (!locked_.compare_exchange_strong(expected, 1)) {
      // mutex was locked, we need to wait
      // expected = 1 or 2
      do {
        if (expected != 2) {
          locked_.compare_exchange_strong(expected, 2);
        }
        locked_.wait(2);
        // try to lock again
        // (we make locked_ = 2 as there may be another thread here)
        expected = 0;
      } while (!locked_.compare_exchange_strong(expected, 2));
    }
  }

  void Unlock() {
    if (locked_.fetch_sub(1) == 2) {
      locked_.store(0);
      locked_.notify_one();
    }
  }

 private:
  // 0 = is unlocked; 1 = is locked; 2 = is locked and has waiters
  twist::stdlike::atomic<uint32_t> locked_{0};
};

}  // namespace stdlike
