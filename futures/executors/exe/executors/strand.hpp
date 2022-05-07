#pragma once

#include <exe/executors/executor.hpp>
#include <twist/stdlike/atomic.hpp>
#include <exe/executors/counting_queue.hpp>

namespace exe::executors {

// Strand (serial executor, asynchronous mutex)
// Executes (via underlying executor) tasks
// non-concurrently and in FIFO order

class Strand : public IExecutor {
 public:
  explicit Strand(IExecutor& underlying);

  // IExecutor
  void Execute(Task task) override;

 private:
  void Submit();
  void Step();

 private:
  IExecutor& underlying_;
  CountingQueue tasks_;
};

}  // namespace exe::executors
