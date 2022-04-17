#include <exe/executors/strand.hpp>

namespace exe::executors {

Strand::Strand(IExecutor& underlying) : underlying_(underlying) {
}

Strand::~Strand() {
  delete head_.load();
}

void Strand::Execute(Task task) {
  auto new_node = new Node{std::move(task), nullptr};

  spinlock_.lock();
  auto cur_tail = tail_.exchange(new_node);
  cur_tail->next_ = new_node;
  spinlock_.unlock();

  if (state_.exchange(1) == 0) {
    Submit();
  }
}

void Strand::Submit() {
  underlying_.Execute([this]() {
    Step();
    int32_t expected = 3;
    if (!state_.compare_exchange_strong(expected, 0)) {
      Submit();
    }
  });
}

void Strand::Step() {
  spinlock_.lock();
  auto cur_tail = tail_.load();
  spinlock_.unlock();

  while (head_.load() != cur_tail) {
    DoTask();
  }

  spinlock_.lock();
  if (cur_tail == tail_.load()) {
    state_.store(3);
  }
  spinlock_.unlock();
}

void Strand::DoTask() {
  auto cur_head = head_.load();
  head_.store(head_.load()->next_);
  delete cur_head;
  head_.load()->task_();
}

}  // namespace exe::executors
