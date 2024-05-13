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

#include "mutex.h"

#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

void plx_mutex_init(plx_mutex* const mutex) {
  *mutex = CreateMutex(NULL, FALSE, NULL);
}

void plx_mutex_destroy(plx_mutex* const mutex) { CloseHandle(*mutex); }

void plx_mutex_lock(plx_mutex* const mutex) {
  WaitForSingleObject(*mutex, INFINITE);
}

bool plx_mutex_try_lock(plx_mutex* const mutex) {
  return WaitForSingleObject(*mutex, 0) == WAIT_OBJECT_0;
}

void plx_mutex_unlock(plx_mutex* const mutex) { ReleaseMutex(*mutex); }

#else

void plx_mutex_init(plx_mutex* const mutex) {
  *mutex = PTHREAD_MUTEX_INITIALIZER;
}

void plx_mutex_destroy(plx_mutex* const mutex) { pthread_mutex_destroy(mutex); }

void plx_mutex_lock(plx_mutex* const mutex) { pthread_mutex_lock(mutex); }

bool plx_mutex_try_lock(plx_mutex* const mutex) {
  return pthread_mutex_trylock(mutex) == 0;
}

void plx_mutex_unlock(plx_mutex* const mutex) { pthread_mutex_unlock(mutex); }

#endif  // _WIN32
