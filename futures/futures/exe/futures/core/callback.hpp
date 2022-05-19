#pragma once

#include <wheels/result/result.hpp>
#include <wheels/support/function.hpp>

namespace exe::futures {

// Intrusive?
// Asynchronous callback
template <typename T>
using Callback = wheels::UniqueFunction<void(wheels::Result<T>)>;

}  // namespace exe::futures
