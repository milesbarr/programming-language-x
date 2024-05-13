// Copyright 2023 Miles Barr
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

#include "parser.h"

#include <assert.h>
#include <stdio.h>

#include "macros.h"
#include "source_code_location.h"

struct plx_node* plx_parse_module(struct plx_tokenizer* const tokenizer) {
  struct plx_node* const module =
      plx_new_node(PLX_NODE_MODULE, &tokenizer->loc);
  struct plx_node** next = &module->children;
  while (tokenizer->token != PLX_TOKEN_EOF) {
    struct plx_node* def;
    switch (tokenizer->token) {
      case PLX_TOKEN_CONST:
        def = plx_parse_const_def(tokenizer);
        break;
      case PLX_TOKEN_VAR:
        def = plx_parse_var_def_or_decl(tokenizer);
        break;
      case PLX_TOKEN_STRUCT:
        def = plx_parse_struct_def(tokenizer);
        break;
      case PLX_TOKEN_FUNC:
        def = plx_parse_func_def(tokenizer);
        break;
      default:
        return NULL;
    }
    if (plx_unlikely(def == NULL)) return NULL;
    *next = def;
    next = &def->next;
  }
  return module;
}

struct plx_node* plx_parse_const_def(struct plx_tokenizer* const tokenizer) {
  assert(tokenizer->token == PLX_TOKEN_CONST);
  const struct plx_source_code_location loc = tokenizer->loc;
  plx_next_token(tokenizer);

  // Parse the name.
  struct plx_node* const name = plx_parse_identifier(tokenizer);
  if (plx_unlikely(name == NULL)) return NULL;

  if (!plx_accept_token(tokenizer, PLX_TOKEN_ASSIGN)) return NULL;

  // Parse the value.
  struct plx_node* const value = plx_parse_expr(tokenizer);
  if (plx_unlikely(value == NULL)) return NULL;

  if (!plx_accept_token(tokenizer, PLX_TOKEN_SEMICOLON)) return NULL;

  // Create the node.
  struct plx_node* const const_def = plx_new_node(PLX_NODE_CONST_DEF, &loc);
  const_def->children = name;
  name->next = value;
  return const_def;
}

struct plx_node* plx_parse_var_def_or_decl(
    struct plx_tokenizer* const tokenizer) {
  assert(tokenizer->token == PLX_TOKEN_VAR);
  const struct plx_source_code_location loc = tokenizer->loc;
  plx_next_token(tokenizer);

  // Parse the name.
  struct plx_node* const name = plx_parse_identifier(tokenizer);
  if (plx_unlikely(name == NULL)) return NULL;

  switch (tokenizer->token) {
    case PLX_TOKEN_ASSIGN: {
      plx_next_token(tokenizer);

      // Parse the value.
      struct plx_node* const value = plx_parse_expr(tokenizer);
      if (plx_unlikely(value == NULL)) return NULL;

      if (!plx_accept_token(tokenizer, PLX_TOKEN_SEMICOLON)) return NULL;

      // Create the node.
      struct plx_node* const var_def = plx_new_node(PLX_NODE_VAR_DEF, &loc);
      var_def->children = name;
      name->next = value;
      return var_def;
    }
    case PLX_TOKEN_COLON: {
      plx_next_token(tokenizer);

      // Parse the type.
      struct plx_node* const type = plx_parse_type(tokenizer);
      if (plx_unlikely(type == NULL)) return NULL;

      if (!plx_accept_token(tokenizer, PLX_TOKEN_SEMICOLON)) return NULL;

      // Create the node.
      struct plx_node* const var_decl = plx_new_node(PLX_NODE_VAR_DECL, &loc);
      var_decl->children = name;
      name->next = type;
      return var_decl;
    }
    default:
      return NULL;
  }
}

struct plx_node* plx_parse_struct_def(struct plx_tokenizer* const tokenizer) {
  assert(tokenizer->token == PLX_TOKEN_STRUCT);
  const struct plx_source_code_location loc = tokenizer->loc;
  plx_next_token(tokenizer);

  // Parse the name.
  struct plx_node* const name = plx_parse_identifier(tokenizer);
  if (plx_unlikely(name == NULL)) return NULL;

