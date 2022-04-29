#include <exe/fibers/core/stacks.hpp>

#include <exe/support/spinlock.hpp>
#include <mutex>
#include <optional>
#include <vector>

using context::Stack;

namespace exe::fibers {

//////////////////////////////////////////////////////////////////////

class StackAllocator {
 public:
  Stack Allocate() {
    if (auto stack = TryTakeFromPool()) {
      return std::move(*stack);
    }
    return AllocateNewStack();
  }

  void Release(Stack stack) {
    std::lock_guard guard_lock(spinlock_);
    pool_.push_back(std::move(stack));
  }

 private:
  static Stack AllocateNewStack() {
    static const size_t kStackPages = 16;  // 16 * 4KB = 64KB
    return Stack::AllocatePages(kStackPages);
  }

  std::optional<Stack> TryTakeFromPool() {
    std::lock_guard guard_lock(spinlock_);
    if (pool_.empty()) {
      return std::nullopt;
    }
    Stack stack = std::move(pool_.back());
    pool_.pop_back();
    return stack;
  }

 private:
  support::SpinLock spinlock_;
  std::vector<Stack> pool_;
};

//////////////////////////////////////////////////////////////////////

StackAllocator allocator;

context::Stack AllocateStack() {
  return allocator.Allocate();
}

void ReleaseStack(context::Stack stack) {
  allocator.Release(std::move(stack));
}

}  // namespace exe::fibers
