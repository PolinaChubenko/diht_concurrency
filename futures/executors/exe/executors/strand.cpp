#include <exe/executors/strand.hpp>

namespace exe::executors {

Strand::Strand(IExecutor& underlying) : underlying_(underlying) {
}

void Strand::Execute(Task task) {
  if (tasks_.Push(std::move(task)) == 0) {
    Submit();
  }
}

void Strand::Submit() {
  underlying_.Execute([this]() {
    Step();
  });
}

void Strand::Step() {
  std::stack<Task> batch;
  tasks_.Grab(batch);
  size_t batch_sz = batch.size();
  while (!batch.empty()) {
    Task task{std::move(batch.top())};
    batch.pop();
    task();
  }
  if (tasks_.Drain(batch_sz) != batch_sz) {
    Submit();
  }
}

}  // namespace exe::executors
