#include <exe/coroutine/impl.hpp>

#include <wheels/support/assert.hpp>
#include <wheels/support/compiler.hpp>

namespace exe::coroutine {

CoroutineImpl::CoroutineImpl(Routine routine, wheels::MutableMemView stack)
    : routine_(std::move(routine)) {
  context_routine_.Setup(stack, this);
}

void CoroutineImpl::Run() {
  try {
    routine_();
  } catch (...) {
    error_ptr_ = std::current_exception();
  }
  is_routine_complete_ = true;
  context_routine_.SwitchTo(context_main_);
  std::abort();
}

void CoroutineImpl::Resume() {
  context_main_.SwitchTo(context_routine_);
  if (error_ptr_) {
    std::rethrow_exception(error_ptr_);
  }
}

void CoroutineImpl::Suspend() {
  context_routine_.SwitchTo(context_main_);
}

bool CoroutineImpl::IsCompleted() const {
  return is_routine_complete_;
}

}  // namespace exe::coroutine
