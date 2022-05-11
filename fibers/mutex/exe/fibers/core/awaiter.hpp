#pragma once

#include <exe/fibers/core/handle.hpp>
#include <wheels/intrusive/list.hpp>
#include <twist/stdlike/atomic.hpp>

namespace exe::fibers {

struct IAwaiter {
  explicit IAwaiter(FiberHandle handle) : handle_(handle) {
  }

  virtual void AwaitSuspend() = 0;

 protected:
  FiberHandle handle_;
};

struct YieldScheduler : IAwaiter {
  explicit YieldScheduler(FiberHandle handle) : IAwaiter(handle) {
  }

  void AwaitSuspend() override {
    handle_.Schedule();
  }
};

struct FutexScheduler : IAwaiter, wheels::IntrusiveListNode<FutexScheduler> {
  explicit FutexScheduler(FiberHandle handle) : IAwaiter(handle) {
  }

  void AwaitSuspend() override {
    if (count_awaits_.fetch_sub(1) == 1) {
      handle_.Schedule();
    }
  }

  twist::stdlike::atomic<uint32_t> count_awaits_{2};
};

}  // namespace exe::fibers
