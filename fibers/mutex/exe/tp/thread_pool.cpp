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
  count_tasks_.fetch_add(1);
  task_queue_.Put(std::move(task));
}

void ThreadPool::WaitIdle() {
  uint32_t tasks;
  while ((tasks = count_tasks_.load()) > 0) {
    count_tasks_.wait(tasks);
  }
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
      tp::pool = this;
      WorkerRoutine();
    });
  }
}

void ThreadPool::WorkerRoutine() {
  while (true) {
    auto task = task_queue_.Take();
    if (!task.has_value()) {
      break;
    }
    Invoke(task.value());
    count_tasks_.fetch_sub(1);
    count_tasks_.notify_all();
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