  // Parse the members.
  if (!plx_accept_token(tokenizer, PLX_TOKEN_OPEN_CURLY_BRACE)) return NULL;
  struct plx_node* const members = plx_new_node(PLX_NODE_OTHER, /*loc=*/NULL);
  struct plx_node** next = &members->children;
  while (!plx_accept_token(tokenizer, PLX_TOKEN_CLOSE_CURLY_BRACE)) {
    // Parse the member name.
    struct plx_node* const member_name = plx_parse_identifier(tokenizer);
    if (plx_unlikely(member_name == NULL)) return NULL;

    if (!plx_accept_token(tokenizer, PLX_TOKEN_COLON)) return NULL;

    // Parse the member type.
    struct plx_node* const member_type = plx_parse_type(tokenizer);
    if (plx_unlikely(member_type == NULL)) return NULL;

    if (!plx_accept_token(tokenizer, PLX_TOKEN_SEMICOLON)) return NULL;

    struct plx_node* const member = plx_new_node(PLX_NODE_OTHER, /*loc=*/NULL);
    member->children = member_name;
    member_name->next = member_type;
    *next = member;
    next = &member->next;
  }

  // Create the node.
  struct plx_node* const struct_def = plx_new_node(PLX_NODE_STRUCT_DEF, &loc);
  struct_def->children = name;
  name->next = members;
  return struct_def;
}

struct plx_node* plx_parse_func_def(struct plx_tokenizer* const tokenizer) {
  assert(tokenizer->token == PLX_TOKEN_FUNC);
  const struct plx_source_code_location loc = tokenizer->loc;
  plx_next_token(tokenizer);

  // Parse the name.
  struct plx_node* const name = plx_parse_identifier(tokenizer);
  if (plx_unlikely(name == NULL)) return NULL;

  // Parse the parameters.
  struct plx_node* const params = plx_parse_params(tokenizer);
  if (plx_unlikely(params == NULL)) return NULL;

  // Parse the return type.
  struct plx_node* return_type;
  if (plx_accept_token(tokenizer, PLX_TOKEN_ARROW)) {
    return_type = plx_parse_type(tokenizer);
    if (plx_unlikely(return_type == NULL)) return NULL;
  } else {
    return_type = plx_new_node(PLX_NODE_VOID_TYPE, /*loc=*/NULL);
  }

  // Parse the body.
  struct plx_node* const body = plx_parse_block(tokenizer);
  if (plx_unlikely(body == NULL)) return NULL;

  // Create the node.
  struct plx_node* const func_def = plx_new_node(PLX_NODE_FUNC_DEF, &loc);
  func_def->children = name;
  name->next = params;
  params->next = return_type;
  return_type->next = body;
  return func_def;
}

struct plx_node* plx_parse_params(struct plx_tokenizer* const tokenizer) {
  if (!plx_accept_token(tokenizer, PLX_TOKEN_OPEN_PAREN)) return NULL;

  struct plx_node* const params = plx_new_node(PLX_NODE_OTHER, /*loc=*/NULL);
  struct plx_node** next = &params->children;
  if (!plx_accept_token(tokenizer, PLX_TOKEN_CLOSE_PAREN)) {
    while (1) {
      // Parse the parameter name.
      struct plx_node* const param_name = plx_parse_identifier(tokenizer);
      if (plx_unlikely(param_name == NULL)) return NULL;

      if (!plx_accept_token(tokenizer, PLX_TOKEN_COLON)) return NULL;

      // Parse the parameter type.
      struct plx_node* const param_type = plx_parse_type(tokenizer);
      if (plx_unlikely(param_type == NULL)) return NULL;

      // Create the parameter node.
      struct plx_node* const param = plx_new_node(PLX_NODE_OTHER, /*loc=*/NULL);
      param->children = param_name;
      param_name->next = param_type;
      *next = param;
      next = &param->next;

      if (plx_accept_token(tokenizer, PLX_TOKEN_CLOSE_PAREN)) break;
      if (!plx_accept_token(tokenizer, PLX_TOKEN_COMMA)) return NULL;
    }
  }

  return params;
}

