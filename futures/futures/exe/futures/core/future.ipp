#ifndef FUTURE_IMPL
#error Do not include this file directly
#endif

#include <exe/futures/core/detail/traits.hpp>

namespace exe::futures {

//////////////////////////////////////////////////////////////////////

// Static constructors

template <typename T>
Future<T> Future<T>::Invalid() {
  return Future<T>{nullptr};
}

//////////////////////////////////////////////////////////////////////

// Executors

template <typename T>
Future<T> Future<T>::Via(executors::IExecutor& e) && {
  auto state = ReleaseState();
  state->SetExecutor(&e);
  return Future{std::move(state)};
}

template <typename T>
executors::IExecutor& Future<T>::GetExecutor() const {
  return AccessState().GetExecutor();
}

template <typename T>
void Future<T>::Subscribe(Callback<T> callback) && {
  return ReleaseState()->SetCallback(std::move(callback));
}

//////////////////////////////////////////////////////////////////////

// Synchronous Then

template <typename T>
template <typename F> requires SyncContinuation<F, T>
auto Future<T>::Then(F continuation) && {
  using U = std::invoke_result_t<F, T>;

  auto [f, p] = MakeContractVia<U>(GetExecutor());
  Callback<T> callback = [p = std::move(p), cont = std::move(continuation)]
      (wheels::Result<T>&& input) mutable {
    if (input.HasError()) {
      std::move(p).SetError(std::move(input.GetError()));
    } else {
      try {
        std::move(p).SetValue(cont(std::move(input)));
      } catch (...) {
        std::move(p).SetError(std::current_exception());
      }
    }
  };
  std::move(*this).Subscribe(std::move(callback));
  return std::move(f);
}

//////////////////////////////////////////////////////////////////////

// Asynchronous Then

template <typename T>
template <typename F> requires AsyncContinuation<F, T>
auto Future<T>::Then(F continuation) && {
  using U = typename detail::Flatten<std::invoke_result_t<F, T>>::ValueType;

  auto [f, p] = MakeContractVia<U>(GetExecutor());
  Callback<T> callback = [p = std::move(p), cont = std::move(continuation)]
      (wheels::Result<T>&& input) mutable {
    if (input.HasError()) {
      std::move(p).SetError(std::move(input.GetError()));
    } else {
      try {
        auto cont_future = cont(std::move(*input));
        std::move(cont_future).Subscribe([p = std::move(p)]
                                         (wheels::Result<U>&& result) mutable {
          std::move(p).SetValue(std::move(result));
        });
      } catch (...) {
        std::move(p).SetError(std::current_exception());
      }
    }
  };
  std::move(*this).Subscribe(std::move(callback));
  return std::move(f);
}

//////////////////////////////////////////////////////////////////////

// Recover

template <typename T>
template <typename F> requires ErrorHandler<F, T>
    Future<T> Future<T>::Recover(F error_handler) && {
  auto [f, p] = MakeContractVia<T>(GetExecutor());
  Callback<T> callback = [p = std::move(p), handler = std::move(error_handler)]
      (wheels::Result<T>&& input) mutable {
    if (input.HasError()) {
      std::move(p).SetValue(handler(std::move(input.GetError())));
    } else {
      std::move(p).Set(std::move(input));
    }
  };
  std::move(*this).Subscribe(std::move(callback));
  return std::move(f);

}

}  // namespace exe::futures
