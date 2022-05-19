#pragma once

#include <exe/futures/core/future.hpp>

#include <vector>

namespace exe::futures::detail {

// Generic Combine algorithm

template <typename Combinator, typename T, typename... Args>
auto Combine(std::vector<Future<T>> futures, Args&&... args) {
  auto combinator =
      std::make_shared<Combinator>(futures.size(), std::forward<Args>(args)...);

  auto f = combinator->MakeFuture();

  for (size_t i = 0; i < futures.size(); ++i) {
    std::move(futures[i])
        .Subscribe([combinator = combinator,
                    index = i](wheels::Result<T>&& result) mutable {
          combinator->AddResult(index, std::move(result));
        });
  }

  return std::move(f);
}

}  // namespace exe::futures::detail
