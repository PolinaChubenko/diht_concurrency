#pragma once

#include <exe/futures/core/future.hpp>

namespace exe::futures {

namespace detail {

template <typename T>
struct ResultWaiter {
 public:
  void Receive(wheels::Result<T>&& received) {
    result_ = std::move(received);
    has_result_.store(1);
    has_result_.notify_one();
  }

  void Wait() {
    while (has_result_.load() == 0) {
      has_result_.wait(0);
    }
  }

  wheels::Result<T> GetResult() {
    return std::move(*result_);
  }

 private:
  twist::stdlike::atomic<uint32_t> has_result_{0};
  std::optional<wheels::Result<T>> result_;
};

}  // namespace detail

// ~ std::future::get
// Blocking
template <typename T>
wheels::Result<T> GetResult(Future<T> future) {
  detail::ResultWaiter<T> result_waiter;

  std::move(future).Subscribe([&result_waiter](wheels::Result<T> input) {
    result_waiter.Receive(std::move(input));
  });

  result_waiter.Wait();
  return result_waiter.GetResult();
}

// Blocking
template <typename T>
T GetValue(Future<T> future) {
  return GetResult(std::move(future)).ValueOrThrow();
}

}  // namespace exe::futures
