#include <exe/fibers/core/fiber.hpp>
#include <exe/fibers/core/stacks.hpp>

#include <twist/util/thread_local.hpp>
#include <twist/util/spin_wait.hpp>

#include <asio/steady_timer.hpp>
#include <asio/post.hpp>

namespace exe::fibers {

static twist::util::ThreadLocalPtr<Fiber> local_fiber;

//////////////////////////////////////////////////////////////////////

Fiber::Fiber(Scheduler& scheduler, Routine routine)
    : scheduler_(&scheduler),
      stack_(AllocateStack()),
      coroutine_(std::move(routine), stack_.View()),
      state_(FiberState::Starting) {
}

void Fiber::Destroy() {
  ReleaseStack(std::move(stack_));
  delete this;
}

void Fiber::Schedule() {
  asio::post(*scheduler_, [this]() {
    fibers::local_fiber = this;
    Step();
  });
}

void Fiber::Yield() {
  SetState(FiberState::Runnable);
  coroutine_.Suspend();
}

void Fiber::SleepFor(Millis delay) {
  auto timer = new asio::steady_timer(*scheduler_);
  timer->expires_after(delay);
  timer->async_wait([this, timer](std::error_code /*ec*/) {
    Resume();
    delete timer;
  });
  Suspend();
}

void Fiber::Suspend() {
  is_suspended_.store(false);
  SetState(FiberState::Suspended);
  coroutine_.Suspend();
}

void Fiber::Resume() {
  twist::util::SpinWait spin_wait;
  while (!is_suspended_.load()) {
    spin_wait();
  }
  is_suspended_.store(false);
  SetState(FiberState::Runnable);
  Schedule();
}

void Fiber::Step() {
  SetState(FiberState::Running);
  try {
    coroutine_.Resume();
  } catch (...) {
    std::cout << "Caught exception from coroutine\n";
    return;
  }
  if (coroutine_.IsCompleted()) {
    SetState(FiberState::Terminated);
  }
  Dispatch();
}

void Fiber::Dispatch() {
  switch (State()) {
    case FiberState::Runnable:
      Schedule();
      break;
    case FiberState::Suspended:
      is_suspended_.store(true);
      //      SetState(FiberState::Suspended);
      break;
    case FiberState::Terminated:
      Destroy();
      break;
    default:
      std::cout << "Unexpected fiber state\n";
      break;
  }
}

Fiber::FiberState Fiber::State() const {
  return state_;
}

void Fiber::SetState(Fiber::FiberState target) {
  state_ = target;
}

Fiber& Fiber::Self() {
  return *local_fiber;
}

Scheduler* Fiber::GetCurrentSchedule() {
  return scheduler_;
}

//////////////////////////////////////////////////////////////////////

// API Implementation

void Go(Scheduler& scheduler, Routine routine) {
  auto fiber = new Fiber(scheduler, std::move(routine));
  fiber->Schedule();
}

void Go(Routine routine) {
  Go(*Fiber::Self().GetCurrentSchedule(), std::move(routine));
}

namespace self {

void Yield() {
  Fiber::Self().Yield();
}

void SleepFor(Millis delay) {
  Fiber::Self().SleepFor(delay);
}

}  // namespace self

}  // namespace exe::fibers