struct plx_node* plx_parse_stmt(struct plx_tokenizer* const tokenizer) {
  const struct plx_source_code_location loc = tokenizer->loc;
  switch (tokenizer->token) {
    case PLX_TOKEN_OPEN_CURLY_BRACE:
      return plx_parse_block(tokenizer);
    case PLX_TOKEN_CONST:
      return plx_parse_const_def(tokenizer);
    case PLX_TOKEN_VAR:
      return plx_parse_var_def_or_decl(tokenizer);
    case PLX_TOKEN_IF:
      return plx_parse_if_then_else(tokenizer);
    case PLX_TOKEN_LOOP:
      return plx_parse_loop(tokenizer);
    case PLX_TOKEN_WHILE:
      return plx_parse_while_loop(tokenizer);
    case PLX_TOKEN_CONTINUE:
      plx_next_token(tokenizer);
      if (!plx_accept_token(tokenizer, PLX_TOKEN_SEMICOLON)) return NULL;
      return plx_new_node(PLX_NODE_CONTINUE, &loc);
    case PLX_TOKEN_BREAK:
      plx_next_token(tokenizer);
      if (!plx_accept_token(tokenizer, PLX_TOKEN_SEMICOLON)) return NULL;
      return plx_new_node(PLX_NODE_BREAK, &loc);
    case PLX_TOKEN_RETURN:
      return plx_parse_return(tokenizer);
    default:
      return plx_parse_assign(tokenizer);
  }
}

struct plx_node* plx_parse_block(struct plx_tokenizer* const tokenizer) {
  const struct plx_source_code_location loc = tokenizer->loc;
  if (!plx_accept_token(tokenizer, PLX_TOKEN_OPEN_CURLY_BRACE)) return NULL;
  struct plx_node* const block = plx_new_node(PLX_NODE_BLOCK, &loc);
  struct plx_node** next = &block->children;
  while (!plx_accept_token(tokenizer, PLX_TOKEN_CLOSE_CURLY_BRACE)) {
    // Parse a statement.
    struct plx_node* const stmt = plx_parse_stmt(tokenizer);
    if (plx_unlikely(stmt == NULL)) return NULL;
    *next = stmt;
    next = &stmt->next;
  }
  return block;
}

struct plx_node* plx_parse_if_then_else(struct plx_tokenizer* const tokenizer) {
  assert(tokenizer->token == PLX_TOKEN_IF);
  const struct plx_source_code_location loc = tokenizer->loc;
  plx_next_token(tokenizer);

  // Parse the condition.
  struct plx_node* const cond = plx_parse_rel_expr(tokenizer);
  if (plx_unlikely(cond == NULL)) return NULL;

  // Parse the "then" block.
  struct plx_node* const then = plx_parse_block(tokenizer);
  if (plx_unlikely(then == NULL)) return NULL;

  // Parse the "else" block.
  struct plx_node* els;
  if (plx_accept_token(tokenizer, PLX_TOKEN_ELSE)) {
    els = tokenizer->token == PLX_TOKEN_IF ? plx_parse_if_then_else(tokenizer)
                                           : plx_parse_block(tokenizer);
    if (plx_unlikely(els == NULL)) return NULL;
  } else {
    els = plx_new_node(PLX_NODE_BLOCK, /*loc=*/NULL);
  }

  // Create the node.
  struct plx_node* const if_then_else =
      plx_new_node(PLX_NODE_IF_THEN_ELSE, &loc);
  if_then_else->children = cond;
  cond->next = then;
  then->next = els;
  return if_then_else;
}

struct plx_node* plx_parse_loop(struct plx_tokenizer* const tokenizer) {
  assert(tokenizer->token == PLX_TOKEN_LOOP);
  const struct plx_source_code_location loc = tokenizer->loc;
  plx_next_token(tokenizer);

  // Parse the body.
  struct plx_node* const body = plx_parse_block(tokenizer);
  if (plx_unlikely(body == NULL)) return NULL;

  // Create the node.
  struct plx_node* const loop = plx_new_node(PLX_NODE_LOOP, &loc);
  loop->children = body;
  return loop;
}

struct plx_node* plx_parse_while_loop(struct plx_tokenizer* const tokenizer) {
  assert(tokenizer->token == PLX_TOKEN_WHILE);
  const struct plx_source_code_location loc = tokenizer->loc;
  plx_next_token(tokenizer);

