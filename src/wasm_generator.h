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

#ifndef PLX_WASM_GENERATOR_H
#define PLX_WASM_GENERATOR_H

#include <stdbool.h>
#include <stdio.h>

#include "ast.h"

// Generates a WebAssembly module from the abstract syntax tree to the output
// stream.
bool plx_generate_wasm(const struct plx_node* module, FILE* stream);

#endif  // PLX_WASM_GENERATOR_H
