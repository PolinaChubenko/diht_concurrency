#include <exe/tp/thread_pool.hpp>

#include <twist/util/thread_local.hpp>

namespace exe::tp {

////////////////////////////////////////////////////////////////////////////////

static twist::util::ThreadLocalPtr<ThreadPool> pool;

////////////////////////////////////////////////////////////////////////////////

ThreadPool::ThreadPool(size_t workers) {
  StartWorkerThreads(workers);
}

ThreadPool::~ThreadPool() {
  assert(workers_.empty());
}

void ThreadPool::Submit(Task task) {
  task_counter_.Increment();
  task_queue_.Put(std::move(task));
}

void ThreadPool::WaitIdle() {
  task_counter_.WaitZero();
}

void ThreadPool::Stop() {
  if (!workers_.empty()) {
    task_queue_.Cancel();
    JoinWorkerThreads();
    workers_.clear();
  }
}

void ThreadPool::StartWorkerThreads(size_t count) {
  for (size_t i = 0; i < count; ++i) {
    workers_.emplace_back([this]() {
      WorkerRoutine();
    });
  }
}

void ThreadPool::WorkerRoutine() {
  tp::pool = this;
  while (auto task = task_queue_.Take()) {
    Invoke(task.value());
    task_counter_.Decrement();
  }
}

void ThreadPool::JoinWorkerThreads() {
  for (auto&& worker : workers_) {
    if (worker.joinable()) {
      worker.join();
    }
  }
}

void ThreadPool::Invoke(Task& task) {
  try {
    task();
  } catch (...) {
    std::cout << "Task is invalid\n";
  }
}

ThreadPool* ThreadPool::Current() {
  return pool;
}

}  // namespace exe::tp
