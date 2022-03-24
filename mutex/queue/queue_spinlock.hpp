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
    QueueSpinLock& spinlock_;
    twist::stdlike::atomic<Guard*> next_{nullptr};
    twist::stdlike::atomic<bool> owner_{false};
  };

 private:
  void Acquire(Guard* node) {
    twist::stdlike::atomic<Guard*> prev_tail{tail_.exchange(node)};
    if (prev_tail.load() != nullptr) {
      prev_tail.load()->next_.store(node);
      twist::util::SpinWait spin_wait;
      while (!node->owner_.load()) {
        spin_wait();
      }
    } else {
      tail_.load()->owner_.store(true);
    }
  }

  void Release(Guard* node) {
    if (node->next_.load() == nullptr) {
      auto copy = node;
      if (tail_.compare_exchange_strong(copy, nullptr)) {
        //        node->owner_.store(0);
        return;
      }
      twist::util::SpinWait spin_wait;
      while (node->next_.load() == nullptr) {
        spin_wait();
      }
    }
    node->owner_.store(false);
    node->next_.load()->owner_.store(true);
  }

 private:
  twist::stdlike::atomic<Guard*> tail_{nullptr};
};

}  // namespace spinlocks
