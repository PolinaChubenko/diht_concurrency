#pragma once

#include <exe/futures/core/future.hpp>
#include <exe/futures/combine/detail/combine.hpp>
#include <exe/futures/util/just.hpp>
#include <twist/stdlike/mutex.hpp>

#include <wheels/support/vector.hpp>

#include <iostream>

namespace exe::futures {

namespace detail {

template <typename T>
class AllCombinator {
 public:
  explicit AllCombinator(size_t inputs) : collection_(inputs, std::nullopt) {
  }

  void AddResult(size_t index, wheels::Result<T> result) {
    std::lock_guard guard_lock(mutex_);
    if (!finished_ && promise_.has_value()) {
      if (result.HasError()) {
        std::move(*promise_).SetError(std::move(result.GetError()));
        finished_ = true;
      } else {
        collection_[index] = std::move(result.ValueUnsafe());
        if (++inputs_ == collection_.size()) {
          std::vector<T> values;
          for (size_t i = 0; i < inputs_; ++i) {
            values.emplace_back(std::move(collection_[i].value()));
          }
          std::move(*promise_).SetValue(std::move(values));
          finished_ = true;
        }
      }
    }
  }

  auto MakeFuture() {
    auto [f, p] = MakeContract<std::vector<T>>();
    promise_.emplace(std::move(p));
    return std::move(f);
  }

 private:
  size_t inputs_ = 0;
  std::optional<Promise<std::vector<T>>> promise_;
  twist::stdlike::mutex mutex_;
  std::vector<std::optional<T>> collection_;
  bool finished_ = false;
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
