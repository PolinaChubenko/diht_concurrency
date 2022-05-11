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
    assert(parking_.IsEmpty());
  }

  // Park current fiber if cell.load() == `old`
  void ParkIfEqual(T old) {
    std::unique_lock unique_lock(spinlock_);
    if (cell_.load() == old) {
      FutexScheduler awaiter(self::GetHandle());
      parking_.PushBack(&awaiter);
      unique_lock.unlock();
      self::Suspend(&awaiter);
      unique_lock.lock();
    }
  }

  void WakeOne() {
    std::unique_lock unique_lock(spinlock_);
    if (!parking_.IsEmpty()) {
      auto awaiter = parking_.PopFront();
      unique_lock.unlock();
      awaiter->AwaitSuspend();
      unique_lock.lock();
    }
  }

  void WakeAll() {
    std::unique_lock unique_lock(spinlock_);
    size_t sz = parking_.Size();
    while (!parking_.IsEmpty() && sz != 0) {
      auto awaiter = parking_.PopFront();
      unique_lock.unlock();
      awaiter->AwaitSuspend();
      unique_lock.lock();
      --sz;
    }
  }

 private:
  twist::stdlike::atomic<T>& cell_;
  exe::support::SpinLock spinlock_;
  wheels::IntrusiveList<FutexScheduler> parking_;
};

}  // namespace exe::fibers
