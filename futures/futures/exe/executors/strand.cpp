#include <exe/executors/strand.hpp>
#include <exe/executors/execute.hpp>

namespace exe::executors {

Strand::Strand(IExecutor& underlying) : underlying_(underlying) {
}

void Strand::Execute(TaskBase* task) {
  if (tasks_.Push(task) == 0) {
    Submit();
  }
}

void Strand::Submit() {
  ::exe::executors::Execute(underlying_, [this]() {
    Step();
  });
}

void Strand::Step() {
  wheels::IntrusiveForwardList<TaskBase> batch;
  tasks_.Grab(batch);
  size_t batch_sz = batch.Size();
  while (!batch.IsEmpty()) {
    auto task = batch.PopFront();
    task->Run();
    task->Discard();
  }
  if (tasks_.Drain(batch_sz) != batch_sz) {
    Submit();
  }
}

}  // namespace exe::executors
