#pragma once

#include <twist/stdlike/atomic.hpp>

#include <experimental/coroutine>

// std::unique_lock
#include <mutex>

namespace exe::tasks {

class Mutex {
  using UniqueLock = std::unique_lock<Mutex>;

  struct [[nodiscard]] Locker {
    Mutex& mutex_;

    explicit Locker(Mutex& mutex) : mutex_(mutex) {
    }

    // Awaiter protocol

    // NOLINTNEXTLINE
    bool await_ready() {
      return false;  // Not implemented
    }

    // NOLINTNEXTLINE
    void await_suspend(std::experimental::coroutine_handle<> /*handle*/) {
      // Not implemented
    }

    // NOLINTNEXTLINE
    UniqueLock await_resume() {
      std::abort();  // Not implemented
    }
  };

 public:
  // Asynchronous
  auto ScopedLock() {
    return Locker{*this};
  }

  bool TryLock() {
    return false;  // Not implemented
  }

  // For std::unique_lock
  // Do not use directly
  void unlock() {  // NOLINT
    Unlock();
  }

 private:
  // Returns true if lock acquired
  bool TryLockOrEnqueue(Locker* /*locker*/) {
    return false;  // Not implemented
  }

  void Unlock() {
    // Not implemented
  }

 private:
  // ???
};

}  // namespace exe::tasks
