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

#ifndef PLX_AST_H
#define PLX_AST_H

#include <stdbool.h>
#include <stddef.h>

#include "source_code_location.h"

enum plx_node_kind {
  // Module
  PLX_NODE_MODULE,

  // Definitions
  PLX_NODE_CONST_DEF,
  PLX_NODE_VAR_DEF,
  PLX_NODE_VAR_DECL,
  PLX_NODE_STRUCT_DEF,
  PLX_NODE_FUNC_DEF,

  // Statements
  PLX_NODE_NOP,
  PLX_NODE_BLOCK,
  PLX_NODE_IF_THEN_ELSE,
  PLX_NODE_LOOP,
  PLX_NODE_WHILE_LOOP,
  PLX_NODE_CONTINUE,
  PLX_NODE_BREAK,
  PLX_NODE_RETURN,
  PLX_NODE_ASSIGN,
  PLX_NODE_ADD_ASSIGN,
  PLX_NODE_SUB_ASSIGN,
  PLX_NODE_MUL_ASSIGN,
  PLX_NODE_DIV_ASSIGN,
  PLX_NODE_REM_ASSIGN,
  PLX_NODE_LSHIFT_ASSIGN,
  PLX_NODE_RSHIFT_ASSIGN,

  // Expressions
  PLX_NODE_AND,
  PLX_NODE_OR,
  PLX_NODE_XOR,
  PLX_NODE_EQ,
  PLX_NODE_NEQ,
  PLX_NODE_LTE,
  PLX_NODE_LT,
  PLX_NODE_GTE,
  PLX_NODE_GT,
  PLX_NODE_ADD,
  PLX_NODE_SUB,
  PLX_NODE_MUL,
  PLX_NODE_DIV,
  PLX_NODE_REM,
  PLX_NODE_LSHIFT,
  PLX_NODE_RSHIFT,
  PLX_NODE_NOT,
  PLX_NODE_NEG,
  PLX_NODE_REF,
  PLX_NODE_DEREF,
  PLX_NODE_CALL,
  PLX_NODE_INDEX,
  PLX_NODE_SLICE,
  PLX_NODE_FIELD,
  PLX_NODE_IDENTIFIER,
  PLX_NODE_STRUCT,
  PLX_NODE_S8,
  PLX_NODE_S16,
  PLX_NODE_S32,
  PLX_NODE_S64,
  PLX_NODE_U8,
  PLX_NODE_U16,
  PLX_NODE_U32,
  PLX_NODE_U64,
  PLX_NODE_F16,
  PLX_NODE_F32,
  PLX_NODE_F64,
  PLX_NODE_BOOL,
  PLX_NODE_STRING,

  // Type
  PLX_NODE_VOID_TYPE,
  PLX_NODE_S8_TYPE,
  PLX_NODE_S16_TYPE,
  PLX_NODE_S32_TYPE,
  PLX_NODE_S64_TYPE,
  PLX_NODE_U8_TYPE,
  PLX_NODE_U16_TYPE,
  PLX_NODE_U32_TYPE,
  PLX_NODE_U64_TYPE,
  PLX_NODE_F16_TYPE,
  PLX_NODE_F32_TYPE,
  PLX_NODE_F64_TYPE,
  PLX_NODE_BOOL_TYPE,
  PLX_NODE_STRING_TYPE,
  PLX_NODE_FUNC_TYPE,
  PLX_NODE_REF_TYPE,
  PLX_NODE_ARRAY_TYPE,
  PLX_NODE_SLICE_TYPE,

  // Other
  PLX_NODE_OTHER,
};

struct plx_symbol_table_entry;

// Node in the abstract syntax tree.
struct plx_node {
  enum plx_node_kind kind;
  union {
    struct {
      char* name;
      struct plx_symbol_table_entry* entry;
    };
    long long sint;
    unsigned long long uint;
    double f;
    bool b;
    struct {
      size_t len;
      char* str;
    };
  };
  struct plx_node* children;
  struct plx_node* next;
  const struct plx_node* type;
  struct plx_source_code_location loc;
};

// Creates and returns a new abstract syntax tree node.
struct plx_node* plx_new_node(enum plx_node_kind kind,
                              const struct plx_source_code_location* loc);

// Returns a copy of an abstract syntax tree node.
struct plx_node* plx_copy_node(const struct plx_node* node);

// Returns the number of children of an abstract syntax tree node.
size_t plx_count_children(const struct plx_node* node);

// Extracts the children of an abtract syntax tree node into the argument list.
void plx_extract_children(const struct plx_node* node, ...);

// Returns whether a node is a constant.
bool plx_is_constant(const struct plx_node* node);

#endif  // PLX_AST_H
