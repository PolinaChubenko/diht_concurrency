#pragma once

#include <exe/fibers/core/api.hpp>
#include <exe/coroutine/impl.hpp>
#include <context/stack.hpp>
#include <exe/fibers/core/awaiter.hpp>

namespace exe::fibers {

// Fiber = Stackful coroutine + Scheduler (Thread pool)

class Fiber {
 public:
  Fiber(Scheduler& scheduler, Routine routine);
  ~Fiber();
  // ~ System calls

  void Schedule();

  void Yield();

  void Suspend(IAwaiter*);
  void Resume();

  FiberHandle GetHandle();
  static Fiber& Self();

 private:
  // Task
  void Step();

 private:
  Scheduler* scheduler_;
  context::Stack stack_;
  coroutine::CoroutineImpl coroutine_;
};

}  // namespace exe::fibers
