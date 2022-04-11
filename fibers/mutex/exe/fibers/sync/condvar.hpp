#pragma once

#include <exe/fibers/sync/mutex.hpp>
#include <exe/fibers/sync/futex.hpp>

#include <twist/stdlike/atomic.hpp>

// std::unique_lock
#include <mutex>

namespace exe::fibers {

class CondVar {
  using Lock = std::unique_lock<Mutex>;

 public:
  void Wait(Lock& lock) {
    uint32_t notified = notified_.load();
    lock.unlock();
    futex_.ParkIfEqual(notified);
    lock.lock();
  }

  void NotifyOne() {
    notified_.fetch_add(1);
    futex_.WakeOne();
  }

  void NotifyAll() {
    notified_.fetch_add(1);
    futex_.WakeAll();
  }

 private:
  twist::stdlike::atomic<uint32_t> notified_{0};
  FutexLike<uint32_t> futex_{notified_};
};

}  // namespace exe::fibers
