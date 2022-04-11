#include <exe/fibers/core/fiber.hpp>
#include <exe/fibers/core/stacks.hpp>

#include <twist/util/thread_local.hpp>

namespace exe::fibers {

static twist::util::ThreadLocalPtr<Fiber> local_fiber;
static twist::util::ThreadLocalPtr<IAwaiter> local_awaiter;

//////////////////////////////////////////////////////////////////////

Fiber::Fiber(Scheduler& scheduler, Routine routine)
    : scheduler_(&scheduler),
      stack_(AllocateStack()),
      coroutine_(std::move(routine), stack_.View()) {
}

Fiber::~Fiber() {
  ReleaseStack(std::move(stack_));
}

void Fiber::Schedule() {
  scheduler_->Submit([this]() {
    fibers::local_fiber = this;
    Step();
  });
}

void Fiber::Yield() {
  auto handle = GetHandle();
  YieldScheduler awaiter(handle);
  Suspend(&awaiter);
}

void Fiber::Suspend(IAwaiter* awaiter) {
  local_awaiter = awaiter;
  coroutine_.Suspend();
}

void Fiber::Resume() {
  coroutine_.Resume();
}

void Fiber::Step() {
  try {
    Resume();
  } catch (...) {
    std::cout << "Caught exception from coroutine\n";
    return;
  }
  if (coroutine_.IsCompleted()) {
    delete this;
  } else {
    local_awaiter->CallStrategy();
  }
}

FiberHandle Fiber::GetHandle() {
  return FiberHandle(this);
}

Fiber& Fiber::Self() {
  return *local_fiber;
}

//////////////////////////////////////////////////////////////////////

// API Implementation

void Go(Scheduler& scheduler, Routine routine) {
  auto fiber = new Fiber(scheduler, std::move(routine));
  fiber->Schedule();
}

void Go(Routine routine) {
  Go(*Scheduler::Current(), std::move(routine));
}

namespace self {

void Yield() {
  Fiber::Self().Yield();
}

FiberHandle GetHandle() {
  return Fiber::Self().GetHandle();
}

void Suspend(IAwaiter* awaiter) {
  Fiber::Self().Suspend(awaiter);
}

}  // namespace self

}  // namespace exe::fibers
