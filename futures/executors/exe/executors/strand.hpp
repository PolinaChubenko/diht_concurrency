#pragma once

#include <exe/executors/executor.hpp>
#include <exe/support/spinlock.hpp>
#include <twist/stdlike/atomic.hpp>

namespace exe::executors {

// Strand (serial executor, asynchronous mutex)
// Executes (via underlying executor) tasks
// non-concurrently and in FIFO order

class Strand : public IExecutor {
 public:
  explicit Strand(IExecutor& underlying);
  ~Strand();

  // IExecutor
  void Execute(Task task) override;

 private:
  void Submit();
  void Step();
  void DoTask();

 private:
  support::SpinLock spinlock_;
  IExecutor& underlying_;

  struct Node {
    Task task_;
    Node* next_{nullptr};
  };

  twist::stdlike::atomic<Node*> head_{new Node};
  twist::stdlike::atomic<Node*> tail_{head_.load()};

  // 0 - no submission, 1 - submitting, 3 - no new tasks, finish submission
  twist::stdlike::atomic<int32_t> state_{0};
};

}  // namespace exe::executors
