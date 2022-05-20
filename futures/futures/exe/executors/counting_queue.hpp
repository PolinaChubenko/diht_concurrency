#include <exe/executors/executor.hpp>
#include <twist/stdlike/atomic.hpp>

#include <stack>

namespace exe::executors {

namespace detail {

struct Node {
  TaskBase* task_;
  Node* next_{nullptr};
};

class LockFreeStack {
 public:
  void Push(TaskBase* task) {
    auto new_node = new Node{task};
    auto cur_top = top_.load();
    new_node->next_ = cur_top;
    while (!top_.compare_exchange_weak(new_node->next_, new_node)) {
      ;  // Backoff
    }
  }

  bool IsEmpty() {
    return top_.load() == nullptr;
  }

  void Swap(LockFreeStack& other) {
    top_.store(other.top_.exchange(top_.load()));
  }

  void GrabReversed(wheels::IntrusiveForwardList<TaskBase>& storage) {
    while (top_.load() != nullptr) {
      auto cur_node = top_.exchange(top_.load()->next_);
      storage.PushFront(cur_node->task_);
      delete cur_node;
    }
  }

 private:
  twist::stdlike::atomic<Node*> top_{nullptr};
};

}  // namespace detail

class CountingQueue {
 public:
  bool Push(TaskBase* task) {
    tasks_.Push(task);
    return is_scheduled_.exchange(true);
  }

  void Finish() {
    is_scheduled_.store(false);
  }

  void Grab(wheels::IntrusiveForwardList<TaskBase>& batch) {
    detail::LockFreeStack cur_tasks;
    cur_tasks.Swap(tasks_);
    cur_tasks.GrabReversed(batch);
  }

  bool NeedResubmit() {
    return !tasks_.IsEmpty() && !is_scheduled_.exchange(true);
  }

 private:
  detail::LockFreeStack tasks_;
  twist::stdlike::atomic<bool> is_scheduled_{false};
};

}  // namespace exe::executors
