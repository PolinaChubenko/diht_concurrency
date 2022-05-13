#pragma once

#include <exe/fibers/core/handle.hpp>
#include <wheels/intrusive/list.hpp>
#include <twist/stdlike/atomic.hpp>

namespace exe::fibers {

struct IAwaiter {
  virtual void AwaitSuspend(FiberHandle handle) = 0;
};

struct YieldAwaiter : IAwaiter {
  void AwaitSuspend(FiberHandle handle) override {
    handle.Schedule();
  }
};

}  // namespace exe::fibers
