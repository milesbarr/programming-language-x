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

#ifndef PLX_NAME_RESOLVER_H
#define PLX_NAME_RESOLVER_H

#include <stdbool.h>

#include "ast.h"
#include "symbol_table.h"

// Resolves names in the abstract syntax tree, checking the names for validity,
// and linking the names with the corresponding symbol table entries.
// https://en.wikipedia.org/wiki/Name_resolution_(programming_languages)
bool plx_resolve_names(struct plx_node* node,
                       struct plx_symbol_table* symbol_table);

#endif  // PLX_NAME_RESOLVER_H
