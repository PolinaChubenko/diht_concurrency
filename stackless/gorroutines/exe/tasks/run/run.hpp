#pragma once

#include <exe/tasks/core/task.hpp>

namespace exe::tasks {

// Blocks current thread
template <typename T>
T Run(Task<T>&& /*task*/) {
  std::abort();  // Not implemented
}

}  // namespace exe::tasks
