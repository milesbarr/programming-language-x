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

#include "types.h"

bool plx_is_sint_type(const struct plx_node* const type) {
  return type->kind == PLX_NODE_S8_TYPE || type->kind == PLX_NODE_S16_TYPE ||
         type->kind == PLX_NODE_S32_TYPE || type->kind == PLX_NODE_S64_TYPE;
}

bool plx_is_uint_type(const struct plx_node* const type) {
  return type->kind == PLX_NODE_U8_TYPE || type->kind == PLX_NODE_U16_TYPE ||
         type->kind == PLX_NODE_U32_TYPE || type->kind == PLX_NODE_U64_TYPE;
}

bool plx_is_int_type(const struct plx_node* const type) {
  return plx_is_sint_type(type) || plx_is_uint_type(type);
}

bool plx_is_float_type(const struct plx_node* const type) {
  return type->kind == PLX_NODE_F16_TYPE || type->kind == PLX_NODE_F32_TYPE ||
         type->kind == PLX_NODE_F64_TYPE;
}

bool plx_is_numeric_type(const struct plx_node* const type) {
  return plx_is_int_type(type) || plx_is_float_type(type);
}

bool plx_is_logical_type(const struct plx_node* const type) {
  return plx_is_int_type(type) || type->kind == PLX_NODE_BOOL_TYPE;
}

bool plx_is_equality_type(const struct plx_node* const type) {
  return plx_is_numeric_type(type) || type->kind == PLX_NODE_BOOL_TYPE ||
         type->kind == PLX_NODE_STRING_TYPE;
}

bool plx_type_eq(const struct plx_node* const type_a,
                 const struct plx_node* const type_b) {
  if (type_a == type_b) return true;
  if (type_a->kind != type_b->kind) return false;
  if (type_a->kind == PLX_NODE_IDENTIFIER) {
    return type_a->entry == type_b->entry;
  }
  const struct plx_node* child_a = type_a->children;
  const struct plx_node* child_b = type_b->children;
  while (child_a != NULL && child_b != NULL) {
    if (!plx_type_eq(child_a, child_b)) return false;
    child_a = child_a->next;
    child_b = child_b->next;
  }
  return child_a == NULL && child_b == NULL;
}
