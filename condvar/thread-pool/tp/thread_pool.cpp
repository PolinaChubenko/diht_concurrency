#include <tp/thread_pool.hpp>

#include <twist/util/thread_local.hpp>

namespace tp {

////////////////////////////////////////////////////////////////////////////////

static twist::util::ThreadLocalPtr<ThreadPool> pool;

////////////////////////////////////////////////////////////////////////////////

ThreadPool::ThreadPool(size_t workers) {
  StartWorkerThreads(workers);
}

ThreadPool::~ThreadPool() {
  assert(is_stopped_.load());
}

void ThreadPool::Submit(Task task) {
  if (task_queue_.Put(std::move(task))) {
    count_tasks_.fetch_add(1);
  }
}

void ThreadPool::WaitIdle() {
  while (count_tasks_.load() != 0) {
  }
}

void ThreadPool::Stop() {
  size_t only_once = 0;
  if (is_stopped_.compare_exchange_strong(only_once, 1)) {
    task_queue_.Cancel();
    JoinWorkerThreads();
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
  while (is_stopped_.load() == 0) {
    auto task = task_queue_.Take();
    if (!task.has_value()) {
      break;
    }
    Invoke(task.value());
    count_tasks_.fetch_sub(1);
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

}  // namespace tp
