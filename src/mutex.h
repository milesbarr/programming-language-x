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

#ifndef PLX_MUTEX_H
#define PLX_MUTEX_H

#include <stdbool.h>

#ifdef _WIN32
typedef void* plx_mutex;  // HANDLE
#else
#include <pthread.h>

typedef pthread_mutex_t plx_mutex;
#endif  // _WIN32

void plx_mutex_init(plx_mutex* mutex);
void plx_mutex_destroy(plx_mutex* mutex);
void plx_mutex_lock(plx_mutex* mutex);
bool plx_mutex_try_lock(plx_mutex* mutex);
void plx_mutex_unlock(plx_mutex* mutex);

#endif  // PLX_MUTEX_H
