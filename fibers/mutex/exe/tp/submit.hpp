#pragma once

#include <exe/tp/thread_pool.hpp>

namespace exe::tp {

template <typename F>
void Submit(ThreadPool& thread_pool, F&& f) {
  thread_pool.Submit(std::forward<F>(f));
}

}  // namespace exe::tp
