#pragma once

#include <exe/executors/executor.hpp>

#include <experimental/coroutine>

namespace exe::tasks {

namespace detail {

// Your awaiter goes here

}  // namespace detail

// Reschedule current coroutine to executor `target`
inline auto TeleportTo(executors::IExecutor& /*target*/) {
  return std::experimental::suspend_never{};  // Not implemented
}

}  // namespace exe::tasks
