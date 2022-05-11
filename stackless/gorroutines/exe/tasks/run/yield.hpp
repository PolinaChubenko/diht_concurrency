#pragma once

#include <exe/tasks/run/teleport.hpp>

namespace exe::tasks {

// Precondition: coroutine is running in `current` executor
inline auto Yield(executors::IExecutor& current) {
  return TeleportTo(current);
}

}  // namespace exe::tasks
