#pragma once

#include <exe/fibers/core/handle.hpp>

namespace exe::fibers {

struct IAwaiter {
  FiberHandle* GetHandle() {
    return handle_;
  }

 private:
  FiberHandle* handle_;
};

}  // namespace exe::fibers
