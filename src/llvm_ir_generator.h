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

#ifndef PLX_LLVM_IR_GENERATOR_H
#define PLX_LLVM_IR_GENERATOR_H

#include <stdio.h>

#include "ast.h"

// Represents an LLVM unnamed identifier.
// https://llvm.org/docs/LangRef.html#identifiers
typedef unsigned int plx_llvm_unnamed_identifier;

// Generates an LLVM IR module from the abstract syntax tree to the output stream.
void plx_generate_llvm_ir(const struct plx_node* node, FILE* stream);

// Generates LLVM IR for a statement in the abstract syntax tree to the output stream.
void plx_generate_llvm_ir_stmt(const struct plx_node* node, FILE* stream,
                               plx_llvm_unnamed_identifier* locals,
                               plx_llvm_unnamed_identifier loop_enter_label,
                               plx_llvm_unnamed_identifier loop_exit_label);

// Generates LLVM IR for an expression in the abstract syntax tree to the output
// stream, returning a temporary variable that holds the result.
plx_llvm_unnamed_identifier plx_generate_llvm_ir_expr(
    const struct plx_node* node, FILE* stream,
    plx_llvm_unnamed_identifier* locals);

#endif  // PLX_LLVM_IR_GENERATOR_H
