#pragma once

#include <array>
#include <span>

#include <twist/stdlike/atomic.hpp>

namespace lockfree {

// Single-Producer / Multi-Consumer Bounded Ring Buffer

template <typename T, size_t Capacity>
class WorkStealingQueue {
  struct Slot {
    twist::stdlike::atomic<T*> item{nullptr};
  };

 public:
  bool TryPush(T* item) {
    size_t cur_head = head_.load(std::memory_order_acquire);
    size_t cur_tail = tail_.load(std::memory_order_acquire);
    if (!IsFull(cur_head, cur_tail)) {
      GetBuffer(cur_head).store(item, std::memory_order_relaxed);
      head_.fetch_add(1, std::memory_order_release);
      return true;
    }
    return false;
  }

  // Returns nullptr if queue is empty
  T* TryPop() {
    T* buffer[1];
    if (Grab({buffer, 1})) {
      return buffer[0];
    }
    return nullptr;
  }

  // Returns number of tasks
  size_t Grab(std::span<T*> out_buffer) {
    while (true) {
      size_t cur_tail = tail_.load(std::memory_order_acquire);
      size_t cur_head = head_.load(std::memory_order_acquire);
      if (cur_head - cur_tail >= 0) {
        size_t batch_sz = std::min(cur_head - cur_tail, out_buffer.size());
        for (size_t i = 0; i < batch_sz; ++i) {
          out_buffer[i] =
              GetBuffer(cur_tail + i).load(std::memory_order_relaxed);
        }
        if (tail_.compare_exchange_weak(cur_tail, cur_tail + batch_sz,
                                        std::memory_order_release,
                                        std::memory_order_relaxed)) {
          return batch_sz;
        }
      }
    }
  }

 private:
  bool IsFull(size_t head, size_t tail) {
    return Capacity == head - tail;
  }

  twist::stdlike::atomic<T*>& GetBuffer(size_t index) {
    return buffer_[index % Capacity].item;
  }

 private:
  std::array<Slot, Capacity> buffer_;
  twist::stdlike::atomic<size_t> head_{0};
  twist::stdlike::atomic<size_t> tail_{0};
};

}  // namespace lockfree
