#pragma once

#include <twist/stdlike/atomic.hpp>

#include <experimental/coroutine>

namespace exe::tasks {

class OneShotEvent {
 public:
  // Asynchronous
  auto Wait() {
    return std::experimental::suspend_never{};  // Not implemented
  }

  // One-shot
  void Fire() {
    // Not implemented
  }

 private:
  // ???
};

}  // namespace exe::tasks
