#pragma once

#include <twist/stdlike/mutex.hpp>
#include <twist/stdlike/condition_variable.hpp>
#include <wheels/intrusive/forward_list.hpp>

#include <deque>
#include <optional>

namespace exe::executors {

// Unbounded blocking multi-producers/multi-consumers queue

template <typename T>
class UnboundedBlockingQueue {
 public:
  bool Put(T* value) {
    std::lock_guard guard_lock(mutex_);
    if (is_closed_) {
      return false;
    }
    deque_.PushBack(value);
    not_empty_.notify_one();
    return true;
  }

  std::optional<T*> Take() {
    std::unique_lock unique_lock(mutex_);
    not_empty_.wait(unique_lock, [&]() {
      return !(deque_.IsEmpty() && !is_closed_);
    });
    if (is_closed_ && deque_.IsEmpty()) {
      return std::nullopt;
    }
    auto value{deque_.PopFront()};
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
    std::lock_guard guard_lock(mutex_);
    if (clear) {
      while (!deque_.IsEmpty()) {
        deque_.PopFront()->Discard();
      }
    }
    is_closed_ = true;
    not_empty_.notify_all();
  }

 private:
  twist::stdlike::condition_variable not_empty_;
  twist::stdlike::mutex mutex_;
  wheels::IntrusiveForwardList<T> deque_;
  bool is_closed_ = false;
};

}  // namespace exe::executors
