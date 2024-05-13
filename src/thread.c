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

#include "thread.h"

#ifdef _WIN32

void plx_thread_init(plx_thread* const thread,
                     const plx_thread_start_routine start_routine,
                     void* const arg) {
  *thread = CreateThread(NULL, 0, start_routine, NULL, 0, NULL);
}

void plx_thread_join(plx_thread* const thread) {
  WaitForSingleObject(*thread, INFINITE);
  CloseHandle(*thread);
}

#else

void plx_thread_init(plx_thread* const thread,
                     const plx_thread_start_routine start_routine,
                     void* const arg) {
  pthread_create(thread, NULL, start_routine, arg);
}

void plx_thread_join(plx_thread* const thread) { pthread_join(*thread, NULL); }

#endif  // _WIN32
