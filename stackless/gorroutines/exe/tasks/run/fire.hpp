#pragma once

#include <exe/tasks/core/task.hpp>

namespace exe::tasks {

inline void FireAndForget(Task<>&& task) {
  task.ReleaseCoroutine().resume();
}

}  // namespace exe::tasks
