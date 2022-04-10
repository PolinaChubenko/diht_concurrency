#pragma once

#include <wheels/support/function.hpp>

namespace exe::tp {

// Intrusive tasks?
using Task = wheels::UniqueFunction<void()>;

}  // namespace exe::tp
