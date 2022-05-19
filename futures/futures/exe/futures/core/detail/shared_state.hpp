#pragma once

#include <exe/futures/core/callback.hpp>
#include <twist/stdlike/atomic.hpp>

#include <exe/executors/executor.hpp>
#include <exe/executors/execute.hpp>

#include <wheels/result/result.hpp>
#include <wheels/support/function.hpp>

#include <optional>
#include <memory>

namespace exe::futures {

namespace detail {

//////////////////////////////////////////////////////////////////////

// State shared between Promise and Future

template <typename T>
class SharedState {
 public:
  explicit SharedState(executors::IExecutor* executor) : executor_(executor) {
  }

  bool HasResult() const {
    return state_ == State::has_result;
  }

  wheels::Result<T> GetReadyResult() {
    return std::move(*result_);
  }

  void SetExecutor(executors::IExecutor* executor) {
    executor_ = executor;
  }

  executors::IExecutor& GetExecutor() const {
    return *executor_;
  }

  // Producer
  // Progress guarantee: wait-free
  void SetResult(wheels::Result<T> result) {
    result_.emplace(std::move(result));
    UpdateState(State::has_result);
  }

  // Consumer
  // Progress guarantee: wait-free
  void SetCallback(Callback<T> callback) {
    callback_ = std::move(callback);
    UpdateState(State::has_callback);
  }

 private:
  void UpdateState(uint32_t with) {
    if (state_.fetch_or(with) == (State::completed ^ with)) {
      Finalize();
    }
  }

  void Finalize() {
    Execute(*executor_, [callback = std::move(callback_),
                         result = std::move(result_)]() mutable {
      callback(std::move(result.value()));
    });
  }

 private:
  std::optional<wheels::Result<T>> result_;
  Callback<T> callback_;
  executors::IExecutor* executor_;
  enum State { empty = 0, has_result = 1, has_callback = 2, completed = 3 };
  twist::stdlike::atomic<uint32_t> state_{State::empty};
};

//////////////////////////////////////////////////////////////////////

template <typename T>
using StateRef = std::shared_ptr<SharedState<T>>;

template <typename T>
StateRef<T> MakeSharedState(executors::IExecutor& executor) {
  return std::make_shared<SharedState<T>>(&executor);
}

//////////////////////////////////////////////////////////////////////

// Common base for Promise and Future

template <typename T>
class HoldState {
  using State = SharedState<T>;

 protected:
  explicit HoldState(StateRef<T> state) : state_(std::move(state)) {
  }

  // Movable
  HoldState(HoldState&& that) = default;
  HoldState& operator=(HoldState&& that) = default;

  // Non-copyable
  HoldState(const HoldState& that) = delete;
  HoldState& operator=(const HoldState& that) = delete;

  StateRef<T> ReleaseState() {
    CheckState();
    return std::move(state_);
  }

  bool HasState() const {
    return (bool)state_;
  }

  const State& AccessState() const {
    CheckState();
    return *state_.get();
  }

 private:
  void CheckState() const {
    WHEELS_VERIFY(HasState(), "No shared state / shared state released");
  }

 protected:
  StateRef<T> state_;
};

}  // namespace detail

}  // namespace exe::futures
