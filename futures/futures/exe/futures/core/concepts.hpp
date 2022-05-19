#pragma once

#include <exe/futures/core/fwd.hpp>

#include <wheels/result/result.hpp>
#include <wheels/support/concepts.hpp>

#include <concepts>

namespace exe::futures {

// Continuation: T -> U

template <typename F, typename T>
concept Continuation = requires(F f, T value) {
  f(std::move(value));
};

// Asynchronous continuation: T -> Future<U>

template <typename F, typename T>
concept AsyncContinuation = requires(F f, T value) {
  { f(std::move(value)) } -> wheels::InstantiationOf<Future>;
};

// Synchronous continuation: T -> U != Future

template <typename F, typename T>
concept SyncContinuation = Continuation<F, T> && !AsyncContinuation<F, T>;

// Error Handler: Error -> Result<T>

template <typename H, typename T>
concept ErrorHandler = requires(H handler, wheels::Error error) {
  { handler(error) } -> std::same_as<wheels::Result<T>>;
};

}  // namespace exe::futures
