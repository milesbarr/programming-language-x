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

#ifndef PLX_PARSER_H
#define PLX_PARSER_H

#include "ast.h"
#include "tokenizer.h"

// Module
struct plx_node* plx_parse_module(struct plx_tokenizer* tokenizer);

// Definitions
struct plx_node* plx_parse_const_def(struct plx_tokenizer* tokenizer);
struct plx_node* plx_parse_var_def_or_decl(struct plx_tokenizer* tokenizer);
struct plx_node* plx_parse_struct_def(struct plx_tokenizer* tokenizer);
struct plx_node* plx_parse_func_def(struct plx_tokenizer* tokenizer);
struct plx_node* plx_parse_params(struct plx_tokenizer* tokenizer);

// Statements
struct plx_node* plx_parse_stmt(struct plx_tokenizer* tokenizer);
struct plx_node* plx_parse_block(struct plx_tokenizer* tokenizer);
struct plx_node* plx_parse_loop(struct plx_tokenizer* tokenizer);
struct plx_node* plx_parse_while_loop(struct plx_tokenizer* tokenizer);
struct plx_node* plx_parse_return(struct plx_tokenizer* tokenizer);
struct plx_node* plx_parse_assign(struct plx_tokenizer* tokenizer);

// Expressions
struct plx_node* plx_parse_expr(struct plx_tokenizer* tokenizer);
struct plx_node* plx_parse_if_then_else(struct plx_tokenizer* tokenizer);
struct plx_node* plx_parse_logical_expr(struct plx_tokenizer* tokenizer);
struct plx_node* plx_parse_rel_expr(struct plx_tokenizer* tokenizer);
struct plx_node* plx_parse_arithmetic_expr(struct plx_tokenizer* tokenizer);
struct plx_node* plx_parse_unary_expr(struct plx_tokenizer* tokenizer);
struct plx_node* plx_parse_postfix_expr(struct plx_tokenizer* tokenizer);
struct plx_node* plx_parse_primary_expr(struct plx_tokenizer* tokenizer);
struct plx_node* plx_parse_func(struct plx_tokenizer* tokenizer);
struct plx_node* plx_parse_identifier(struct plx_tokenizer* tokenizer);
struct plx_node* plx_parse_identifier_or_struct(
    struct plx_tokenizer* tokenizer);
struct plx_node* plx_parse_int_lit(struct plx_tokenizer* tokenizer);
struct plx_node* plx_parse_string_lit(struct plx_tokenizer* tokenizer);

// Types
struct plx_node* plx_parse_type(struct plx_tokenizer* tokenizer);
struct plx_node* plx_parse_func_type(struct plx_tokenizer* tokenizer);
struct plx_node* plx_parse_ref_type(struct plx_tokenizer* tokenizer);
struct plx_node* plx_parse_array_or_slice_type(struct plx_tokenizer* tokenizer);

#endif  // PLX_PARSER_H
