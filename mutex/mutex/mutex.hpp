#pragma once

#include <twist/stdlike/atomic.hpp>

#include <cstdlib>

namespace stdlike {

class Mutex {
 public:
  void Lock() {
    count_contentions_.fetch_add(1);
    while (is_locked_.exchange(1) == 1) {
      is_locked_.FutexWait(1);
    }
  }

  void Unlock() {
    is_locked_.store(0);
    if (count_contentions_.fetch_sub(1) > 1) {
      is_locked_.FutexWakeOne();
    }
  }

 private:
  twist::stdlike::atomic<uint32_t> is_locked_{0};
  twist::stdlike::atomic<uint32_t> count_contentions_{0};
};

}  // namespace stdlike
