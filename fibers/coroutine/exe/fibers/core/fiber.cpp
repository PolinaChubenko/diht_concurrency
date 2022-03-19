#include <exe/fibers/core/fiber.hpp>
#include <exe/fibers/core/stacks.hpp>

#include <twist/util/thread_local.hpp>

namespace exe::fibers {

//////////////////////////////////////////////////////////////////////

void Fiber::Schedule() {
  // No impl
}

void Fiber::Yield() {
  // No impl
}

void Fiber::Step() {
  // No impl
}

Fiber& Fiber::Self() {
  std::abort();  // No impl
}

//////////////////////////////////////////////////////////////////////

// API Implementation

void Go(Scheduler& /*scheduler*/, Routine /*routine*/) {
  // No impl
}

void Go(Routine /*routine*/) {
  // No impl
}

namespace self {

void Yield() {
  // No impl
}

}  // namespace self

}  // namespace exe::fibers
