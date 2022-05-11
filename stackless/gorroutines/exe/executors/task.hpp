#pragma once

#include <wheels/intrusive/forward_list.hpp>
#include <wheels/support/function.hpp>

namespace exe::executors {

struct ITask {
  virtual ~ITask() = default;

  virtual void Run() noexcept = 0;
  virtual void Discard() noexcept = 0;
};

// Intrusive task
struct TaskBase : ITask, wheels::IntrusiveForwardListNode<TaskBase> {};

struct TaskLambda : TaskBase {
  template <typename F>
  explicit TaskLambda(F&& f) : function_(std::forward<F>(f)) {
    //    std::cout << "CONSTRUCT\n";
  }

  void Run() noexcept override {
    try {
      function_();
    } catch (...) {
      // noting to do;
    }
  }

  void Discard() noexcept override {
    //    std::cout << "CLEAR\n";
    delete this;
  }

 private:
  wheels::UniqueFunction<void()> function_;
};

}  // namespace exe::executors
