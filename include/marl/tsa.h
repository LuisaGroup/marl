// Copyright 2020 The Marl Authors.
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

// Stubs Thread-Safty-Analysis annotation macros for platforms that do not
// support them.
// See https://clang.llvm.org/docs/ThreadSafetyAnalysis.html

#ifndef marl_tsa_h
#define marl_tsa_h

// Enable thread safety attributes only with clang.
// The attributes can be safely erased when compiling with other compilers.
#if defined(__clang__) && (!defined(SWIG))
#define MARL_THREAD_ANNOTATION_ATTRIBUTE__(x) __attribute__((x))
#else
#define MARL_THREAD_ANNOTATION_ATTRIBUTE__(x)  // no-op
#endif

#define MARL_CAPABILITY(x) MARL_THREAD_ANNOTATION_ATTRIBUTE__(capability(x))

#define MARL_SCOPED_CAPABILITY MARL_THREAD_ANNOTATION_ATTRIBUTE__(scoped_lockable)

#define MARL_GUARDED_BY(x) MARL_THREAD_ANNOTATION_ATTRIBUTE__(guarded_by(x))

#define MARL_PT_GUARDED_BY(x) MARL_THREAD_ANNOTATION_ATTRIBUTE__(pt_guarded_by(x))

#define MARL_ACQUIRED_BEFORE(...) \
  MARL_THREAD_ANNOTATION_ATTRIBUTE__(acquired_before(__VA_ARGS__))

#define MARL_ACQUIRED_AFTER(...) \
  MARL_THREAD_ANNOTATION_ATTRIBUTE__(acquired_after(__VA_ARGS__))

#define MARL_REQUIRES(...) \
  MARL_THREAD_ANNOTATION_ATTRIBUTE__(requires_capability(__VA_ARGS__))

#define MARL_REQUIRES_SHARED(...) \
  MARL_THREAD_ANNOTATION_ATTRIBUTE__(requires_shared_capability(__VA_ARGS__))

#define MARL_ACQUIRE(...) \
  MARL_THREAD_ANNOTATION_ATTRIBUTE__(acquire_capability(__VA_ARGS__))

#define MARL_ACQUIRE_SHARED(...) \
  MARL_THREAD_ANNOTATION_ATTRIBUTE__(acquire_shared_capability(__VA_ARGS__))

#define MARL_RELEASE(...) \
  MARL_THREAD_ANNOTATION_ATTRIBUTE__(release_capability(__VA_ARGS__))

#define MARL_RELEASE_SHARED(...) \
  MARL_THREAD_ANNOTATION_ATTRIBUTE__(release_shared_capability(__VA_ARGS__))

#define MARL_TRY_ACQUIRE(...) \
  MARL_THREAD_ANNOTATION_ATTRIBUTE__(try_acquire_capability(__VA_ARGS__))

#define MARL_TRY_ACQUIRE_SHARED(...) \
  MARL_THREAD_ANNOTATION_ATTRIBUTE__(try_acquire_shared_capability(__VA_ARGS__))

#define MARL_EXCLUDES(...) MARL_THREAD_ANNOTATION_ATTRIBUTE__(locks_excluded(__VA_ARGS__))

#define MARL_ASSERT_CAPABILITY(x) MARL_THREAD_ANNOTATION_ATTRIBUTE__(assert_capability(x))

#define MARL_ASSERT_SHARED_CAPABILITY(x) \
  MARL_THREAD_ANNOTATION_ATTRIBUTE__(assert_shared_capability(x))

#define MARL_RETURN_CAPABILITY(x) MARL_THREAD_ANNOTATION_ATTRIBUTE__(lock_returned(x))

#define MARL_NO_THREAD_SAFETY_ANALYSIS \
  MARL_THREAD_ANNOTATION_ATTRIBUTE__(no_thread_safety_analysis)

#endif  // marl_tsa_h