  // Parse the condition.
  struct plx_node* const cond = plx_parse_rel_expr(tokenizer);
  if (plx_unlikely(cond == NULL)) return NULL;

  // Parse the body.
  struct plx_node* const body = plx_parse_block(tokenizer);
  if (plx_unlikely(body == NULL)) return NULL;

  // Create the node.
  struct plx_node* const while_loop = plx_new_node(PLX_NODE_WHILE_LOOP, &loc);
  while_loop->children = cond;
  cond->next = body;
  return while_loop;
}

struct plx_node* plx_parse_return(struct plx_tokenizer* const tokenizer) {
  assert(tokenizer->token == PLX_TOKEN_RETURN);
  const struct plx_source_code_location loc = tokenizer->loc;
  plx_next_token(tokenizer);

  // Parse the return value.
  struct plx_node* value = NULL;
  if (!plx_accept_token(tokenizer, PLX_TOKEN_SEMICOLON)) {
    value = plx_parse_expr(tokenizer);
    if (plx_unlikely(value == NULL)) return NULL;
    if (!plx_accept_token(tokenizer, PLX_TOKEN_SEMICOLON)) return NULL;
  }

  // Create the node.
  struct plx_node* const rreturn = plx_new_node(PLX_NODE_RETURN, &loc);
  rreturn->children = value;
  return rreturn;
}

struct plx_node* plx_parse_assign(struct plx_tokenizer* const tokenizer) {
  const struct plx_source_code_location loc = tokenizer->loc;

  // Parse the target.
  struct plx_node* const target = plx_parse_unary_expr(tokenizer);
  if (plx_unlikely(target == NULL)) return NULL;

  enum plx_node_kind kind;
  switch (tokenizer->token) {
    case PLX_TOKEN_ASSIGN:
      kind = PLX_NODE_ASSIGN;
      break;
    case PLX_TOKEN_ADD_ASSIGN:
      kind = PLX_NODE_ADD_ASSIGN;
      break;
    case PLX_TOKEN_SUB_ASSIGN:
      kind = PLX_NODE_SUB_ASSIGN;
      break;
    case PLX_TOKEN_MUL_ASSIGN:
      kind = PLX_NODE_MUL_ASSIGN;
      break;
    case PLX_TOKEN_DIV_ASSIGN:
      kind = PLX_NODE_DIV_ASSIGN;
      break;
    case PLX_TOKEN_REM_ASSIGN:
      kind = PLX_NODE_REM_ASSIGN;
      break;
    case PLX_TOKEN_LSHIFT_ASSIGN:
      kind = PLX_NODE_LSHIFT_ASSIGN;
      break;
    case PLX_TOKEN_RSHIFT_ASSIGN:
      kind = PLX_NODE_RSHIFT_ASSIGN;
      break;
    default:
      if (!plx_accept_token(tokenizer, PLX_TOKEN_SEMICOLON)) return NULL;
      return target;
  }
  plx_next_token(tokenizer);

  // Parse the value.
  struct plx_node* const value = plx_parse_expr(tokenizer);
  if (plx_unlikely(value == NULL)) return NULL;

  if (!plx_accept_token(tokenizer, PLX_TOKEN_SEMICOLON)) return NULL;

  // Create the node.
  struct plx_node* const assign = plx_new_node(kind, &loc);
  assign->children = target;
  target->next = value;
  return assign;
}

struct plx_node* plx_parse_expr(struct plx_tokenizer* const tokenizer) {
  switch (tokenizer->token) {
    default:
      return plx_parse_logical_expr(tokenizer);
  }
}

struct plx_node* plx_parse_logical_expr(struct plx_tokenizer* const tokenizer) {
  const struct plx_source_code_location loc = tokenizer->loc;

  // Parse the left operand.
  struct plx_node* left = plx_parse_rel_expr(tokenizer);
  if (plx_unlikely(left == NULL)) return NULL;

  const enum plx_token op = tokenizer->token;
  enum plx_node_kind kind;
  switch (op) {
    case PLX_TOKEN_AND:
      kind = PLX_NODE_AND;
      break;
    case PLX_TOKEN_OR:
      kind = PLX_NODE_OR;
      break;
    case PLX_TOKEN_XOR:
      kind = PLX_NODE_XOR;
      break;
    default:
      return left;
  }

