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

#include "return_checker.h"

#include "error.h"
#include "source_code_printer.h"

static void plx_missing_return_statement(
    const struct plx_node* const func_def) {
  plx_error("missing return statement");
  plx_print_source_code(
      &func_def->loc,
      /*annotation=*/"this function is missing a return statement",
      PLX_SOURCE_ANNOTATION_ERROR);
}

bool plx_check_returns(const struct plx_node* const node) {
  switch (node->kind) {
    case PLX_NODE_MODULE: {
      bool result = true;
      for (const struct plx_node* def = node->children; def != NULL;
           def = def->next) {
        if (def->kind != PLX_NODE_FUNC_DEF) continue;
        if (!plx_check_returns(def)) result = false;
      }
      return result;
    }
    case PLX_NODE_FUNC_DEF: {
      const struct plx_node *name, *params, *return_type, *body;
      plx_extract_children(node, &name, &params, &return_type, &body);
      if (return_type->kind == PLX_NODE_VOID_TYPE) return true;
      if (!plx_check_returns(body)) {
        plx_missing_return_statement(node);
        return false;
      }
      return true;
    }
    case PLX_NODE_BLOCK:
      for (const struct plx_node* stmt = node->children; stmt != NULL;
           stmt = stmt->next) {
        if (plx_check_returns(stmt)) return true;
      }
      return false;
    case PLX_NODE_IF_THEN_ELSE: {
      const struct plx_node *cond, *then, *els;
      plx_extract_children(node, &cond, &then, &els);
      return plx_check_returns(then) && plx_check_returns(els);
    }
    case PLX_NODE_RETURN:
      return true;
    default:
      return false;
  }
}
