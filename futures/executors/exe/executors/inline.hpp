#pragma once

#include <exe/executors/executor.hpp>

namespace exe::executors {

// Executes task immediately in the current thread
IExecutor& GetInlineExecutor();

}  // namespace exe::executors