  while (plx_accept_token(tokenizer, op)) {
    // Parse the right operand.
    struct plx_node* const right = plx_parse_rel_expr(tokenizer);
    if (plx_unlikely(right == NULL)) return NULL;

    // Create the node.
    struct plx_node* const expr = plx_new_node(kind, &loc);
    expr->children = left;
    left->next = right;
    left = expr;
  }

  return left;
}

struct plx_node* plx_parse_rel_expr(struct plx_tokenizer* const tokenizer) {
  const struct plx_source_code_location loc = tokenizer->loc;

  // Parse the left operand.
  struct plx_node* const left = plx_parse_arithmetic_expr(tokenizer);
  if (plx_unlikely(left == NULL)) return NULL;

  enum plx_node_kind kind;
  switch (tokenizer->token) {
    case PLX_TOKEN_EQ:
      kind = PLX_NODE_EQ;
      break;
    case PLX_TOKEN_NEQ:
      kind = PLX_NODE_NEQ;
      break;
    case PLX_TOKEN_LTE:
      kind = PLX_NODE_LTE;
      break;
    case PLX_TOKEN_LT:
      kind = PLX_NODE_LT;
      break;
    case PLX_TOKEN_GTE:
      kind = PLX_NODE_GTE;
      break;
    case PLX_TOKEN_GT:
      kind = PLX_NODE_GT;
      break;
    default:
      return left;
  }
  plx_next_token(tokenizer);

  // Parse the right operand.
  struct plx_node* const right = plx_parse_arithmetic_expr(tokenizer);
  if (plx_unlikely(right == NULL)) return NULL;

  // Create the node.
  struct plx_node* const expr = plx_new_node(kind, &loc);
  expr->children = left;
  left->next = right;
  return expr;
}

struct plx_node* plx_parse_arithmetic_expr(
    struct plx_tokenizer* const tokenizer) {
  const struct plx_source_code_location loc = tokenizer->loc;

  // Parse the left operand.
  struct plx_node* left = plx_parse_unary_expr(tokenizer);
  if (plx_unlikely(left == NULL)) return NULL;

  const enum plx_token op = tokenizer->token;
  enum plx_node_kind kind;
  switch (op) {
    case PLX_TOKEN_ADD:
      kind = PLX_NODE_ADD;
      break;
    case PLX_TOKEN_SUB:
      kind = PLX_NODE_SUB;
      break;
    case PLX_TOKEN_MUL:
      kind = PLX_NODE_MUL;
      break;
    case PLX_TOKEN_DIV:
      kind = PLX_NODE_DIV;
      break;
    case PLX_TOKEN_REM:
      kind = PLX_NODE_REM;
      break;
    case PLX_TOKEN_LSHIFT:
      kind = PLX_NODE_LSHIFT;
      break;
    case PLX_TOKEN_RSHIFT:
      kind = PLX_NODE_RSHIFT;
      break;
    default:
      return left;
  }

  while (plx_accept_token(tokenizer, op)) {
    // Parse the right operand.
    struct plx_node* const right = plx_parse_unary_expr(tokenizer);
    if (plx_unlikely(right == NULL)) return NULL;

    // Create the node.
    struct plx_node* const expr = plx_new_node(kind, &loc);
    expr->children = left;
    left->next = right;
    left = expr;
  }

  return left;
}

struct plx_node* plx_parse_unary_expr(struct plx_tokenizer* const tokenizer) {
  enum plx_node_kind kind;
  switch (tokenizer->token) {
    case PLX_TOKEN_NOT:
      kind = PLX_NODE_NOT;
      break;
    case PLX_TOKEN_SUB:
      kind = PLX_NODE_NEG;
      break;
    case PLX_TOKEN_MUL:
      kind = PLX_NODE_DEREF;
      break;
    case PLX_TOKEN_REF:
      kind = PLX_NODE_REF;
      break;
    default:
      return plx_parse_postfix_expr(tokenizer);
  }
  const struct plx_source_code_location loc = tokenizer->loc;
  plx_next_token(tokenizer);

  // Parse the operand.
  struct plx_node* const operand = plx_parse_unary_expr(tokenizer);
  if (plx_unlikely(operand == NULL)) return NULL;

