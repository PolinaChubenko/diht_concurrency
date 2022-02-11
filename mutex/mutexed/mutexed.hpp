#pragma once

#include <twist/stdlike/mutex.hpp>

namespace util {

//////////////////////////////////////////////////////////////////////

// Safe API for mutual exclusion

template <typename T>
class Mutexed {
  using MutexImpl = twist::stdlike::mutex;

  class UniqueRef {
   private:
    Mutexed<T>* const mutexed_;
    std::lock_guard<MutexImpl> lock_guard_;

   public:
    explicit UniqueRef(Mutexed<T>& mutexed)
        : mutexed_(&mutexed), lock_guard_(mutexed_->mutex_) {
    }

    // Non-copyable
    UniqueRef(const UniqueRef&) = delete;

    // Non-movable
    UniqueRef(UniqueRef&&) = delete;

    T& operator*() {
      return mutexed_->object_;
    }

    T* operator->() {
      return &(mutexed_->object_);
    }
  };

 public:
  // https://eli.thegreenplace.net/2014/perfect-forwarding-and-universal-references-in-c/
  template <typename... Args>
  explicit Mutexed(Args&&... args) : object_(std::forward<Args>(args)...) {
  }

  UniqueRef Lock() {
    return UniqueRef(*this);
  }

 private:
  T object_;
  MutexImpl mutex_;  // Guards access to object_
};

//////////////////////////////////////////////////////////////////////

// Helper function for single operations over shared object:
// Usage:
//   Mutexed<vector<int>> ints;
//   Locked(ints)->push_back(42);

template <typename T>
auto Locked(Mutexed<T>& object) {
  return object.Lock();
}

}  // namespace util
