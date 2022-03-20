#include <exe/fibers/core/fiber.hpp>
#include <exe/fibers/core/stacks.hpp>

#include <twist/util/thread_local.hpp>
#include <exe/tp/submit.hpp>

namespace exe::fibers {

twist::util::ThreadLocalPtr<Fiber> local_fiber;

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
  tp::Submit(*scheduler_, [this]() {
    fibers::local_fiber = this;
    Step();
  });
}

void Fiber::Yield() {
  coroutine_.Suspend();
}

void Fiber::Step() {
  try {
    this->coroutine_.Resume();
  } catch (...) {
    std::cout << "Caught exception from coroutine\n";
    return;
  }
  if (this->coroutine_.IsCompleted()) {
    delete this;
  } else {
    Schedule();
  }
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

}  // namespace self

}  // namespace exe::fibers
