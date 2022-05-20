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

template <typename F>
struct AutoDeletingTask : TaskBase {
  explicit AutoDeletingTask(F f) : function_(std::move(f)) {
  }

  void Run() noexcept override {
    try {
      function_();
    } catch (...) {
      std::cout << "Invalid task\n";
    }
  }

  void Discard() noexcept override {
    delete this;
  }

 private:
  F function_;
};

}  // namespace exe::executors