  // Create the node.
  struct plx_node* const expr = plx_new_node(kind, &loc);
  expr->children = operand;
  return expr;
}

struct plx_node* plx_parse_postfix_expr(struct plx_tokenizer* const tokenizer) {
  const struct plx_source_code_location loc = tokenizer->loc;
  struct plx_node* const expr = plx_parse_primary_expr(tokenizer);
  if (plx_unlikely(expr == NULL)) return NULL;

  while (1) {
    switch (tokenizer->token) {
      case PLX_TOKEN_OPEN_PAREN: {
        plx_next_token(tokenizer);

        // Parse the arguments.
        struct plx_node* const args =
            plx_new_node(PLX_NODE_OTHER, /*loc=*/NULL);
        struct plx_node** next = &args->children;
        if (!plx_accept_token(tokenizer, PLX_TOKEN_CLOSE_PAREN)) {
          while (1) {
            struct plx_node* const arg = plx_parse_expr(tokenizer);
            if (plx_unlikely(arg == NULL)) return NULL;
            *next = arg;
            next = &arg->next;
            if (plx_accept_token(tokenizer, PLX_TOKEN_CLOSE_PAREN)) break;
            if (!plx_accept_token(tokenizer, PLX_TOKEN_COMMA)) return NULL;
          }
        }

        // Create the node.
        struct plx_node* const call = plx_new_node(PLX_NODE_CALL, &loc);
        call->children = expr;
        expr->next = args;
        return call;
      }
      case PLX_TOKEN_OPEN_SQUARE_BRACKET: {
        plx_next_token(tokenizer);

        // Parse the start.
        struct plx_node* const start = plx_parse_expr(tokenizer);
        if (plx_unlikely(start == NULL)) return NULL;

        if (plx_accept_token(tokenizer, PLX_TOKEN_COLON)) {
          // Parse the end.
          struct plx_node* const end = plx_parse_expr(tokenizer);
          if (plx_unlikely(end == NULL)) return NULL;

          // Create the node.
          struct plx_node* const slice = plx_new_node(PLX_NODE_SLICE, &loc);
          slice->children = expr;
          expr->next = start;
          start->next = end;
          return slice;
        }

        // Create the node.
        struct plx_node* const index = plx_new_node(PLX_NODE_INDEX, &loc);
        index->children = expr;
        expr->next = start;
        return index;
      }
      case PLX_TOKEN_PERIOD: {
        plx_next_token(tokenizer);

        // Parse the name.
        struct plx_node* const name = plx_parse_identifier(tokenizer);
        if (plx_unlikely(name == NULL)) return NULL;

        // Create the node.
        struct plx_node* const field = plx_new_node(PLX_NODE_FIELD, &loc);
        field->children = expr;
        expr->next = name;
        return field;
      }
      default:
        return expr;
    }
  }
}

struct plx_node* plx_parse_primary_expr(struct plx_tokenizer* const tokenizer) {
  switch (tokenizer->token) {
    case PLX_TOKEN_IDENTIFIER:
      return plx_parse_identifier_or_struct(tokenizer);
    case PLX_TOKEN_INT:
      return plx_parse_int_lit(tokenizer);
    case PLX_TOKEN_TRUE: {
      struct plx_node* const bbool =
          plx_new_node(PLX_NODE_BOOL, &tokenizer->loc);
      plx_next_token(tokenizer);
      bbool->b = true;
      return bbool;
    }
    case PLX_TOKEN_FALSE: {
      struct plx_node* const bbool =
          plx_new_node(PLX_NODE_BOOL, &tokenizer->loc);
      plx_next_token(tokenizer);
      bbool->b = false;
      return bbool;
    }
    case PLX_TOKEN_STRING:
      return plx_parse_string_lit(tokenizer);
    case PLX_TOKEN_OPEN_PAREN:
      return plx_parse_expr(tokenizer);
    default:
      return NULL;
  }
}

