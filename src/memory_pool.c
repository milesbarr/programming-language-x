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

#include "memory_pool.h"

#include <assert.h>
#include <stdlib.h>

#include "macros.h"

void* plx_memory_pool_alloc(struct plx_memory_pool* const pool,
                            const size_t size) {
  assert(size >= sizeof(void*));

  // Remove from the free list.
  if (pool->free_list != NULL) {
    void* const ptr = pool->free_list;
    pool->free_list = *(void**)ptr;
    return ptr;
  }

  // Allocate a new block.
  assert(pool->len <= pool->cap);
  if (plx_unlikely(pool->len == pool->cap)) {
    const size_t cap = pool->cap * 2 + !pool->cap;
    void* const block = malloc(cap * size);
    if (plx_unlikely(block == NULL)) return NULL;
    pool->cap = cap;
    pool->block = block;
  }

  return &((char*)pool->block)[size * pool->len++];
}

void plx_memory_pool_dealloc(struct plx_memory_pool* const pool,
                             void* const ptr) {
  *((void**)ptr) = pool->free_list;
  pool->free_list = ptr;
}
