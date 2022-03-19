#include <exe/coroutine/standalone.hpp>

#include <twist/util/thread_local.hpp>

#include <wheels/support/assert.hpp>
#include <wheels/support/defer.hpp>

namespace exe::coroutine {

// static thread_local SimpleCoroutine* current = nullptr;
static twist::util::ThreadLocalPtr<Coroutine> current;

Coroutine::Coroutine(Routine routine)
    : stack_(AllocateStack()), impl_(std::move(routine), stack_.View()) {
}

void Coroutine::Resume() {
  Coroutine* prev = current.Exchange(this);

  wheels::Defer rollback([prev]() {
    current = prev;
  });

  impl_.Resume();
}

void Coroutine::Suspend() {
  WHEELS_VERIFY(current, "Not a coroutine");
  current->impl_.Suspend();
}

context::Stack Coroutine::AllocateStack() {
  static const size_t kStackPages = 16;  // 16 * 4KB = 64KB
  return context::Stack::AllocatePages(kStackPages);
}

}  // namespace exe::coroutine
