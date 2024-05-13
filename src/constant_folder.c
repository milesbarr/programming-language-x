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

#include "constant_folder.h"

#include <limits.h>

#include "symbol_table_entry.h"

static void plx_nop(struct plx_node* const node) {
  node->kind = PLX_NODE_NOP;
  node->children = NULL;
}

bool plx_fold_constants(struct plx_node* const node) {
  bool changed = false;
  for (struct plx_node* child = node->children; child != NULL;
       child = child->next) {
    if (plx_fold_constants(child)) changed = true;
  }
  switch (node->kind) {
    case PLX_NODE_CONST_DEF: {
      struct plx_node *name, *value;
      plx_extract_children(node, &name, &value);
      if (name->entry == NULL) break;
      if (!plx_is_constant(value)) break;
      name->entry->value = value;
      plx_nop(node);
      changed = true;
      break;
    }
    case PLX_NODE_IF_THEN_ELSE: {
      struct plx_node *cond, *then, *els;
      plx_extract_children(node, &cond, &then, &els);
      if (cond->kind != PLX_NODE_BOOL) break;
      node->kind = PLX_NODE_BLOCK;
      struct plx_node* const next = node->next;
      *node = cond->b ? *then : *els;
      node->next = next;
      changed = true;
      break;
    }
    case PLX_NODE_WHILE_LOOP: {
      struct plx_node *cond, *body;
      plx_extract_children(node, &cond, &body);
      if (cond->kind != PLX_NODE_BOOL) break;
      if (cond->b) {
        node->kind = PLX_NODE_LOOP;
        node->children = body;
      } else {
        plx_nop(node);
      }
      changed = true;
      break;
    }
    case PLX_NODE_AND: {
      const struct plx_node *left, *right;
      plx_extract_children(node, &left, &right);
      if (left->kind != right->kind) break;
      switch (left->kind) {
        case PLX_NODE_S8:
        case PLX_NODE_S16:
        case PLX_NODE_S32:
        case PLX_NODE_S64:
          node->kind = left->kind;
          node->sint = left->sint & right->sint;
          node->children = NULL;
          changed = true;
          break;
        case PLX_NODE_U8:
        case PLX_NODE_U16:
        case PLX_NODE_U32:
        case PLX_NODE_U64:
          node->kind = left->kind;
          node->uint = left->uint & right->uint;
          node->children = NULL;
          changed = true;
          break;
        case PLX_NODE_BOOL:
          node->kind = left->kind;
          node->b = left->b && right->b;
          node->children = NULL;
          changed = true;
          break;
        default: {
        }
      }
      break;
    }
    case PLX_NODE_OR: {
      const struct plx_node *left, *right;
      plx_extract_children(node, &left, &right);
      if (left->kind != right->kind) break;
      switch (left->kind) {
        case PLX_NODE_S8:
        case PLX_NODE_S16:
        case PLX_NODE_S32:
        case PLX_NODE_S64:
          node->kind = left->kind;
          node->sint = left->sint | right->sint;
          node->children = NULL;
          changed = true;
          break;
        case PLX_NODE_U8:
        case PLX_NODE_U16:
        case PLX_NODE_U32:
        case PLX_NODE_U64:
          node->kind = left->kind;
          node->uint = left->uint | right->uint;
          node->children = NULL;
          changed = true;
          break;
        case PLX_NODE_BOOL:
          node->kind = left->kind;
          node->b = left->b || right->b;
          node->children = NULL;
          changed = true;
          break;
        default: {
        }
      }
      break;
    }
    case PLX_NODE_XOR: {
      const struct plx_node *left, *right;
      plx_extract_children(node, &left, &right);
      if (left->kind != right->kind) break;
      switch (left->kind) {
        case PLX_NODE_S8:
        case PLX_NODE_S16:
        case PLX_NODE_S32:
        case PLX_NODE_S64:
          node->kind = left->kind;
          node->sint = left->sint ^ right->sint;
          node->children = NULL;
          changed = true;
          break;
        case PLX_NODE_U8:
        case PLX_NODE_U16:
        case PLX_NODE_U32:
        case PLX_NODE_U64:
          node->kind = left->kind;
          node->uint = left->uint ^ right->uint;
          node->children = NULL;
          changed = true;
          break;
        case PLX_NODE_BOOL:
          node->kind = left->kind;
          node->b = left->b ^ right->b;
          node->children = NULL;
          changed = true;
          break;
        default: {
        }
      }
      break;
    }
    case PLX_NODE_EQ:
      // TODO
      break;
    case PLX_NODE_NEQ:
      // TODO
      break;
    case PLX_NODE_LTE: {
      const struct plx_node *left, *right;
      plx_extract_children(node, &left, &right);
      if (left->kind != right->kind) break;
      switch (left->kind) {
        case PLX_NODE_S8:
        case PLX_NODE_S16:
        case PLX_NODE_S32:
        case PLX_NODE_S64:
          node->kind = PLX_NODE_BOOL;
          node->b = left->sint <= right->sint;
          node->children = NULL;
          changed = true;
          break;
        case PLX_NODE_U8:
        case PLX_NODE_U16:
        case PLX_NODE_U32:
        case PLX_NODE_U64:
          node->kind = PLX_NODE_BOOL;
          node->b = left->uint <= right->uint;
          node->children = NULL;
          changed = true;
          break;
        case PLX_NODE_F16:
        case PLX_NODE_F32:
        case PLX_NODE_F64:
          node->kind = PLX_NODE_BOOL;
          node->b = left->f <= right->f;
          node->children = NULL;
          changed = true;
          break;
      }
      break;
    }
    case PLX_NODE_LT: {
      const struct plx_node *left, *right;
      plx_extract_children(node, &left, &right);
      if (left->kind != right->kind) break;
      switch (left->kind) {
        case PLX_NODE_S8:
        case PLX_NODE_S16:
        case PLX_NODE_S32:
        case PLX_NODE_S64:
          node->kind = PLX_NODE_BOOL;
          node->b = left->sint < right->sint;
          node->children = NULL;
          changed = true;
          break;
        case PLX_NODE_U8:
        case PLX_NODE_U16:
        case PLX_NODE_U32:
        case PLX_NODE_U64:
          node->kind = PLX_NODE_BOOL;
          node->b = left->uint < right->uint;
          node->children = NULL;
          changed = true;
          break;
        case PLX_NODE_F16:
        case PLX_NODE_F32:
        case PLX_NODE_F64:
          node->kind = PLX_NODE_BOOL;
          node->b = left->f < right->f;
          node->children = NULL;
          changed = true;
          break;
      }
      break;
    }
    case PLX_NODE_GTE: {
      const struct plx_node *left, *right;
      plx_extract_children(node, &left, &right);
      if (left->kind != right->kind) break;
      switch (left->kind) {
        case PLX_NODE_S8:
        case PLX_NODE_S16:
        case PLX_NODE_S32:
        case PLX_NODE_S64:
          node->kind = PLX_NODE_BOOL;
          node->b = left->sint >= right->sint;
          node->children = NULL;
          changed = true;
          break;
        case PLX_NODE_U8:
        case PLX_NODE_U16:
        case PLX_NODE_U32:
        case PLX_NODE_U64:
          node->kind = PLX_NODE_BOOL;
          node->b = left->uint >= right->uint;
          node->children = NULL;
          changed = true;
          break;
        case PLX_NODE_F16:
        case PLX_NODE_F32:
        case PLX_NODE_F64:
          node->kind = PLX_NODE_BOOL;
          node->b = left->f >= right->f;
          node->children = NULL;
          changed = true;
          break;
      }
      break;
    }
    case PLX_NODE_GT: {
      const struct plx_node *left, *right;
      plx_extract_children(node, &left, &right);
      if (left->kind != right->kind) break;
      switch (left->kind) {
        case PLX_NODE_S8:
        case PLX_NODE_S16:
        case PLX_NODE_S32:
        case PLX_NODE_S64:
          node->kind = PLX_NODE_BOOL;
          node->b = left->sint > right->sint;
          node->children = NULL;
          changed = true;
          break;
        case PLX_NODE_U8:
        case PLX_NODE_U16:
        case PLX_NODE_U32:
        case PLX_NODE_U64:
          node->kind = PLX_NODE_BOOL;
          node->b = left->uint > right->uint;
          node->children = NULL;
          changed = true;
          break;
        case PLX_NODE_F16:
        case PLX_NODE_F32:
        case PLX_NODE_F64:
          node->kind = PLX_NODE_BOOL;
          node->b = left->f > right->f;
          node->children = NULL;
          changed = true;
          break;
      }
      break;
    }
    case PLX_NODE_ADD: {
      const struct plx_node *left, *right;
      plx_extract_children(node, &left, &right);
      if (left->kind != right->kind) break;
      switch (left->kind) {
        case PLX_NODE_S8:
        case PLX_NODE_S16:
        case PLX_NODE_S32:
        case PLX_NODE_S64:
          // Overflow
          if (right->sint > 0 && left->sint > LLONG_MAX - right->sint) break;
          // Underflow
          if (right->sint < 0 && left->sint < LLONG_MIN - right->sint) break;
          node->kind = left->kind;
          node->sint = left->sint + right->sint;
          node->children = NULL;
          changed = true;
          break;
        case PLX_NODE_U8:
        case PLX_NODE_U16:
        case PLX_NODE_U32:
        case PLX_NODE_U64:
          // Overflow
          if (right->uint > 0 && left->uint > ULLONG_MAX - right->uint) break;
          // Underflow
          if (right->uint < 0 && left->uint < right->uint) break;
          node->kind = left->kind;
          node->uint = left->uint + right->uint;
          node->children = NULL;
          changed = true;
          break;
        case PLX_NODE_F16:
        case PLX_NODE_F32:
        case PLX_NODE_F64:
          node->kind = left->kind;
          node->f = left->f + right->f;
          node->children = NULL;
          changed = true;
          break;
        default: {
        }
      }
      break;
    }
    case PLX_NODE_SUB: {
      const struct plx_node *left, *right;
      plx_extract_children(node, &left, &right);
      if (left->kind != right->kind) break;
      switch (left->kind) {
        case PLX_NODE_S8:
        case PLX_NODE_S16:
        case PLX_NODE_S32:
        case PLX_NODE_S64:
          // Overflow
          if (right->sint < 0 && left->sint > LLONG_MAX + right->sint) break;
          // Underflow
          if (right->sint > 0 && left->sint < LLONG_MIN + right->sint) break;
          node->kind = left->kind;
          node->sint = left->sint - right->sint;
          node->children = NULL;
          changed = true;
          break;
        case PLX_NODE_U8:
        case PLX_NODE_U16:
        case PLX_NODE_U32:
        case PLX_NODE_U64:
          // Overflow
          if (right->uint < 0 && left->uint > ULLONG_MAX + right->uint) break;
          // Underflow
          if (right->uint > 0 && left->uint < right->uint) break;
          node->kind = left->kind;
          node->uint = left->uint - right->uint;
          node->children = NULL;
          changed = true;
          break;
        case PLX_NODE_F16:
        case PLX_NODE_F32:
        case PLX_NODE_F64:
          node->kind = left->kind;
          node->f = left->f - right->f;
          node->children = NULL;
          changed = true;
          break;
        default: {
        }
      }
      break;
    }
    case PLX_NODE_MUL: {
      const struct plx_node *left, *right;
      plx_extract_children(node, &left, &right);
      if (left->kind != right->kind) break;
      switch (left->kind) {
        case PLX_NODE_S8:
        case PLX_NODE_S16:
        case PLX_NODE_S32:
        case PLX_NODE_S64:
          // Overflow
          if (left->sint == -1 && right->sint == LLONG_MIN) break;
          if (right->sint == -1 && left->sint == LLONG_MIN) break;
          if (right->sint != 0 && left->sint > LLONG_MAX / right->sint) break;
          // Underflow
          if (right->sint != 0 && left->sint < LLONG_MIN / right->sint) break;
          node->kind = left->kind;
          node->sint = left->sint * right->sint;
          node->children = NULL;
          changed = true;
          break;
        case PLX_NODE_U8:
        case PLX_NODE_U16:
        case PLX_NODE_U32:
        case PLX_NODE_U64:
          node->kind = left->kind;
          node->uint = left->uint * right->uint;
          node->children = NULL;
          changed = true;
          break;
        case PLX_NODE_F16:
        case PLX_NODE_F32:
        case PLX_NODE_F64:
          node->kind = left->kind;
          node->f = left->f * right->f;
          node->children = NULL;
          changed = true;
          break;
        default: {
        }
      }
      break;
    }
    case PLX_NODE_DIV: {
      const struct plx_node *left, *right;
      plx_extract_children(node, &left, &right);
      if (left->kind != right->kind) break;
      switch (left->kind) {
        case PLX_NODE_S8:
        case PLX_NODE_S16:
        case PLX_NODE_S32:
        case PLX_NODE_S64:
          node->kind = left->kind;
          node->sint = left->sint / right->sint;
          node->children = NULL;
          changed = true;
          break;
        case PLX_NODE_U8:
        case PLX_NODE_U16:
        case PLX_NODE_U32:
        case PLX_NODE_U64:
          node->kind = left->kind;
          node->uint = left->uint / right->uint;
          node->children = NULL;
          changed = true;
          break;
        case PLX_NODE_F16:
        case PLX_NODE_F32:
        case PLX_NODE_F64:
          node->kind = left->kind;
          node->f = left->f / right->f;
          node->children = NULL;
          changed = true;
          break;
        default: {
        }
      }
      break;
    }
    case PLX_NODE_REM: {
      const struct plx_node *left, *right;
      plx_extract_children(node, &left, &right);
      if (left->kind != right->kind) break;
      switch (left->kind) {
        case PLX_NODE_S8:
        case PLX_NODE_S16:
        case PLX_NODE_S32:
        case PLX_NODE_S64:
          node->kind = left->kind;
          node->sint = left->sint % right->sint;
          node->children = NULL;
          changed = true;
          break;
        case PLX_NODE_U8:
        case PLX_NODE_U16:
        case PLX_NODE_U32:
        case PLX_NODE_U64:
          node->kind = left->kind;
          node->uint = left->uint % right->uint;
          node->children = NULL;
          changed = true;
          break;
        default: {
        }
      }
      break;
    }
    case PLX_NODE_LSHIFT: {
      const struct plx_node *left, *right;
      plx_extract_children(node, &left, &right);
      if (left->kind != right->kind) break;
      switch (left->kind) {
        case PLX_NODE_S8:
        case PLX_NODE_S16:
        case PLX_NODE_S32:
        case PLX_NODE_S64:
          node->kind = left->kind;
          node->sint = left->sint << right->sint;
          node->children = NULL;
          changed = true;
          break;
        case PLX_NODE_U8:
        case PLX_NODE_U16:
        case PLX_NODE_U32:
        case PLX_NODE_U64:
          node->kind = left->kind;
          node->uint = left->uint << right->uint;
          node->children = NULL;
          changed = true;
          break;
        default: {
        }
      }
      break;
    }
    case PLX_NODE_RSHIFT: {
      const struct plx_node *left, *right;
      plx_extract_children(node, &left, &right);
      if (left->kind != right->kind) break;
      switch (left->kind) {
        case PLX_NODE_S8:
        case PLX_NODE_S16:
        case PLX_NODE_S32:
        case PLX_NODE_S64:
          node->kind = left->kind;
          node->sint = left->sint >> right->sint;
          node->children = NULL;
          changed = true;
          break;
        case PLX_NODE_U8:
        case PLX_NODE_U16:
        case PLX_NODE_U32:
        case PLX_NODE_U64:
          node->kind = left->kind;
          node->uint = left->uint >> right->uint;
          node->children = NULL;
          changed = true;
          break;
        default: {
        }
      }
      break;
    }
    case PLX_NODE_NOT: {
      struct plx_node* operand;
      plx_extract_children(node, &operand);
      switch (operand->kind) {
        case PLX_NODE_S8:
        case PLX_NODE_S16:
        case PLX_NODE_S32:
        case PLX_NODE_S64:
          node->kind = operand->kind;
          node->sint = ~operand->sint;
          node->children = NULL;
          changed = true;
          break;
        case PLX_NODE_U8:
        case PLX_NODE_U16:
        case PLX_NODE_U32:
        case PLX_NODE_U64:
          node->kind = operand->kind;
          node->sint = ~operand->uint;
          node->children = NULL;
          changed = true;
          break;
        case PLX_NODE_BOOL:
          node->kind = PLX_NODE_BOOL;
          node->b = !operand->b;
          node->children = NULL;
          changed = true;
          break;
        default: {
        }
      }
    }
    case PLX_NODE_NEG: {
      struct plx_node* operand;
      plx_extract_children(node, &operand);
      switch (operand->kind) {
        case PLX_NODE_S8:
        case PLX_NODE_S16:
        case PLX_NODE_S32:
        case PLX_NODE_S64:
          if (operand->sint == LLONG_MIN) break;  // Overflow
          node->kind = operand->kind;
          node->sint = -operand->sint;
          node->children = NULL;
          changed = true;
          break;
        case PLX_NODE_F16:
        case PLX_NODE_F32:
        case PLX_NODE_F64:
          node->kind = operand->kind;
          node->f = -operand->f;
          node->children = NULL;
          changed = true;
          break;
        default: {
        }
      }
      break;
    }
    case PLX_NODE_IDENTIFIER: {
      if (node->entry == NULL || node->entry->value == NULL) break;
      // TODO: Free the name.
      struct plx_node* const next = node->next;
      *node = *node->entry->value;
      node->next = next;
      changed = true;
      break;
    }
    default: {
    }
  }
  return changed;
}
