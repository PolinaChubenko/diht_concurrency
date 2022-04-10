#pragma once

#include <exe/fibers/core/api.hpp>
#include <exe/support/spinlock.hpp>

#include <twist/stdlike/atomic.hpp>

#include <wheels/intrusive/list.hpp>
#include <wheels/support/assert.hpp>

namespace exe::fibers {

template <typename T>
class FutexLike {
 public:
  explicit FutexLike(twist::stdlike::atomic<T>& cell) : cell_(cell) {
  }

  ~FutexLike() {
    assert(parking_.empty());
  }

  // Park current fiber if cell.load() == `old`
  void ParkIfEqual(T old) {
    std::unique_lock lock(spinlock_);
    if (cell_.load() == old) {
      parking_.PushBack(cell_);
      self::Suspend();
    }
  }

  void WakeOne() {
    std::lock_guard guard_lock(spinlock_);
    if (!parking_.IsEmpty()) {
      parking_.PopBack();
    }
  }

  void WakeAll() {
    std::lock_guard guard_lock(spinlock_);
    parking_.UnlinkAll();
  }

 private:
  twist::stdlike::atomic<T>& cell_;
  exe::support::SpinLock spinlock_;
  wheels::IntrusiveList<twist::stdlike::atomic<T>> parking_;
};

}  // namespace exe::fibers
