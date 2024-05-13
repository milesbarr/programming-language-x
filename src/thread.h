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

#ifndef PLX_THREAD_H
#define PLX_THREAD_H

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#define PLX_THREAD_DEF_START_ROUTINE(name) DWORD WINAPI name(LPVOID arg)

typedef HANDLE plx_thread;
typedef LPTHREAD_START_ROUTINE plx_thread_start_routine;
#else
#include <pthread.h>

#define PLX_THREAD_DEF_START_ROUTINE(name) void* name(void* arg)

typedef pthread_t plx_thread;
typedef void* (*plx_thread_start_routine)(void*);
#endif  // _WIN32

void plx_thread_init(plx_thread* thread, plx_thread_start_routine start_routine,
                     void* arg);
void plx_thread_join(plx_thread* thread);

#endif  // PLX_THREAD_H
