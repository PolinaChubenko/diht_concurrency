#pragma once

#include <exe/futures/core/detail/shared_state.hpp>
#include <exe/futures/core/promise.hpp>
#include <exe/futures/core/concepts.hpp>

#include <exe/executors/inline.hpp>

namespace exe::futures {

//////////////////////////////////////////////////////////////////////

/*
 * Affine-like future: future value can be
 * consumed at most once (synchronously via GetResult/Await
 * or asynchronously via callback/continuation/combinator)
 */

template <typename T>
class [[nodiscard]] Future : public detail::HoldState<T> {
  template <typename U>
  friend Contract<U> MakeContractVia(executors::IExecutor&);

  using detail::HoldState<T>::HasState;
  using detail::HoldState<T>::AccessState;
  using detail::HoldState<T>::ReleaseState;

 public:
  static Future<T> Invalid();

  // True if this future has a shared state
  // False if result has already been consumed
  // 1) synchronously via GetReadyResult / GetResult or
  // 2) via Subscribe
  bool IsValid() const {
    return HasState();
  }

  // Non-blocking
  // True if this future has result in its shared state
  bool IsReady() const {
    return AccessState().HasResult();
  }

  // Non-blocking, one-shot
  // Pre-condition: IsReady() == true
  wheels::Result<T> GetReadyResult() && {
    return ReleaseState()->GetReadyResult();
  }

  // Asynchronous API

  // Set executor for asynchronous callback / continuation
  // Usage: std::move(f).Via(e).Then(c)
  Future<T> Via(executors::IExecutor& executor) &&;

  executors::IExecutor& GetExecutor() const;

  // Consume future result with asynchronous callback
  // Post-condition: IsValid() == false
  void Subscribe(Callback<T> callback) &&;

  // Combinators

  // Synchronous Then (also known as Map)
  // Future<T> -> U(T) -> Future<U>
  template <typename F>
  requires SyncContinuation<F, T>
  auto Then(F continuation) &&;

  // Asynchronous Then (also known as FlatMap)
  // Future<T> -> Future<U>(T) -> Future<U>
  template <typename F>
  requires AsyncContinuation<F, T>
  auto Then(F continuation) &&;

  // Error handling
  // Future<T> -> Result<T>(Error) -> Future<T>
  template <typename F>
  requires ErrorHandler<F, T> Future<T> Recover(F error_handler) &&;

 private:
  explicit Future(detail::StateRef<T> state)
      : detail::HoldState<T>(std::move(state)) {
  }
};

//////////////////////////////////////////////////////////////////////

template <typename T>
struct Contract {
  Future<T> future;
  Promise<T> promise;
};

// Usage:
// auto [f, p] = futures::MakeContract<T>();
// https://en.cppreference.com/w/cpp/language/structured_binding

template <typename T>
Contract<T> MakeContractVia(executors::IExecutor& e) {
  auto state = detail::MakeSharedState<T>(e);
  return {Future<T>{state}, Promise<T>{state}};
}

template <typename T>
Contract<T> MakeContract() {
  return MakeContractVia<T>(executors::GetInlineExecutor());
}

}  // namespace exe::futures

//////////////////////////////////////////////////////////////////////

#define FUTURE_IMPL
#include <exe/futures/core/future.ipp>
#undef FUTURE_IMPL
