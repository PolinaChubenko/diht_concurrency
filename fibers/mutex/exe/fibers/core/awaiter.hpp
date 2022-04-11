#pragma once

#include <exe/fibers/core/handle.hpp>
#include <wheels/intrusive/list.hpp>
#include <twist/stdlike/atomic.hpp>

namespace exe::fibers {

struct IAwaiter {
  explicit IAwaiter(FiberHandle handle) : handle_(handle) {
  }

  virtual void CallStrategy() = 0;

 protected:
  FiberHandle handle_;
};

struct YieldScheduler : IAwaiter {
  explicit YieldScheduler(FiberHandle handle) : IAwaiter(handle) {
  }

  void CallStrategy() override {
    handle_.Schedule();
  }
};

struct FutexScheduler : IAwaiter, wheels::IntrusiveListNode<FutexScheduler> {
  explicit FutexScheduler(FiberHandle handle) : IAwaiter(handle) {
  }
  void CallStrategy() override {
    if (count_.fetch_sub(1) == 1) {
      handle_.Schedule();
    }
  }

  twist::stdlike::atomic<uint32_t> count_{2};
};

}  // namespace exe::fibers
