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

#include "ast_validator.h"

#include "error.h"
#include "source_code_printer.h"
#include "symbol_table_entry.h"

static void plx_expected_constant(const struct plx_node* const node) {
  plx_error("expected a constant");
  plx_print_source_code(&node->loc,
                        /*annotation=*/"this should be a constant",
                        PLX_SOURCE_ANNOTATION_ERROR);
}

static void plx_expr_not_assignable(const struct plx_node* const node) {
  plx_error("expression is not assignable");
  plx_print_source_code(&node->loc,
                        /*annotation=*/"this expression should be assignable",
                        PLX_SOURCE_ANNOTATION_ERROR);
}

static void plx_expr_not_referenceable(const struct plx_node* const node) {
  plx_error("expression is not referenceable");
  plx_print_source_code(
      &node->loc,
      /*annotation=*/"this expression should be referenceable",
      PLX_SOURCE_ANNOTATION_ERROR);
}

static bool plx_is_referenceable_expr(const struct plx_node* const expr) {
  return expr->kind == PLX_NODE_DEREF || expr->kind == PLX_NODE_INDEX ||
         expr->kind == PLX_NODE_IDENTIFIER;
}

bool plx_validate_ast(const struct plx_node* const node) {
  bool result = true;
  switch (node->kind) {
    case PLX_NODE_CONST_DEF:
    case PLX_NODE_VAR_DEF: {
      const struct plx_node *name, *value;
      plx_extract_children(node, &name, &value);
      if (!plx_validate_ast(value)) result = false;
      if (!plx_is_constant(value)) {
        plx_expected_constant(value);
        result = false;
      }
      break;
    }
    case PLX_NODE_ASSIGN:
    case PLX_NODE_ADD_ASSIGN:
    case PLX_NODE_SUB_ASSIGN:
    case PLX_NODE_MUL_ASSIGN:
    case PLX_NODE_DIV_ASSIGN:
    case PLX_NODE_REM_ASSIGN:
    case PLX_NODE_LSHIFT_ASSIGN:
    case PLX_NODE_RSHIFT_ASSIGN: {
      const struct plx_node *target, *value;
      plx_extract_children(node, &target, &value);
      if (!plx_validate_ast(target)) result = false;
      if (!plx_is_referenceable_expr(target)) {
        plx_expr_not_assignable(target);
        result = false;
      }
      if (!plx_validate_ast(value)) result = false;
      break;
    }
    case PLX_NODE_REF: {
      const struct plx_node* operand;
      plx_extract_children(node, &operand);
      if (!plx_validate_ast(operand)) result = false;
      if (!plx_is_referenceable_expr(operand)) {
        plx_expr_not_referenceable(node);
        result = false;
      }
      break;
    }
    case PLX_NODE_ARRAY_TYPE: {
      const struct plx_node *len, *element_type;
      plx_extract_children(node, &len, &element_type);
      if (!plx_validate_ast(len)) result = false;
      if (!plx_is_constant(len)) {
        plx_expected_constant(node);
        result = false;
      }
      break;
    }
    default:
      for (const struct plx_node* child = node->children; child != NULL;
           child = child->next) {
        if (!plx_validate_ast(child)) result = false;
      }
  }
  return result;
}