struct plx_node* plx_parse_identifier_or_struct(
    struct plx_tokenizer* const tokenizer) {
  const struct plx_source_code_location loc = tokenizer->loc;

  // Parse the identifier.
  struct plx_node* const identifier = plx_parse_identifier(tokenizer);
  if (plx_unlikely(identifier == NULL)) return NULL;

  // Parse the struct.
  if (!plx_accept_token(tokenizer, PLX_TOKEN_OPEN_CURLY_BRACE)) {
    return identifier;
  }

  struct plx_node* const members = plx_new_node(PLX_NODE_OTHER, /*loc=*/NULL);
  struct plx_node** next = &members->children;
  while (!plx_accept_token(tokenizer, PLX_TOKEN_CLOSE_CURLY_BRACE)) {
    // Parse the member name.
    struct plx_node* const member_name = plx_parse_identifier(tokenizer);
    if (plx_unlikely(member_name == NULL)) return NULL;

    if (!plx_accept_token(tokenizer, PLX_TOKEN_COLON)) return NULL;

    // Parse the member value.
    struct plx_node* const member_value = plx_parse_expr(tokenizer);
    if (plx_unlikely(member_value == NULL)) return NULL;

    if (!plx_accept_token(tokenizer, PLX_TOKEN_SEMICOLON)) return NULL;

    // Create the member node.
    struct plx_node* const member = plx_new_node(PLX_NODE_OTHER, /*loc=*/NULL);
    member->children = member_name;
    member_name->next = member_value;
    *next = member;
    next = &member->next;
  }

  // Create the node.
  struct plx_node* const sstruct = plx_new_node(PLX_NODE_STRUCT, &loc);
  sstruct->children = identifier;
  identifier->next = members;
  return sstruct;
}

struct plx_node* plx_parse_identifier(struct plx_tokenizer* const tokenizer) {
  if (plx_unlikely(tokenizer->token != PLX_TOKEN_IDENTIFIER)) return NULL;
  struct plx_node* const identifier =
      plx_new_node(PLX_NODE_IDENTIFIER, &tokenizer->loc);
  identifier->name = plx_read_identifier_or_string(tokenizer);
  return identifier;
}

struct plx_node* plx_parse_int_lit(struct plx_tokenizer* const tokenizer) {
  assert(tokenizer->token == PLX_TOKEN_INT);
  struct plx_node* const s32 = plx_new_node(PLX_NODE_S32, &tokenizer->loc);
  s32->uint = tokenizer->uint;
  plx_next_token(tokenizer);
  return s32;
}

struct plx_node* plx_parse_string_lit(struct plx_tokenizer* const tokenizer) {
  if (plx_unlikely(tokenizer->token != PLX_TOKEN_STRING)) return NULL;
  struct plx_node* const string =
      plx_new_node(PLX_NODE_STRING, &tokenizer->loc);
  string->len = tokenizer->len;
  string->str = plx_read_identifier_or_string(tokenizer);
  return string;
}

struct plx_node* plx_parse_type(struct plx_tokenizer* const tokenizer) {
  const struct plx_source_code_location loc = tokenizer->loc;
  switch (tokenizer->token) {
    case PLX_TOKEN_IDENTIFIER:
      return plx_parse_identifier(tokenizer);
    case PLX_TOKEN_S8:
      plx_next_token(tokenizer);
      return plx_new_node(PLX_NODE_S8_TYPE, &loc);
    case PLX_TOKEN_S16:
      plx_next_token(tokenizer);
      return plx_new_node(PLX_NODE_S16_TYPE, &loc);
    case PLX_TOKEN_S32:
      plx_next_token(tokenizer);
      return plx_new_node(PLX_NODE_S32_TYPE, &loc);
    case PLX_TOKEN_S64:
      plx_next_token(tokenizer);
      return plx_new_node(PLX_NODE_S64_TYPE, &loc);
    case PLX_TOKEN_U8:
      plx_next_token(tokenizer);
      return plx_new_node(PLX_NODE_U8_TYPE, &loc);
    case PLX_TOKEN_U16:
      plx_next_token(tokenizer);
      return plx_new_node(PLX_NODE_U16_TYPE, &loc);
    case PLX_TOKEN_U32:
      plx_next_token(tokenizer);
      return plx_new_node(PLX_NODE_U32_TYPE, &loc);
    case PLX_TOKEN_U64:
      plx_next_token(tokenizer);
      return plx_new_node(PLX_NODE_U64_TYPE, &loc);
    case PLX_TOKEN_F32:
      plx_next_token(tokenizer);
      return plx_new_node(PLX_NODE_F32_TYPE, &loc);
    case PLX_TOKEN_F64:
      plx_next_token(tokenizer);
      return plx_new_node(PLX_NODE_F64_TYPE, &loc);
    case PLX_TOKEN_FUNC:
      return plx_parse_func_type(tokenizer);
    case PLX_TOKEN_REF:
      return plx_parse_ref_type(tokenizer);
    case PLX_TOKEN_OPEN_SQUARE_BRACKET:
      return plx_parse_array_or_slice_type(tokenizer);
    default:
      return NULL;
  }
}

