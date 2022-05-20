#include <exe/executors/strand.hpp>
#include <exe/executors/execute.hpp>

namespace exe::executors {

Strand::Strand(IExecutor& underlying) : underlying_(underlying) {
}

void Strand::Execute(TaskBase* task) {
  if (!tasks_.Push(task)) {
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
  while (!batch.IsEmpty()) {
    auto task = batch.PopFront();
    task->Run();
    task->Discard();
  }
  tasks_.Finish();
  if (tasks_.NeedResubmit()) {
    Submit();
  }
}

}  // namespace exe::executors
