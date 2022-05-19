#pragma once

#include <exe/futures/core/future.hpp>
#include <exe/futures/combine/detail/combine.hpp>
#include <exe/futures/util/just.hpp>
#include <twist/stdlike/mutex.hpp>

#include <wheels/support/vector.hpp>

namespace exe::futures {

namespace detail {

template <typename T>
class AllCombinator {
 public:
  explicit AllCombinator(size_t inputs) : inputs_(inputs) {
    collection_.reserve(inputs);
  }

  void AddResult(wheels::Result<T> result) {
    if (promise_.has_value()) {
      if (result.HasError()) {
        std::move(*promise_).SetError(result.GetError());
      } else {
        std::lock_guard lock(mutex_);
        collection_.template emplace_back(std::move(result.ValueUnsafe()));
        if (collection_.size() == inputs_) {
          std::move(*promise_).SetValue(std::move(collection_));
        }
      }
    }
  }

  auto MakeFuture() {
    auto [f, p] = MakeContract<std::vector<T>>();
    promise_.template emplace(std::move(p));
    return std::move(f);
  }

 private:
  size_t inputs_;
  std::optional<Promise<std::vector<T>>> promise_;
  twist::stdlike::mutex mutex_;
  std::vector<T> collection_;
};

}  // namespace detail

// All combinator
// All values or first error
// std::vector<Future<T>> -> Future<std::vector<T>>

template <typename T>
Future<std::vector<T>> All(std::vector<Future<T>> inputs) {
  if (inputs.empty()) {
    return JustValue<std::vector<T>>({});
  }
  return detail::Combine<detail::AllCombinator<T>>(std::move(inputs));
}

// Usage:
// auto all = futures::All(std::move(f1), std::move(f2));

template <typename T, typename... Fs>
Future<std::vector<T>> All(Future<T>&& first, Fs&&... rest) {
  return All(wheels::ToVector(std::move(first), std::forward<Fs>(rest)...));
}

}  // namespace exe::futures
