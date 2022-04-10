#include <exe/coroutine/impl.hpp>

#include <context/stack.hpp>

namespace exe::coroutine {

// Standalone coroutine

class Coroutine {
 public:
  explicit Coroutine(Routine routine);

  void Resume();

  // Suspend current coroutine
  static void Suspend();

  bool IsCompleted() {
    return impl_.IsCompleted();
  }

 private:
  static context::Stack AllocateStack();

 private:
  context::Stack stack_;
  CoroutineImpl impl_;
};

}  // namespace exe::coroutine
