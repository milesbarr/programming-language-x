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

#ifndef PLX_SYMBOL_TABLE_H
#define PLX_SYMBOL_TABLE_H

#include <stddef.h>

enum { PLX_MAX_DEPTH = 256 };

struct plx_symbol_table_entry;

// https://en.wikipedia.org/wiki/Symbol_table
struct plx_symbol_table {
  size_t depth;
  struct plx_symbol_table_entry* scopes[PLX_MAX_DEPTH];
  struct plx_symbol_table_entry* head;
};

#define PLX_SYMBOL_TABLE_INIT ((struct plx_symbol_table){0, {0}, NULL})
void plx_enter_scope(struct plx_symbol_table* symbol_table);
void plx_exit_scope(struct plx_symbol_table* symbol_table);
struct plx_symbol_table_entry* plx_declare_symbol(
    struct plx_symbol_table* symbol_table, const char* name);
struct plx_symbol_table_entry* plx_lookup_symbol(
    const struct plx_symbol_table* symbol_table, const char* name);

#endif  // PLX_SYMBOL_TABLE_H
