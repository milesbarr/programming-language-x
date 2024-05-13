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

#include "symbol_table.h"

#include <assert.h>
#include <string.h>

#include "error.h"
#include "macros.h"
#include "memory_pool.h"
#include "symbol_table_entry.h"

void plx_enter_scope(struct plx_symbol_table* const symbol_table) {
  assert(symbol_table->depth + 1 < PLX_MAX_DEPTH);
  symbol_table->scopes[++symbol_table->depth] = symbol_table->head;
}

void plx_exit_scope(struct plx_symbol_table* const symbol_table) {
  assert(symbol_table->depth > 0);
  symbol_table->head = symbol_table->scopes[symbol_table->depth--];
}

struct plx_symbol_table_entry* plx_declare_symbol(
    struct plx_symbol_table* const symbol_table, const char* const name) {
  // Check that the symbol wasn't already declared in the scope.
  for (struct plx_symbol_table_entry* entry = symbol_table->head;
       entry != symbol_table->scopes[symbol_table->depth];
       entry = entry->prev) {
    if (strcmp(entry->name, name) == 0) return NULL;
  }

  // Allocate a new symbol table entry.
  static thread_local struct plx_memory_pool pool;
  struct plx_symbol_table_entry* const entry =
      plx_memory_pool_alloc(&pool, sizeof(struct plx_symbol_table_entry));
  if (plx_unlikely(entry == NULL)) plx_oom();
  *entry = (struct plx_symbol_table_entry){symbol_table->head, name};
  symbol_table->head = entry;
  return entry;
}

struct plx_symbol_table_entry* plx_lookup_symbol(
    const struct plx_symbol_table* const symbol_table, const char* const name) {
  for (struct plx_symbol_table_entry* entry = symbol_table->head; entry != NULL;
       entry = entry->prev) {
    if (strcmp(entry->name, name) == 0) return entry;
  }
  return NULL;
}
