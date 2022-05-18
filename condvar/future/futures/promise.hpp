#pragma once

#include <futures/future.hpp>
#include <futures/shared_state.hpp>
#include <memory>

namespace stdlike {

template <typename T>
class Promise {
 public:
  Promise() {
  }

  // Non-copyable
  Promise(const Promise&) = delete;
  Promise& operator=(const Promise&) = delete;

  // Movable
  Promise(Promise&& another) = default;
  Promise& operator=(Promise&&) = default;

  // One-shot
  Future<T> MakeFuture() {
    state_ = std::make_shared<detail::SharedState<T>>();
    return Future<T>(state_);
  }

  // One-shot
  // Fulfill promise with value
  void SetValue(T value) {
    state_->SetValue(std::move(value));
  }

  // One-shot
  // Fulfill promise with exception
  void SetException(std::exception_ptr ex) {
    state_->SetException(std::move(ex));
  }

 private:
  std::shared_ptr<detail::SharedState<T>> state_;
};

}  // namespace stdlike
