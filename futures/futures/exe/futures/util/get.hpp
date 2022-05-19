#pragma once

#include <exe/futures/core/future.hpp>

namespace exe::futures {

namespace detail {}  // namespace detail

// ~ std::future::get
// Blocking
template <typename T>
wheels::Result<T> GetResult(Future<T> future) {
  twist::stdlike::atomic<uint32_t> has_result{0};
  std::optional<wheels::Result<T>> result;

  std::move(future).Subscribe([&has_result, &result](wheels::Result<T> input) {
    result = std::move(input);
    has_result.store(1);
    has_result.notify_one();
  });

  while (has_result.load() == 0) {
    has_result.wait(0);
  }

  return std::move(*result);
}

// Blocking
template <typename T>
T GetValue(Future<T> future) {
  return GetResult(std::move(future)).ValueOrThrow();
}

}  // namespace exe::futures
