#pragma once

#include <exe/futures/core/fwd.hpp>

#include <exe/futures/core/detail/shared_state.hpp>

#include <wheels/result/result.hpp>
#include <wheels/result/make.hpp>

namespace exe::futures {

//////////////////////////////////////////////////////////////////////

template <typename T>
class Promise : public detail::HoldState<T> {
  template <typename U>
  friend Contract<U> MakeContractVia(executors::IExecutor&);

  using detail::HoldState<T>::ReleaseState;

 public:
  void Set(wheels::Result<T> result) && {
    ReleaseState()->SetResult(std::move(result));
  }

  void SetError(wheels::Error error) && {
    ReleaseState()->SetResult(wheels::make_result::Fail(std::move(error)));
  }

  void SetValue(T value) && {
    ReleaseState()->SetResult(wheels::make_result::Ok(std::move(value)));
  }

 private:
  explicit Promise(detail::StateRef<T> state)
      : detail::HoldState<T>(std::move(state)) {
  }
};

}  // namespace exe::futures
