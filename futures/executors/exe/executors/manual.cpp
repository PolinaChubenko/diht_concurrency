#include <exe/executors/manual.hpp>

namespace exe::executors {

void ManualExecutor::Execute(Task task) {
  tasks_queue_.push(std::move(task));
}

// Run tasks

size_t ManualExecutor::RunAtMost(size_t limit) {
  size_t tasks_done_cnt = 0;
  while (tasks_done_cnt < limit && HasTasks()) {
    ++tasks_done_cnt;
    DoTask();
  }
  return tasks_done_cnt;
}

size_t ManualExecutor::Drain() {
  return RunAtMost(-1);
}

void ManualExecutor::DoTask() {
  auto task = std::move(tasks_queue_.front());
  tasks_queue_.pop();
  task();
}

}  // namespace exe::executors
