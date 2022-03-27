#pragma once

#include <twist/stdlike/atomic.hpp>
#include <twist/util/spin_wait.hpp>

namespace spinlocks {

/*  Scalable Queue SpinLock
 *
 *  Usage:
 *
 *  QueueSpinLock qspinlock;
 *  {
 *    QueueSpinLock::Guard guard(qspinlock);  // <-- Acquire
 *    // Critical section
 *  }  // <-- Release
 */

class QueueSpinLock {
 public:
  class Guard {
    friend class QueueSpinLock;

   public:
    explicit Guard(QueueSpinLock& spinlock) : spinlock_(spinlock) {
      spinlock_.Acquire(this);
    }

    ~Guard() {
      spinlock_.Release(this);
    }

   private:
    void Freedom() {
      twist::util::SpinWait spin_wait;
      while (!owner_.load()) {
        spin_wait();
      }
    }

    void Connection() {
      twist::util::SpinWait spin_wait;
      while (next_.load() == nullptr) {
        spin_wait();
      }
    }

   private:
    QueueSpinLock& spinlock_;
    twist::stdlike::atomic<Guard*> next_{nullptr};
    twist::stdlike::atomic<bool> owner_{false};
  };

 private:
  void Acquire(Guard* node) {
    Guard* prev_tail = tail_.exchange(node);
    if (prev_tail != nullptr) {
      prev_tail->next_.store(node);
      node->Freedom();
    } else {
      node->owner_.store(true);
    }
  }

  void Release(Guard* node) {
    if (node->next_.load() == nullptr) {
      Guard* copy = node;
      if (tail_.compare_exchange_strong(copy, nullptr)) {
        return;
      }
      node->Connection();
    }
    node->next_.load()->owner_.store(true);
  }

 private:
  twist::stdlike::atomic<Guard*> tail_{nullptr};
};

}  // namespace spinlocks
