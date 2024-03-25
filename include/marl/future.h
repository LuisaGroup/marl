// Copyright 2019 The Marl Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef marl_future_h
#define marl_future_h

#include <EASTL/allocator.h>
#include <chrono>
#include "conditionvariable.h"
#include "containers.h"
#include "export.h"
#include "memory.h"

namespace marl {

// Future is a synchronization primitive used to block until a signal is raised.

class Future {
 public:
  Future(size_t mem_size, Allocator* allocator = Allocator::Default);

  void signal(eastl::move_only_function<void(void*)> const& new_ctor,
              void (*new_dtor)(void*)) const;

  // clear() clears the signaled state.
  void clear() const;

  // wait() blocks until the event is signaled.
  // If the event was constructed with the Auto Mode, then only one
  // call to wait() will unblock before returning, upon which the signalled
  // state will be automatically cleared.
  [[nodiscard]] void* wait() const;

  // test() returns true if the event is signaled, otherwise false.
  // If the event is signalled and was constructed with the Auto Mode
  // then the signalled state will be automatically cleared upon returning.
  [[nodiscard]] bool test() const;

  // isSignalled() returns true if the event is signaled, otherwise false.
  // Unlike test() the signal is not automatically cleared when the event was
  // constructed with the Auto Mode.
  // Note: No lock is held after bool() returns, so the event state may
  // immediately change after returning. Use with caution.
  [[nodiscard]] bool isSignalled() const;

 private:
  struct Shared {
    [[nodiscard]] Shared(Allocator* allocator);
    void signal(eastl::move_only_function<void(void*)> const& new_ctor,
                void (*new_dtor)(void*));
    void* wait();

    marl::mutex mutex;
    ConditionVariable cv;
    void (*dtor)(void*) = nullptr;
    [[no_unique_address]] std::byte placeholder[0];
    void* ptr() {
      constexpr auto aligned_size = (sizeof(Shared) + 15ull) & ~(15ull);
      constexpr auto offset = aligned_size - sizeof(Shared);
      return &placeholder[offset];
    }
    void dispose() {
      if (dtor) {
        dtor(ptr());
      }
      dtor = nullptr;
    }
    ~Shared() { dispose(); }
  };

  const eastl::shared_ptr<Shared> shared;
};

inline Future::Shared::Shared(Allocator* allocator) : cv(allocator) {}

inline void* Future::Shared::wait() {
  marl::lock lock(mutex);
  cv.wait(lock, [&] { return static_cast<bool>(dtor); });
  return ptr();
}

inline Future::Future(size_t mem_size,
                      Allocator* allocator /* = Allocator::Default */)
    : shared([&]() {
        auto aligned_size = (sizeof(Shared) + 15ull) & ~(15ull);
        auto ptr = eastl::GetDefaultAllocator()->allocate(
            aligned_size + mem_size, 0u, 0u);
        return new (ptr) Shared{allocator};
      }()) {}

inline void Future::signal(
    eastl::move_only_function<void(void*)> const& new_ctor,
    void (*new_dtor)(void*)) const {
  shared->signal(new_ctor, new_dtor);
}

inline void Future::Shared::signal(
    eastl::move_only_function<void(void*)> const& new_ctor,
    void (*new_dtor)(void*)) {
  marl::lock lock(mutex);
  dispose();
  new_ctor(ptr());
  dtor = new_dtor;
  cv.notify_all();
}

inline void Future::clear() const {
  marl::lock lock(shared->mutex);
  shared->dispose();
}

inline void* Future::wait() const {
  return shared->wait();
}

inline bool Future::test() const {
  marl::lock lock(shared->mutex);
  if (!shared->dtor) {
    return false;
  }
  return true;
}

inline bool Future::isSignalled() const {
  marl::lock lock(shared->mutex);
  return shared->dtor;
}

}  // namespace marl

#endif  // marl_event_h
