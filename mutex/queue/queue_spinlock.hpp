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
    twist::stdlike::atomic<uint32_t> owner_{0};
  };

 private:
  void Acquire(Guard* node) {
    auto* prev_tail = tail_.exchange(node);
    if (prev_tail != nullptr) {
      prev_tail->next_.store(node);
      twist::util::SpinWait spin_wait;
      while (node->owner_.load() == 0) {
        spin_wait();
      }
    } else {
      (*tail_).owner_.store(1);
    }
  }

  void Release(Guard* node) {
    node->owner_.store(0);
    if (node->next_ != nullptr) {
      (*node->next_).owner_.store(1);
    } else {
      tail_.store(nullptr);
    }
  }

 private:
  twist::stdlike::atomic<Guard*> tail_{nullptr};
};

}  // namespace spinlocks
