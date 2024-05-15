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

#ifndef PLX_SYMBOL_TABLE_ENTRY_H
#define PLX_SYMBOL_TABLE_ENTRY_H

#include "ast.h"
#include "source_code_location.h"

// Scope of a symbol.
enum plx_symbol_scope {
  PLX_SYMBOL_SCOPE_LOCAL,
  PLX_SYMBOL_SCOPE_GLOBAL,
};

// Mutability of a symbol.
enum plx_symbol_mutability {
  PLX_SYMBOL_MUTABILITY_MUTABLE,
  PLX_SYMBOL_MUTABILITY_CONST,
};

// Entry that stores information related to a symbol.
struct plx_symbol_table_entry {
  // Previous entry.
  struct plx_symbol_table_entry* prev;

  // Name of the symbol.
  const char* name;

  // Location where the symbol was declared.
  const struct plx_source_code_location* decl;

  // Scope of the symbol.
  enum plx_symbol_scope scope;

  // Mutability of the symbol.
  enum plx_symbol_mutability mutability;

  // Type of the symbol.
  const struct plx_node* type;

  // Value of the symbol.
  const struct plx_node* value;

  // LLVM local variable.
  unsigned int llvm_local_var;
};

#endif  // PLX_SYMBOL_TABLE_ENTRY_H
