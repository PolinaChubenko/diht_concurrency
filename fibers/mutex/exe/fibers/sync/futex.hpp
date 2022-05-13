#pragma once

#include <exe/fibers/core/api.hpp>
#include <exe/support/spinlock.hpp>

#include <twist/stdlike/atomic.hpp>

#include <wheels/intrusive/list.hpp>
#include <wheels/support/assert.hpp>

namespace exe::fibers {

template <typename T>
class FutexLike {
 private:
  struct FutexNode : IAwaiter, wheels::IntrusiveListNode<FutexNode> {
    using Lock = exe::support::SpinLock;

    explicit FutexNode(Lock* locker) : locker_(locker) {
    }

    void Schedule() {
      handle_.Schedule();
    }

    void AwaitSuspend(FiberHandle handle) override {
      handle_ = handle;
      locker_->unlock();
    }

    Lock* locker_;
    FiberHandle handle_{self::GetHandle()};
  };

 public:
  explicit FutexLike(twist::stdlike::atomic<T>& cell) : cell_(cell) {
  }

  ~FutexLike() {
    assert(parking_.IsEmpty());
  }

  // Park current fiber if cell.load() == `old`
  void ParkIfEqual(T old) {
    spinlock_.lock();
    if (cell_.load() == old) {
      FutexNode awaiter(&spinlock_);
      parking_.PushBack(&awaiter);
      self::Suspend(&awaiter);
    } else {
      spinlock_.unlock();
    }
  }

  void WakeOne() {
    std::unique_lock unique_lock(spinlock_);
    if (auto awaiter = parking_.PopFront()) {
      awaiter->Schedule();
    }
  }

  void WakeAll() {
    std::unique_lock unique_lock(spinlock_);
    while (auto awaiter = parking_.PopFront()) {
      awaiter->Schedule();
    }
  }

 private:
  twist::stdlike::atomic<T>& cell_;
  exe::support::SpinLock spinlock_;
  wheels::IntrusiveList<FutexNode> parking_;
};

}  // namespace exe::fibers
