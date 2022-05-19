#pragma once

#include <exe/futures/core/future.hpp>

namespace exe::futures {

template <typename T>
Future<T> JustValue(T /*value*/) {
  return Future<T>::Invalid();
}

}  // namespace exe::futures
