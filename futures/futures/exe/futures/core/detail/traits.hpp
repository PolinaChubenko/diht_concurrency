#pragma once

#include <exe/futures/core/fwd.hpp>

#include <wheels/result/result.hpp>

namespace exe::futures {

namespace detail {

template <typename T>
struct Flatten {
  using ValueType = T;
};

template <typename T>
struct Flatten<Future<T>> {
  using ValueType = typename Flatten<T>::ValueType;
};

}  // namespace detail

}  // namespace exe::futures
