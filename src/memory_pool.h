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

#ifndef PLX_MEMORY_POOL_H
#define PLX_MEMORY_POOL_H

#include <stddef.h>

// Implements a memory pool.
// https://en.wikipedia.org/wiki/Memory_pool
struct plx_memory_pool {
  size_t cap;
  size_t len;
  void* block;
  void* free_list;
};

#define PLX_MEMORY_POOL_INIT ((struct plx_memory_pool){0, 0, NULL, NULL})

// Allocates memory from the pool.
void* plx_memory_pool_alloc(struct plx_memory_pool* pool, size_t size);

// Deallocates memory from the pool, adding it to the free list.
void plx_memory_pool_dealloc(struct plx_memory_pool* pool, void* ptr);

#endif  // PLX_MEMORY_POOL_H
