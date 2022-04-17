#pragma once

#include <wheels/support/function.hpp>

namespace exe::executors {

// Intrusive task?
using Task = wheels::UniqueFunction<void()>;

}  // namespace exe::executors
