#pragma once

#include "tagged_semaphore.hpp"

#include <deque>

namespace solutions {

// Bounded Blocking Multi-Producer/Multi-Consumer (MPMC) Queue

template <typename T>
class BlockingQueue {
 public:
  explicit BlockingQueue(size_t capacity)
      : capacity_(capacity), size_(0), control_atomicity_(1) {
  }

  // Inserts the specified element into this queue,
  // waiting if necessary for space to become available.
  void Put(T element) {
    capacity_.Acquire();
    control_atomicity_.Acquire();
    size_.Release();
    deque_.emplace_back(std::move(element));
    control_atomicity_.Release();
  }

  // Retrieves and removes the head of this queue,
  // waiting if necessary until an element becomes available
  T Take() {
    size_.Acquire();
    control_atomicity_.Acquire();
    auto element = std::move(deque_[0]);
    deque_.pop_front();
    control_atomicity_.Release();
    capacity_.Release();
    return element;
  }

 private:
  Semaphore capacity_;
  Semaphore size_;
  Semaphore control_atomicity_;
  std::deque<T> deque_;
};

}  // namespace solutions
