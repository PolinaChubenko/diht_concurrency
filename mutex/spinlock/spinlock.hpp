#pragma once

#include "atomic.hpp"

#include <wheels/support/cpu.hpp>

namespace solutions {

// Naive Test-and-Set (TAS) spinlock

class TASSpinLock {
 public:
  void Lock() {
    while (locked_.Exchange(1) == 1) {
      wheels::SpinLockPause();
    }
  }

  bool TryLock() {
    return locked_.Exchange(1) == 0;
  }

  void Unlock() {
    locked_.Store(0);
  }

 private:
  stdlike::Atomic locked_{0};
};

}  // namespace solutions
