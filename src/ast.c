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

#include "ast.h"

#include <stdarg.h>

#include "error.h"
#include "macros.h"
#include "memory_pool.h"

struct plx_node* plx_new_node(
    const enum plx_node_kind kind,
    const struct plx_source_code_location* const loc) {
  static thread_local struct plx_memory_pool pool;
  struct plx_node* const node =
      plx_memory_pool_alloc(&pool, sizeof(struct plx_node));
  if (plx_unlikely(node == NULL)) plx_oom();
  *node = (struct plx_node){kind};
  if (loc != NULL) node->loc = *loc;
  return node;
}

struct plx_node* plx_copy_node(const struct plx_node* const node) {
  if (node == NULL) return NULL;
  struct plx_node* const copy = plx_new_node(node->kind, /*loc=*/NULL);
  *copy = *node;
  copy->next = NULL;
  struct plx_node** next = &copy->children;
  for (const struct plx_node* child = node->children; child != NULL;
       child = child->next) {
    *next = plx_copy_node(child);
    next = &(*next)->next;
  }
  return copy;
}

size_t plx_count_children(const struct plx_node* const node) {
  size_t count = 0;
  for (const struct plx_node* child = node->children; child != NULL;
       child = child->next) {
    ++count;
  }
  return count;
}

void plx_extract_children(const struct plx_node* const node, ...) {
  va_list arg;
  va_start(arg, node);
  for (struct plx_node* child = node->children; child != NULL;
       child = child->next) {
    *va_arg(arg, struct plx_node**) = child;
  }
  va_end(arg);
}

bool plx_is_constant(const struct plx_node* const node) {
  return node->kind == PLX_NODE_S8 || node->kind == PLX_NODE_S16 ||
         node->kind == PLX_NODE_S32 || node->kind == PLX_NODE_S64 ||
         node->kind == PLX_NODE_U8 || node->kind == PLX_NODE_U16 ||
         node->kind == PLX_NODE_U32 || node->kind == PLX_NODE_U64 ||
         node->kind == PLX_NODE_F16 || node->kind == PLX_NODE_F32 ||
         node->kind == PLX_NODE_F64 || node->kind == PLX_NODE_BOOL ||
         node->kind == PLX_NODE_STRING;
}
