#pragma once

#include <exe/executors/executor.hpp>
#include <exe/futures/core/future.hpp>

namespace exe::futures {

// Usage:
// auto f = futures::Execute(pool, []() -> int {
//   return 42;  // <-- Computation runs in provided executor
// });

template <typename F>
auto Execute(executors::IExecutor& where, F func) {
  using T = std::invoke_result_t<F>;

  auto [f, p] = MakeContractVia<T>(where);
  executors::Execute(where,
                     [p = std::move(p), func = std::move(func)]() mutable {
                       try {
                         std::move(p).SetValue(std::move(func()));
                       } catch (...) {
                         std::move(p).SetError(std::current_exception());
                       }
                     });

  return std::move(f);
}

}  // namespace exe::futures
