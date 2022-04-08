#pragma once

#include <twist/stdlike/atomic.hpp>

#include <cstdint>

namespace stdlike {

class CondVar {
 public:
  // Mutex - BasicLockable
  // https://en.cppreference.com/w/cpp/named_req/BasicLockable
  template <class Mutex>
  void Wait(Mutex& mutex) {
    uint32_t notified = notified_.load();
    mutex.unlock();
    notified_.wait(notified);
    mutex.lock();
  }

  void NotifyOne() {
    notified_.fetch_add(1);
    notified_.notify_one();
  }

  void NotifyAll() {
    notified_.fetch_add(1);
    notified_.notify_all();
  }

 private:
  twist::stdlike::atomic<uint32_t> notified_{0};
};

}  // namespace stdlike
