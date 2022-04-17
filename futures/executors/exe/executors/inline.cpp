#include <exe/executors/inline.hpp>

namespace exe::executors {

class InlineExecutor : public IExecutor {
 public:
  // IExecutor
  void Execute(Task task) override {
    task();
  }
};

IExecutor& GetInlineExecutor() {
  static InlineExecutor instance;
  return instance;
}

}  // namespace exe::executors
