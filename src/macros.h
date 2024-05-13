// Copyright 2024 Miles Barr
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef PLX_MACROS_H
#define PLX_MACROS_H

// Likely and unlikely
#ifdef __GNUC__
#define plx_likely(x) __builtin_expect(!!(x), 1)
#define plx_unlikely(x) __builtin_expect(!!(x), 0)
#else
#define plx_likely(x) (x)
#define plx_unlikely(x) (x)
#endif  // __GNUC__

// Thread local
#ifdef __GNUC__
#define thread_local __thread
#elif defined(_MSC_VER)
#define thread_local __declspec(thread)
#else
#define thread_local _Thread_local
#endif

#endif  // PLX_MACROS_H
