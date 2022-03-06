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
}

void ThreadPool::Submit(Task task) {
  task_queue_.Put(std::move(task));
}

void ThreadPool::WaitIdle() {
}

void ThreadPool::Stop() {
}

void ThreadPool::StartWorkerThreads(size_t count) {
  for (size_t i = 0; i < count; ++i) {
    workers_.emplace_back([this]() {
      WorkerRoutine();
    });
  }
}

void ThreadPool::WorkerRoutine() {
  while (true) {
    auto task = task_queue_.Take();
    task.operator->();  // exceptions
    break;              // NOPE
  }
}

ThreadPool* ThreadPool::Current() {
  return pool;
}

}  // namespace tp
