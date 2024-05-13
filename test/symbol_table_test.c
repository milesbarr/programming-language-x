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

// Tests declaring a symbol and immediately looking it up.
static void plx_test_symbol_table_declare_and_lookup(void) {
  struct plx_symbol_table symbol_table = PLX_SYMBOL_TABLE_INIT;
  const struct plx_symbol_table_entry* const entry =
      plx_declare_symbol(&symbol_table, "foo");
  assert(entry != NULL);
  assert(plx_lookup_symbol(&symbol_table, "foo") == entry);
}

// Tests declarating a symbol that has already been declared.
static void plx_test_symbol_table_symbol_already_declared(void) {
  struct plx_symbol_table symbol_table = PLX_SYMBOL_TABLE_INIT;
  assert(plx_declare_symbol(&symbol_table, "foo") != NULL);
  assert(plx_declare_symbol(&symbol_table, "foo") == NULL);
}

// Tests looking up a symbol that has fallen out of scope.
static void plx_test_symbol_table_symbol_falls_out_of_scope(void) {
  struct plx_symbol_table symbol_table = PLX_SYMBOL_TABLE_INIT;
  plx_enter_scope(&symbol_table);
  const struct plx_symbol_table_entry* const entry =
      plx_declare_symbol(&symbol_table, "foo");
  assert(entry != NULL);
  plx_exit_scope(&symbol_table);
  assert(plx_lookup_symbol(&symbol_table, "foo") == NULL);
}

// Tests declaring a symbol in multiple scopes.
static void plx_test_symbol_table_symbol_declared_in_multiple_scopes(void) {
  struct plx_symbol_table symbol_table = PLX_SYMBOL_TABLE_INIT;

  plx_enter_scope(&symbol_table);
  const struct plx_symbol_table_entry* const entry_a =
      plx_declare_symbol(&symbol_table, "foo");
  assert(entry_a != NULL);
  plx_exit_scope(&symbol_table);

  plx_enter_scope(&symbol_table);
  const struct plx_symbol_table_entry* const entry_b =
      plx_declare_symbol(&symbol_table, "foo");
  assert(entry_b != NULL);
  assert(entry_b != entry_a);
  plx_exit_scope(&symbol_table);
}

// Tests variable shadowing behavior.
// https://en.wikipedia.org/wiki/Variable_shadowing
static void plx_test_symbol_table_variable_shadowing(void) {
  struct plx_symbol_table symbol_table = PLX_SYMBOL_TABLE_INIT;
  const struct plx_symbol_table_entry* const entry_a =
      plx_declare_symbol(&symbol_table, "foo");
  assert(entry_a != NULL);

  plx_enter_scope(&symbol_table);
  const struct plx_symbol_table_entry* const entry_b =
      plx_declare_symbol(&symbol_table, "foo");
  assert(entry_b != NULL);
  assert(entry_b != entry_a);
  assert(plx_lookup_symbol(&symbol_table, "foo") == entry_b);
  plx_exit_scope(&symbol_table);

  assert(plx_lookup_symbol(&symbol_table, "foo") == entry_a);
}

void plx_test_symbol_table(void) {
  plx_test_symbol_table_declare_and_lookup();
  plx_test_symbol_table_symbol_already_declared();
  plx_test_symbol_table_symbol_falls_out_of_scope();
  plx_test_symbol_table_symbol_declared_in_multiple_scopes();
  plx_test_symbol_table_variable_shadowing();
}
