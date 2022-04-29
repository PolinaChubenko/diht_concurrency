#include <exe/fibers/core/fiber.hpp>
#include <exe/fibers/core/stacks.hpp>

#include <twist/util/thread_local.hpp>

#include <asio/post.hpp>

namespace exe::fibers {

static twist::util::ThreadLocalPtr<Fiber> local_fiber;

//////////////////////////////////////////////////////////////////////

Fiber::TimerSuspender::TimerSuspender(Fiber* fiber)
    : owner_(fiber), timer_(nullptr) {
}

void Fiber::TimerSuspender::SetTimer(asio::steady_timer* timer) {
  timer_ = timer;
}

void Fiber::TimerSuspender::CallBack() {
  timer_->async_wait([this](std::error_code) {
    timer_ = nullptr;
    owner_->Resume();
  });
}

Fiber::Fiber(Scheduler& scheduler, Routine routine)
    : scheduler_(&scheduler),
      stack_(AllocateStack()),
      coroutine_(std::move(routine), stack_.View()),
      state_(FiberState::Starting),
      timer_suspender_(this) {
}

void Fiber::Destroy() {
  ReleaseStack(std::move(stack_));
  delete this;
}

void Fiber::Schedule() {
  asio::post(*scheduler_, [this]() {
    Step();
  });
}

void Fiber::Yield() {
  SetState(FiberState::Runnable);
  coroutine_.Suspend();
}

void Fiber::SleepFor(Millis delay) {
  asio::steady_timer timer(*scheduler_);
  timer.expires_after(delay);
  timer_suspender_.SetTimer(&timer);
  Suspend();
}

void Fiber::Suspend() {
  SetState(FiberState::Suspendable);
  coroutine_.Suspend();
}

void Fiber::Resume() {
  SetState(FiberState::Runnable);
  Schedule();
}

void Fiber::Step() {
  fibers::local_fiber = this;
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
      // from Yield
      Schedule();
      break;
    case FiberState::Suspendable:
      // from SleepFor
      timer_suspender_.CallBack();
      break;
    case FiberState::Terminated:
      // task completed
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
