#pragma once

#include <context/stack.hpp>

namespace exe::fibers {

context::Stack AllocateStack();
void ReleaseStack(context::Stack stack);

}  // namespace exe::fibers
