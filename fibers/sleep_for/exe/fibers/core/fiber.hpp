#pragma once

#include <exe/fibers/core/api.hpp>
#include <exe/coroutine/impl.hpp>

#include <context/stack.hpp>

#include <twist/stdlike/atomic.hpp>

namespace exe::fibers {

// Fiber = Stackful coroutine + Scheduler

class Fiber {
  enum class FiberState {
    Starting,
    Running,
    Runnable,
    Suspendable,
    Terminated
  };

 public:
  Fiber(Scheduler& scheduler, Routine routine);

  // ~ System calls
  void Schedule();
  void Destroy();

  void Yield();
  void SleepFor(Millis delay);

  void Suspend();
  void Resume();

  static Fiber& Self();
  Scheduler* GetCurrentSchedule();

 private:
  FiberState State() const;
  void SetState(FiberState target);

  // Task
  void Step();
  void Dispatch();

 private:
  Scheduler* scheduler_;
  context::Stack stack_;
  coroutine::CoroutineImpl coroutine_;
  FiberState state_;
  twist::stdlike::atomic<bool> is_suspended_{false};
};

}  // namespace exe::fibers
