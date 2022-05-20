#pragma once

#include <exe/futures/core/future.hpp>

namespace exe::futures {

namespace detail {

template <typename T>
struct OneShotEvent {
 public:
  void Fire() {
    fired_.store(1);
    fired_.notify_one();
  }

  void Wait() {
    while (fired_.load() == 0) {
      fired_.wait(0);
    }
  }

 private:
  twist::stdlike::atomic<uint32_t> fired_{0};
};

}  // namespace detail

// ~ std::future::get
// Blocking
template <typename T>
wheels::Result<T> GetResult(Future<T> future) {
  detail::OneShotEvent<T> one_shot_event;
  std::optional<wheels::Result<T>> result;

  std::move(future).Subscribe(
      [&one_shot_event, &result](wheels::Result<T> input) {
        result = std::move(input);
        one_shot_event.Fire();
      });

  one_shot_event.Wait();
  return std::move(*result);
}

// Blocking
template <typename T>
T GetValue(Future<T> future) {
  return GetResult(std::move(future)).ValueOrThrow();
}

}  // namespace exe::futures
