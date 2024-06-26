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

#ifndef marl_waitgroup_h
#define marl_waitgroup_h

#include "conditionvariable.h"
#include "debug.h"

#include <atomic>
#include <mutex>

namespace marl {

// WaitGroup is a synchronization primitive that holds an internal counter that
// can incremented, decremented and waited on until it reaches 0.
// WaitGroups can be used as a simple mechanism for waiting on a number of
// concurrently execute a number of tasks to complete.
//
// Example:
//
//  void runTasksConcurrently(int numConcurrentTasks)
//  {
//      // Construct the WaitGroup with an initial count of numConcurrentTasks.
//      marl::WaitGroup wg(numConcurrentTasks);
//      for (int i = 0; i < numConcurrentTasks; i++)
//      {
//          // Schedule a task to be run asynchronously.
//          // These may all be run concurrently.
//          marl::schedule([=] {
//              // Once the task has finished, decrement the waitgroup counter
//              // to signal that this has completed.
//              defer(wg.done());
//              doSomeWork();
//          });
//      }
//      // Block until all tasks have completed.
//      wg.wait();
//  }
class WaitGroup {
 public:
  // Constructs the WaitGroup with the specified initial count.
  MARL_NO_EXPORT inline WaitGroup(unsigned int initialCount = 0, bool reverse = false,
                                  Allocator* allocator = Allocator::Default);

  // add() increments the internal counter by count.
  MARL_NO_EXPORT inline void add(unsigned int count = 1) const;

  // done() decrements the internal counter by one.
  // Returns true if the internal count has reached zero.
  MARL_NO_EXPORT inline bool done() const;

  // wait() blocks until the WaitGroup counter reaches zero.
  MARL_NO_EXPORT inline void wait() const;

  MARL_NO_EXPORT inline bool test() const;

  MARL_NO_EXPORT inline WaitGroup(std::nullptr_t) {}

  MARL_NO_EXPORT inline WaitGroup(const WaitGroup&) = default;
  MARL_NO_EXPORT inline WaitGroup(WaitGroup&&) = default;
  MARL_NO_EXPORT inline WaitGroup& operator=(const WaitGroup& other) { data = other.data; return *this; }
  MARL_NO_EXPORT inline explicit operator bool() const { return (bool)data; }
  MARL_NO_EXPORT inline bool operator==(const WaitGroup& other) const { return data == other.data; }
  MARL_NO_EXPORT inline size_t hash() const { return std::hash<void*>{}(data.get()); }
 private:
  friend class WeakWaitGroup;
  struct Data {
    MARL_NO_EXPORT inline Data(Allocator* allocator);

    std::atomic<unsigned int> count = {0};
    bool inverse = false;
    ConditionVariable cv;
    marl::mutex mutex;
  };
  MARL_NO_EXPORT inline WaitGroup(marl::shared_ptr<Data>&& data)
    :data(std::move(data)) {}
  marl::shared_ptr<Data> data;
};

class WeakWaitGroup
{
  public:
    MARL_NO_EXPORT inline WeakWaitGroup() = default;
    MARL_NO_EXPORT inline WeakWaitGroup(const WaitGroup& wg) : data(wg.data) {}
    MARL_NO_EXPORT inline WeakWaitGroup(const WeakWaitGroup& wg) : data(wg.data) {}
    MARL_NO_EXPORT inline WeakWaitGroup(WeakWaitGroup&& wg) : data(std::move(wg.data)) {}
    MARL_NO_EXPORT inline WeakWaitGroup& operator=(const WeakWaitGroup& wg) { data = wg.data; return *this; }
    MARL_NO_EXPORT inline WeakWaitGroup& operator=(WeakWaitGroup&& wg) { data = std::move(wg.data); return *this; }
    MARL_NO_EXPORT inline bool expired() const { return data.expired(); }
    MARL_NO_EXPORT inline bool operator==(const WeakWaitGroup& other) const { return data.lock() == other.data.lock(); }
    MARL_NO_EXPORT inline size_t hash() const { return std::hash<void*>{}(data.lock().get()); }
    WaitGroup lock() const { return WaitGroup(data.lock()); }
  private:
    marl::weak_ptr<WaitGroup::Data> data;
};

WaitGroup::Data::Data(Allocator* allocator) : cv(allocator) {}

WaitGroup::WaitGroup(unsigned int initialCount /* = 0 */, bool reverse /* = 0*/,
                     Allocator* allocator /* = Allocator::Default */)
    : data(marl::make_shared<Data>(allocator)) {
  data->count = initialCount;
  data->inverse = reverse;
}

void WaitGroup::add(unsigned int count /* = 1 */) const {
  MARL_ASSERT(count > 0, "WaitGroup::add() count must be > 0");
  data->count += count;
  if(data->inverse)
  {
    marl::lock lock(data->mutex);
    data->cv.notify_all();
  }
}

bool WaitGroup::done() const {
  MARL_ASSERT(data->count > 0, "marl::WaitGroup::done() called too many times");
  auto count = --data->count;
  if(data->inverse)
    return false;
  if (count == 0) {
    marl::lock lock(data->mutex);
    data->cv.notify_all();
    return true;
  }
  return false;
}

void WaitGroup::wait() const {
  marl::lock lock(data->mutex);
  data->cv.wait(lock, [this] { return test(); });
}

bool WaitGroup::test() const {
  return (!data->inverse) == (data->count == 0);
}

}  // namespace marl

#endif  // marl_waitgroup_h
