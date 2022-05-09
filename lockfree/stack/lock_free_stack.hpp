#pragma once

#include "atomic_stamped_ptr.hpp"

#include <twist/stdlike/atomic.hpp>

#include <optional>

// Treiber unbounded lock-free stack
// https://en.wikipedia.org/wiki/Treiber_stack

template <typename T>
class LockFreeStack {
  struct Node {
    explicit Node(T value) : value(std::move(value)) {
    }

    T value;
    AtomicStampedPtr<Node> next{nullptr};
    twist::stdlike::atomic<uint32_t> decrements{0};
  };

 public:
  void Push(T value) {
    StampedPtr<Node> new_node{new Node(std::move(value)), 0};
    while (true) {
      auto cur_top = top_.Load();
      new_node->next.Store(cur_top);
      if (top_.CompareExchangeWeak(cur_top, new_node)) {
        return;
      }
    }
  }

  std::optional<T> TryPop() {
    StampedPtr<Node> checker{nullptr, 0};
    while (true) {
      FreeOnOverflow(checker);
      auto cur_top = top_.LoadPtr();
      checker = cur_top;
      if (cur_top.raw_ptr == nullptr) {
        return std::nullopt;
      }
      if (top_.CompareExchangeWeak(cur_top, cur_top->next.Load())) {
        return Pop(cur_top);
      }
    }
  }

  ~LockFreeStack() {
    StampedPtr<Node> top = top_.Load();
    while (top.raw_ptr != nullptr) {
      StampedPtr<Node> prev_top = top;
      top = prev_top->next.Load();
      delete prev_top.raw_ptr;
    }
  }

 private:
  static const uint32_t kOverflow = -1;

  void FreeOnOverflow(StampedPtr<Node>& node) {
    if (node.raw_ptr != nullptr && node->decrements.fetch_add(1) == kOverflow) {
      delete node.raw_ptr;
    }
  }

  std::optional<T> Pop(StampedPtr<Node>& node) {
    auto value = std::move(node->value);
    node->decrements.fetch_add(1);
    if (node->decrements.fetch_sub(node.stamp) == node.stamp) {
      delete node.raw_ptr;
    }
    return std::move(value);
  }

 private:
  AtomicStampedPtr<Node> top_{nullptr};
};
