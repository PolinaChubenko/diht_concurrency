#pragma once

#include <twist/stdlike/mutex.hpp>
#include <twist/stdlike/condition_variable.hpp>

// std::lock_guard, std::unique_lock
#include <mutex>
#include <cstdint>

namespace solutions {

// A Counting semaphore

// Semaphores are often used to restrict the number of threads
// than can access some (physical or logical) resource

class Semaphore {
 public:
  // Creates a Semaphore with the given number of permits
  explicit Semaphore(size_t tokens) : tokens_(tokens) {
  }

  // Acquires a permit from this semaphore,
  // blocking until one is available
  void Acquire() {
    std::unique_lock<twist::stdlike::mutex> unique_lock(mutex_);
    not_empty_.wait(unique_lock, [&]() {
      return tokens_ > 0;
    });
    --tokens_;
  }

  // Releases a permit, returning it to the semaphore
  void Release() {
    std::lock_guard<twist::stdlike::mutex> guard_lock(mutex_);
    if (++tokens_ > 0) {
      not_empty_.notify_all();
    }
  }

 private:
  twist::stdlike::condition_variable not_empty_;
  size_t tokens_;
  twist::stdlike::mutex mutex_;
};

}  // namespace solutions