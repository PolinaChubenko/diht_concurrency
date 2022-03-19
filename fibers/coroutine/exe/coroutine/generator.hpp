#pragma once

#include <exe/coroutine/impl.hpp>
#include <context/stack.hpp>

#include <optional>

namespace exe::coroutine::generators {

template <typename T>
class Generator {
 public:
  explicit Generator(Routine /*routine*/) {
    // No impl
  }

  // Pull
  std::optional<T> Receive() {
    return std::nullopt;  // No impl
  }

  static void Send(T /*value*/) {
    // No impl
  }

 private:
  // Intentionally naive and inefficient
  static context::Stack AllocateStack() {
    static const size_t kStackPages = 16;  // 16 * 4KB = 64KB
    return context::Stack::AllocatePages(kStackPages);
  }

 private:
  /// ???
};

// Shortcut
template <typename T>
void Send(T value) {
  Generator<T>::Send(std::move(value));
}

}  // namespace exe::coroutine::generators
