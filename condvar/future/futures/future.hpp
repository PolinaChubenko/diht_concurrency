#pragma once

#include <futures/shared_state.hpp>
#include <memory>
#include <cassert>

namespace stdlike {

template <typename T>
class Future {
  template <typename U>
  friend class Promise;

 public:
  // Non-copyable
  Future(const Future&) = delete;
  Future& operator=(const Future&) = delete;

  // Movable
  Future(Future&&) = default;
  Future& operator=(Future&&) = default;

  // One-shot
  // Wait for result (value or exception)
  T Get() {
    return state_->Get();
  }

 private:
  explicit Future(std::shared_ptr<detail::SharedState<T>> state)
      : state_(state) {
  }

 private:
  std::shared_ptr<detail::SharedState<T>> state_;
};

}  // namespace stdlike
