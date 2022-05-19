#pragma once

#include <exe/futures/core/future.hpp>
#include <exe/futures/combine/detail/combine.hpp>

#include <wheels/support/vector.hpp>

namespace exe::futures {

//////////////////////////////////////////////////////////////////////

namespace detail {

template <typename T>
class FirstOfCombinator {
 public:
  explicit FirstOfCombinator(size_t inputs) : inputs_(inputs) {
  }

  void AddResult(size_t /*index*/, wheels::Result<T> result) {
    if (!finished_.load()) {
      if (result.HasError()) {
        if (inputs_.fetch_sub(1) == 1) {
          std::move(*promise_).SetError(result.GetError());
        }
      } else if (!finished_.exchange(true)) {
        std::move(*promise_).SetValue(std::move(result));
      }
    }
  }

  auto MakeFuture() {
    auto [f, p] = MakeContract<T>();
    promise_.template emplace(std::move(p));
    return std::move(f);
  }

 private:
  twist::stdlike::atomic<size_t> inputs_;
  twist::stdlike::atomic<bool> finished_{false};
  std::optional<Promise<T>> promise_;
};

}  // namespace detail

// FirstOf combinator
// First value or last error
// std::vector<Future<T>> -> Future<T>

template <typename T>
Future<T> FirstOf(std::vector<Future<T>> inputs) {
  return detail::Combine<detail::FirstOfCombinator<T>>(std::move(inputs));
}

// Usage:
// auto first_of = futures::FirstOf(std::move(f1), std::move(f2));

template <typename T, typename... Fs>
auto FirstOf(Future<T>&& first, Fs&&... rest) {
  return FirstOf(wheels::ToVector(std::move(first), std::forward<Fs>(rest)...));
}

}  // namespace exe::futures