struct plx_node* plx_parse_func_type(struct plx_tokenizer* const tokenizer) {
  assert(tokenizer->token == PLX_TOKEN_FUNC);
  const struct plx_source_code_location loc = tokenizer->loc;
  plx_next_token(tokenizer);

  // Parse the parameter types.
  if (!plx_accept_token(tokenizer, PLX_TOKEN_OPEN_PAREN)) return NULL;
  struct plx_node* const param_types =
      plx_new_node(PLX_NODE_OTHER, /*loc=*/NULL);
  struct plx_node** next = &param_types->children;
  if (!plx_accept_token(tokenizer, PLX_TOKEN_CLOSE_PAREN)) {
    while (true) {
      struct plx_node* const param_type = plx_parse_type(tokenizer);
      if (plx_unlikely(param_type == NULL)) return NULL;
      *next = param_type;
      next = &param_type->next;
      if (plx_accept_token(tokenizer, PLX_TOKEN_CLOSE_PAREN)) break;
      if (!plx_accept_token(tokenizer, PLX_TOKEN_COMMA)) return NULL;
    }
  }

  // Parse the return type.
  struct plx_node* return_type;
  if (plx_accept_token(tokenizer, PLX_TOKEN_ARROW)) {
    return_type = plx_parse_type(tokenizer);
    if (plx_unlikely(return_type == NULL)) return NULL;
  } else {
    return_type = plx_new_node(PLX_NODE_VOID_TYPE, /*loc=*/NULL);
  }

  // Create the node.
  struct plx_node* const func_type = plx_new_node(PLX_NODE_FUNC_TYPE, &loc);
  func_type->children = param_types;
  param_types->next = return_type;
  return func_type;
}

struct plx_node* plx_parse_ref_type(struct plx_tokenizer* const tokenizer) {
  assert(tokenizer->token == PLX_TOKEN_REF);
  const struct plx_source_code_location loc = tokenizer->loc;
  plx_next_token(tokenizer);

  // Parse the type.
  struct plx_node* const type = plx_parse_type(tokenizer);
  if (plx_unlikely(type == NULL)) return NULL;

  // Create the node.
  struct plx_node* const ref_type = plx_new_node(PLX_NODE_REF_TYPE, &loc);
  ref_type->children = type;
  return ref_type;
}

struct plx_node* plx_parse_array_or_slice_type(
    struct plx_tokenizer* const tokenizer) {
  assert(tokenizer->token == PLX_TOKEN_OPEN_SQUARE_BRACKET);
  const struct plx_source_code_location loc = tokenizer->loc;
  plx_next_token(tokenizer);

  // Parse a slice.
  if (plx_accept_token(tokenizer, PLX_TOKEN_CLOSE_SQUARE_BRACKET)) {
    // Parse the element type.
    struct plx_node* const element_type = plx_parse_type(tokenizer);
    if (plx_unlikely(element_type == NULL)) return NULL;

    // Create the node.
    struct plx_node* const slice_type = plx_new_node(PLX_NODE_SLICE_TYPE, &loc);
    slice_type->children = element_type;
    return slice_type;
  }

  // Parse the length.
  struct plx_node* const len = plx_parse_expr(tokenizer);
  if (plx_unlikely(len == NULL)) return NULL;

  if (!plx_accept_token(tokenizer, PLX_TOKEN_CLOSE_SQUARE_BRACKET)) return NULL;

  // Parse the element type.
  struct plx_node* const element_type = plx_parse_type(tokenizer);
  if (plx_unlikely(element_type == NULL)) return NULL;

  // Create the node.
  struct plx_node* const array_type = plx_new_node(PLX_NODE_ARRAY_TYPE, &loc);
  array_type->children = len;
  len->next = element_type;
  return array_type;
}
