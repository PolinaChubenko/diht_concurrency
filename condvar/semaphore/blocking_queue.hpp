#pragma once

#include "tagged_semaphore.hpp"

#include <deque>

namespace solutions {

// Bounded Blocking Multi-Producer/Multi-Consumer (MPMC) Queue

template <typename T>
class BlockingQueue {
 public:
  explicit BlockingQueue(size_t capacity)
      : capacity_(capacity), size_(0), is_locked_(1) {
  }

  // Inserts the specified element into this queue,
  // waiting if necessary for space to become available.
  void Put(T element) {
    auto cap_token = capacity_.Acquire();

    auto lock_token = is_locked_.Acquire();
    deque_.emplace_back(std::move(element));
    is_locked_.Release(std::move(lock_token));

    size_.Release(std::move(cap_token));
  }

  // Retrieves and removes the head of this queue,
  // waiting if necessary until an element becomes available
  T Take() {
    auto size_token = size_.Acquire();

    auto lock_token = is_locked_.Acquire();
    auto element = std::move(deque_.front());
    deque_.pop_front();
    is_locked_.Release(std::move(lock_token));

    capacity_.Release(std::move(size_token));
    return element;
  }

 private:
  struct Tag {};
  TaggedSemaphore<Tag> capacity_;
  TaggedSemaphore<Tag> size_;

  struct LockTag {};
  TaggedSemaphore<LockTag> is_locked_;

  std::deque<T> deque_;
};

}  // namespace solutions
