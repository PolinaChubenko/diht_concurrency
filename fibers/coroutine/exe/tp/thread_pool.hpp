#pragma once

#include <exe/tp/task.hpp>
#include <exe/tp/blocking_queue.hpp>
#include <exe/tp/countdown.hpp>

#include <twist/stdlike/thread.hpp>
#include <twist/stdlike/atomic.hpp>
#include <vector>

namespace exe::tp {

// Fixed-size pool of worker threads

class ThreadPool {
 public:
  explicit ThreadPool(size_t workers);
  ~ThreadPool();

  // Non-copyable
  ThreadPool(const ThreadPool&) = delete;
  ThreadPool& operator=(const ThreadPool&) = delete;

  // Schedules task for execution in one of the worker threads
  // Do not use directly, use tp::Submit instead
  void Submit(Task task);

  // Waits until outstanding work count has reached zero
  void WaitIdle();

  // Stops the worker threads as soon as possible
  // Pending tasks will be discarded
  void Stop();

  // Locates current thread pool from worker thread
  static ThreadPool* Current();

 private:
  void StartWorkerThreads(size_t count);
  void JoinWorkerThreads();
  void Invoke(Task& task);
  void WorkerRoutine();

 private:
  std::vector<twist::stdlike::thread> workers_;
  UnboundedBlockingQueue<Task> task_queue_;
  Countdown task_counter_;
};

}  // namespace exe::tp
