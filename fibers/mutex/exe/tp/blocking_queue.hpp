#pragma once

#include <twist/stdlike/mutex.hpp>
#include <twist/stdlike/condition_variable.hpp>

#include <deque>
#include <optional>

namespace exe::tp {

// Unbounded blocking multi-producers/multi-consumers queue

template <typename T>
class UnboundedBlockingQueue {
 public:
  bool Put(T value) {
    std::lock_guard<twist::stdlike::mutex> guard_lock(mutex_);
    if (is_closed_) {
      return false;
    }
    deque_.push_back(std::move(value));
    not_empty_.notify_one();
    return true;
  }

  std::optional<T> Take() {
    std::unique_lock<twist::stdlike::mutex> unique_lock(mutex_);
    not_empty_.wait(unique_lock, [&]() {
      return !(deque_.empty() && !is_closed_);
    });
    if (is_closed_ && deque_.empty()) {
      return std::nullopt;
    }
    auto value{std::move(deque_.front())};
    deque_.pop_front();
    return value;
  }

  void Close() {
    CloseImpl(/*clear=*/false);
  }

  void Cancel() {
    CloseImpl(/*clear=*/true);
  }

 private:
  void CloseImpl(bool clear) {
    std::lock_guard<twist::stdlike::mutex> guard_lock(mutex_);
    if (clear) {
      deque_.clear();
    }
    is_closed_ = true;
    not_empty_.notify_all();
  }

 private:
  twist::stdlike::condition_variable not_empty_;
  twist::stdlike::mutex mutex_;
  std::deque<T> deque_;
  bool is_closed_ = false;
};

}  // namespace exe::tp
