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

#include "name_resolver.h"

#include <assert.h>
#include <stddef.h>

#include "error.h"
#include "source_code_location.h"
#include "source_code_printer.h"
#include "symbol_table_entry.h"

static void plx_undeclared_identifier(const struct plx_node* const identifier) {
  plx_error("undeclared identifier `%s`", identifier->name);
  plx_print_source_code(&identifier->loc,
                        /*annotation=*/"this identifier has not been declared",
                        PLX_SOURCE_ANNOTATION_ERROR);
}

static void plx_identifier_already_declared(
    const struct plx_node* const identifier,
    const struct plx_source_code_location* const decl) {
  plx_error("identifier `%s` already declared", identifier->name);
  plx_print_source_code(&identifier->loc,
                        /*annotation=*/"this identifier was already declared",
                        PLX_SOURCE_ANNOTATION_ERROR);
  if (decl != NULL) {
    plx_print_source_code(decl,
                          /*annotation=*/"it was previously declared here",
                          PLX_SOURCE_ANNOTATION_INFO);
  }
}

static bool plx_declare_identifier(
    struct plx_node* const identifier,
    struct plx_symbol_table* const symbol_table) {
  assert(identifier->kind == PLX_NODE_IDENTIFIER);
  if (identifier->entry != NULL) return true;
  struct plx_symbol_table_entry* entry =
      plx_declare_symbol(symbol_table, identifier->name);
  if (entry == NULL) {
    entry = plx_lookup_symbol(symbol_table, identifier->name);
    plx_identifier_already_declared(identifier, entry->decl);
    return false;
  }
  entry->decl = &identifier->loc;
  identifier->entry = entry;
  return true;
}

bool plx_resolve_names(struct plx_node* const node,
                       struct plx_symbol_table* const symbol_table) {
  bool result = true;
  switch (node->kind) {
    case PLX_NODE_MODULE:
      for (struct plx_node* def = node->children; def != NULL;
           def = def->next) {
        struct plx_node* name = def->children;
        if (!plx_declare_identifier(name, symbol_table)) result = false;
        if (name->entry != NULL) name->entry->scope = PLX_SYMBOL_SCOPE_GLOBAL;
      }
      for (struct plx_node* def = node->children; def != NULL;
           def = def->next) {
        if (!plx_resolve_names(def, symbol_table)) result = false;
      }
      break;
    case PLX_NODE_CONST_DEF:
    case PLX_NODE_VAR_DEF:
    case PLX_NODE_VAR_DECL: {
      struct plx_node *name, *value_or_type;
      plx_extract_children(node, &name, &value_or_type);
      if (!plx_resolve_names(value_or_type, symbol_table)) result = false;
      if (!plx_declare_identifier(name, symbol_table)) result = false;
      break;
    }
    case PLX_NODE_STRUCT_DEF:
      break;
    case PLX_NODE_FUNC_DEF: {
      struct plx_node *name, *params, *return_type, *body;
      plx_extract_children(node, &name, &params, &return_type, &body);
      if (!plx_declare_identifier(name, symbol_table)) result = false;
      plx_enter_scope(symbol_table);
      for (struct plx_node* param = params->children; param != NULL;
           param = param->next) {
        struct plx_node *param_name, *param_type;
        plx_extract_children(param, &param_name, &param_type);
        if (!plx_resolve_names(param_type, symbol_table)) result = false;
        if (!plx_declare_identifier(param_name, symbol_table)) result = false;
      }
      if (!plx_resolve_names(return_type, symbol_table)) result = false;
      if (!plx_resolve_names(body, symbol_table)) result = false;
      plx_exit_scope(symbol_table);
      break;
    }
    case PLX_NODE_BLOCK:
      plx_enter_scope(symbol_table);
      for (struct plx_node* stmt = node->children; stmt != NULL;
           stmt = stmt->next) {
        if (!plx_resolve_names(stmt, symbol_table)) result = false;
      }
      plx_exit_scope(symbol_table);
      break;
    case PLX_NODE_IDENTIFIER:
      if (node->entry != NULL) break;
      node->entry = plx_lookup_symbol(symbol_table, node->name);
      if (node->entry == NULL) {
        plx_undeclared_identifier(node);
        result = false;
      }
      break;
    default:
      for (struct plx_node* child = node->children; child != NULL;
           child = child->next) {
        plx_resolve_names(child, symbol_table);
      }
  }
  return result;
}
