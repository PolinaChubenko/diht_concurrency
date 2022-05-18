#pragma once

#include <variant>
#include <twist/stdlike/condition_variable.hpp>
#include <twist/stdlike/mutex.hpp>

namespace stdlike::detail {

template <typename T>
class SharedState {
 public:
  SharedState() {
  }

  // Non-copyable
  SharedState(const SharedState&) = delete;
  SharedState& operator=(const SharedState&) = delete;

  // Non-movable
  SharedState(SharedState&&) = delete;
  SharedState& operator=(SharedState&&) = default;

  // One-shot
  void SetValue(T value) {
    std::lock_guard guard_lock(mutex_);
    data_.template emplace<1>(std::move(value));
    is_sent_.notify_one();
  }

  // One-shot
  void SetException(std::exception_ptr ex) {
    std::lock_guard guard_lock(mutex_);
    data_.template emplace<2>(ex);
    is_sent_.notify_one();
  }

  T Get() {
    std::unique_lock unique_lock(mutex_);
    while (data_.index() == 0) {
      is_sent_.wait(unique_lock);
    }
    if (auto ex = std::get_if<2>(&data_)) {
      std::rethrow_exception(*ex);
    } else {
      return std::move(std::get<1>(data_));
    }
  }

 private:
  std::variant<std::monostate, T, std::exception_ptr> data_;
  twist::stdlike::condition_variable is_sent_;
  twist::stdlike::mutex mutex_;
};

}  // namespace stdlike::detail