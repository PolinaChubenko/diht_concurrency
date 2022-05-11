#pragma once

#include <exe/tasks/sync/one_shot_event.hpp>

#include <twist/stdlike/atomic.hpp>

namespace exe::tasks {

class WaitGroup {
 public:
  void Add(size_t /*count*/) {
    // Not implemented
  }

  void Done() {
    // Not implemented
  }

  // Asynchronous
  auto Wait() {
    return std::experimental::suspend_never{};  // Not implemented
  }

 private:
  // ???
};

}  // namespace exe::tasks
