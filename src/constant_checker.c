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

#include "constant_checker.h"

#include <assert.h>

#include "error.h"
#include "source_code_printer.h"
#include "symbol_table_entry.h"

static void plx_expected_constant(const struct plx_node* const node) {
  plx_error("expected a constant");
  plx_print_source_code(&node->loc,
                        /*annotation=*/"this should be a constant",
                        PLX_SOURCE_ANNOTATION_ERROR);
}

static void plx_cannot_assign_to_a_constant(const struct plx_node* const node) {
  plx_error("cannot assign to a constant");
  plx_print_source_code(&node->loc,
                        /*annotation=*/"this assignment is to a constant",
                        PLX_SOURCE_ANNOTATION_ERROR);
}

bool plx_check_constants(struct plx_node* const node) {
  bool result = true;
  switch (node->kind) {
    case PLX_NODE_CONST_DEF:
    case PLX_NODE_VAR_DEF: {
      struct plx_node *name, *value;
      plx_extract_children(node, &name, &value);
      if (!plx_check_constants(value)) result = false;
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
      struct plx_node *target, *value;
      plx_extract_children(node, &target, &value);
      if (!plx_check_constants(target)) result = false;
      // TODO: Prevent constant assignment.
      if (!plx_check_constants(value)) result = false;
      break;
    }
    case PLX_NODE_ARRAY_TYPE: {
      struct plx_node *len, *element_type;
      plx_extract_children(node, &len, &element_type);
      if (!plx_check_constants(len)) result = false;
      if (!plx_is_constant(len)) {
        plx_expected_constant(node);
        result = false;
      }
      break;
    }
    default:
      for (struct plx_node* child = node->children; child != NULL;
           child = child->next) {
        if (!plx_check_constants(child)) result = false;
      }
  }
  return result;
}
