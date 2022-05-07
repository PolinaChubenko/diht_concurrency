#include <exe/executors/executor.hpp>
#include <twist/stdlike/atomic.hpp>

#include <stack>

namespace exe::executors {

namespace detail {

struct Node {
  Task task_;
  Node* next_{nullptr};
};

class LockFreeStack {
 public:
  void Push(Task task) {
    auto new_node = new Node{std::move(task)};
    auto cur_top = top_.load();
    new_node->next_ = cur_top;
    while (!top_.compare_exchange_weak(new_node->next_, new_node)) {
      ;  // Backoff
    }
  }

  void Swap(LockFreeStack& other) {
    top_.store(other.top_.exchange(top_.load()));
  }

  void GrabReversed(std::stack<Task>& storage) {
    while (top_.load() != nullptr) {
      auto cur_node = top_.exchange(top_.load()->next_);
      storage.push(std::move(cur_node->task_));
      delete cur_node;
    }
  }

 private:
  twist::stdlike::atomic<Node*> top_{nullptr};
};

}  // namespace detail

class CountingQueue {
 public:
  size_t Push(Task task) {
    size_t initial_sz = counter_.fetch_add(1);
    tasks_.Push(std::move(task));
    return initial_sz;
  }

  size_t Drain(size_t done) {
    return counter_.fetch_sub(done);
  }

  void Grab(std::stack<Task>& batch) {
    detail::LockFreeStack cur_tasks;
    cur_tasks.Swap(tasks_);
    cur_tasks.GrabReversed(batch);
  }

 private:
  detail::LockFreeStack tasks_;
  twist::stdlike::atomic<size_t> counter_{0};
};

}  // namespace exe::executors
