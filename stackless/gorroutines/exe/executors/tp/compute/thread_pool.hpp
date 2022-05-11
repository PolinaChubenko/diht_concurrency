#pragma once

#include <exe/executors/executor.hpp>
#include <exe/executors/tp/blocking_queue.hpp>
#include <exe/executors/tp/countdown.hpp>

#include <twist/stdlike/thread.hpp>
#include <twist/stdlike/atomic.hpp>
#include <vector>

#include <cstdlib>

namespace exe::executors::tp::compute {

// Thread pool for independent CPU-bound tasks
// Fixed pool of worker threads + shared unbounded blocking queue

class ThreadPool : public IExecutor {
 public:
  explicit ThreadPool(size_t threads);
  ~ThreadPool();

  // Non-copyable
  ThreadPool(const ThreadPool&) = delete;
  ThreadPool& operator=(const ThreadPool&) = delete;

  // IExecutor
  void Execute(TaskBase* task) override;

  void WaitIdle();

  void Stop();

  static ThreadPool* Current();

 private:
  void StartWorkerThreads(size_t count);
  void JoinWorkerThreads();
  void Invoke(TaskBase* task);
  void WorkerRoutine();

 private:
  std::vector<twist::stdlike::thread> workers_;
  UnboundedBlockingQueue<TaskBase> task_queue_;
  Countdown task_counter_;
};

}  // namespace exe::executors::tp::compute
