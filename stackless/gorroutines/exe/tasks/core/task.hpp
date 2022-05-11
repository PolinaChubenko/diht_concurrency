#pragma once

#include <cstdlib>
#include <optional>

#include <wheels/support/unit.hpp>
#include <wheels/support/result.hpp>

#include <experimental/coroutine>

namespace exe::tasks {

//////////////////////////////////////////////////////////////////////

template <typename T = wheels::Unit>
struct Task {
  struct Promise {
    // NOLINTNEXTLINE
    auto get_return_object() {
      return Task{
          std::experimental::coroutine_handle<Promise>::from_promise(*this)};
    }

    // NOLINTNEXTLINE
    auto initial_suspend() noexcept {
      return std::experimental::suspend_always{};
    }

    // NOLINTNEXTLINE
    auto final_suspend() noexcept {
      return std::experimental::suspend_never{};
    }

    // NOLINTNEXTLINE
    void set_exception(std::exception_ptr) {
      std::terminate();  // Not implemented
    }

    // NOLINTNEXTLINE
    void unhandled_exception() {
      std::terminate();  // Not implemented
    }

    // NOLINTNEXTLINE
    void return_void() {
      // Not implemented
    }

    std::optional<wheels::Result<T>> result;
  };

  using CoroutineHandle = std::experimental::coroutine_handle<Promise>;

  explicit Task(CoroutineHandle callee) : callee_(callee) {
  }

  Task(Task&&) = default;

  // Non-copyable
  Task(const Task&) = delete;
  Task& operator=(const Task&) = delete;

  ~Task() {
    if (callee_ && !callee_.done()) {
      std::terminate();
    }
  }

  CoroutineHandle ReleaseCoroutine() {
    return std::exchange(callee_, CoroutineHandle());
  }

 private:
  CoroutineHandle callee_;
};

}  // namespace exe::tasks

//////////////////////////////////////////////////////////////////////

template <typename T, typename... Args>
struct std::experimental::coroutine_traits<exe::tasks::Task<T>, Args...> {
  using promise_type = typename exe::tasks::Task<T>::Promise;  // NOLINT
};
