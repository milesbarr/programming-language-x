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

#include "type_checker.h"

#include <string.h>

#include "error.h"
#include "source_code_printer.h"
#include "symbol_table_entry.h"
#include "types.h"

static void plx_unexpected_type(const struct plx_node* const node,
                                const char* const expected) {
  plx_error("expected %s", expected);
  char annotation[256] = "this expression should evaluate to ";
  strcat(annotation, expected);
  plx_print_source_code(&node->loc, annotation, PLX_SOURCE_ANNOTATION_ERROR);
}

static void plx_operand_type_mismatch(const struct plx_node* const node) {
  plx_error("operand type mismatch");
  plx_print_source_code(
      &node->loc,
      /*annotation=*/"the types of the operands in this expression must match",
      PLX_SOURCE_ANNOTATION_ERROR);
}

static void plx_return_type_mismatch(const struct plx_node* const return_value,
                                     const struct plx_node* const return_type) {
  plx_error("return type mismatch");
  plx_print_source_code(
      &return_value->loc,
      /*annotation=*/
      "the type of this return value does not match the function type",
      PLX_SOURCE_ANNOTATION_ERROR);
  if (return_type != NULL) {
    plx_print_source_code(&return_type->loc,
                          /*annotation=*/"this is the function type",
                          PLX_SOURCE_ANNOTATION_INFO);
  }
}

static void plx_too_few_arguments(const struct plx_node* const call) {
  plx_error("too few arguments in function call");
  plx_print_source_code(&call->loc,
                        /*annotation=*/
                        "this function call has too few arguments",
                        PLX_SOURCE_ANNOTATION_ERROR);
}

static void plx_too_many_arguments(const struct plx_node* const call) {
  plx_error("too many arguments in function call");
  plx_print_source_code(&call->loc,
                        /*annotation=*/
                        "this function call has too many arguments",
                        PLX_SOURCE_ANNOTATION_ERROR);
}

static void plx_argument_type_mismatch(
    const struct plx_node* const arg, const struct plx_node* const param_type) {
  plx_error("argument type mismatch");
  plx_print_source_code(
      &arg->loc,
      /*annotation=*/
      "the type of this argument does not match the parameter type",
      PLX_SOURCE_ANNOTATION_ERROR);
  if (param_type != NULL) {
    plx_print_source_code(&param_type->loc,
                          /*annotation=*/"this is the parameter type",
                          PLX_SOURCE_ANNOTATION_INFO);
  }
}

static void plx_set_identifier_type(struct plx_node* const identifier,
                                    const struct plx_node* const type) {
  identifier->type = type;
  if (identifier->entry != NULL) identifier->entry->type = type;
}

bool plx_type_check(struct plx_node* const node,
                    const struct plx_node* return_type) {
  static const struct plx_node s8_type = {PLX_NODE_S8_TYPE};
  static const struct plx_node s16_type = {PLX_NODE_S16_TYPE};
  static const struct plx_node s32_type = {PLX_NODE_S32_TYPE};
  static const struct plx_node s64_type = {PLX_NODE_S64_TYPE};
  static const struct plx_node u8_type = {PLX_NODE_U8_TYPE};
  static const struct plx_node u16_type = {PLX_NODE_U16_TYPE};
  static const struct plx_node u32_type = {PLX_NODE_U32_TYPE};
  static const struct plx_node u64_type = {PLX_NODE_U64_TYPE};
  static const struct plx_node f16_type = {PLX_NODE_F16_TYPE};
  static const struct plx_node f32_type = {PLX_NODE_F32_TYPE};
  static const struct plx_node f64_type = {PLX_NODE_F64_TYPE};
  static const struct plx_node bool_type = {PLX_NODE_BOOL_TYPE};
  static const struct plx_node string_type = {PLX_NODE_STRING_TYPE};

  bool result = true;
  switch (node->kind) {
    case PLX_NODE_CONST_DEF:
    case PLX_NODE_VAR_DEF: {
      struct plx_node *name, *value;
      plx_extract_children(node, &name, &value);
      if (!plx_type_check(value, return_type)) result = false;
      plx_set_identifier_type(name, value->type);
      break;
    }
    case PLX_NODE_VAR_DECL: {
      struct plx_node *name, *type;
      plx_extract_children(node, &name, &type);
      if (!plx_type_check(type, return_type)) result = false;
      plx_set_identifier_type(name, type);
      break;
    }
    case PLX_NODE_STRUCT_DEF:
      // TODO
      break;
    case PLX_NODE_FUNC_DEF: {
      struct plx_node *name, *params, *return_type, *body;
      plx_extract_children(node, &name, &params, &return_type, &body);

      // Type check the parameters.
      for (struct plx_node* param = params->children; param != NULL;
           param = param->next) {
        struct plx_node *param_name, *param_type;
        plx_extract_children(param, &param_name, &param_type);
        if (!plx_type_check(param_type, return_type)) result = false;
        plx_set_identifier_type(param_name, param_type);
      }

      // Type check the return type.
      if (!plx_type_check(return_type, return_type)) result = false;

      // Type check the body.
      if (!plx_type_check(body, return_type)) result = false;

      // Set the identifier type.
      struct plx_node* const param_types =
          plx_new_node(PLX_NODE_OTHER, /*loc=*/NULL);
      struct plx_node** next = &param_types->children;
      for (struct plx_node* param = params->children; param != NULL;
           param = param->next) {
        struct plx_node *param_name, *param_type;
        plx_extract_children(param, &param_name, &param_type);
        *next = plx_copy_node(param_type);
        next = &(*next)->next;
      }
      struct plx_node* const type =
          plx_new_node(PLX_NODE_FUNC_TYPE, /*loc=*/NULL);
      type->children = param_types;
      param_types->next = return_type;
      plx_set_identifier_type(name, type);
      break;
    }
    case PLX_NODE_NOP:
      break;
    case PLX_NODE_BLOCK:
      for (struct plx_node* stmt = node->children; stmt != NULL;
           stmt = stmt->next) {
        if (!plx_type_check(stmt, return_type)) result = false;
        if (stmt->type != NULL && stmt->type->kind != PLX_NODE_VOID_TYPE) {
          plx_unexpected_type(stmt, /*expected=*/"void");
          result = false;
        }
      }
      break;
    case PLX_NODE_IF_THEN_ELSE: {
      struct plx_node *cond, *then, *els;
      plx_extract_children(node, &cond, &then, &els);

      // Type check the condition.
      if (!plx_type_check(cond, return_type)) result = false;
      if (cond->type != NULL && cond->type->kind != PLX_NODE_BOOL_TYPE) {
        plx_unexpected_type(cond, /*expected=*/"a bool");
        result = false;
      }

      // Type check the "then" block.
      if (!plx_type_check(then, return_type)) result = false;

      // Type check the "else" block.
      if (!plx_type_check(els, return_type)) result = false;
      break;
    }
    case PLX_NODE_WHILE_LOOP: {
      struct plx_node *cond, *body;
      plx_extract_children(node, &cond, &body);

      // Type check the condition.
      if (!plx_type_check(cond, return_type)) result = false;
      if (cond->type != NULL && cond->type->kind != PLX_NODE_BOOL_TYPE) {
        plx_unexpected_type(cond, /*expected=*/"a bool");
        result = false;
      }

      // Type check the body.
      if (!plx_type_check(body, return_type)) result = false;
      break;
    }
    case PLX_NODE_RETURN: {
      struct plx_node* return_value;
      plx_extract_children(node, &return_value);

      // Type check the return value.
      if (return_value == NULL) {
        if (return_type->kind != PLX_NODE_VOID_TYPE) {
          plx_return_type_mismatch(node, return_type);
          result = false;
        }
      } else {
        if (!plx_type_check(return_value, return_type)) result = false;
        if (return_value->type != NULL &&
            !plx_type_eq(return_value->type, return_type)) {
          plx_return_type_mismatch(return_value, return_type);
          result = false;
        }
      }
      break;
    }
    case PLX_NODE_ASSIGN: {
      struct plx_node *assignee, *value;
      plx_extract_children(node, &assignee, &value);

      // Type check the assignee.
      if (!plx_type_check(assignee, return_type)) result = false;

      // Type check the value.
      if (!plx_type_check(value, return_type)) result = false;

      // Check for matching types.
      if (assignee->type != NULL && value->type != NULL &&
          !plx_type_eq(assignee->type, value->type)) {
        plx_operand_type_mismatch(node);
        result = false;
      }
      break;
    }
    case PLX_NODE_ADD_ASSIGN:
    case PLX_NODE_SUB_ASSIGN:
    case PLX_NODE_MUL_ASSIGN:
    case PLX_NODE_DIV_ASSIGN: {
      struct plx_node *assignee, *value;
      plx_extract_children(node, &assignee, &value);

      // Type check the assignee.
      if (!plx_type_check(assignee, return_type)) result = false;
      if (assignee->type != NULL && !plx_is_numeric_type(assignee->type)) {
        plx_unexpected_type(assignee, /*expected=*/"a number");
        result = false;
        break;
      }

      // Type check the value.
      if (!plx_type_check(value, return_type)) result = false;
      if (value->type != NULL && !plx_is_numeric_type(value->type)) {
        plx_unexpected_type(value, /*expected=*/"a number");
        result = false;
        break;
      }

      // Check for matching types.
      if (!plx_type_eq(assignee->type, value->type)) {
        plx_operand_type_mismatch(node);
        result = false;
        break;
      }
      break;
    }
    case PLX_NODE_REM_ASSIGN:
    case PLX_NODE_LSHIFT_ASSIGN:
    case PLX_NODE_RSHIFT_ASSIGN: {
      struct plx_node *assignee, *value;
      plx_extract_children(node, &assignee, &value);

      // Type check the assignee.
      if (!plx_type_check(assignee, return_type)) result = false;
      if (assignee->type != NULL && !plx_is_int_type(assignee->type)) {
        plx_unexpected_type(assignee, /*expected=*/"an integer");
        result = false;
        break;
      }

      // Type check the value operand.
      if (!plx_type_check(value, return_type)) result = false;
      if (value->type != NULL && !plx_is_int_type(value->type)) {
        plx_unexpected_type(value, /*expected=*/"an integer");
        result = false;
      }
      break;
    }
    case PLX_NODE_AND:
    case PLX_NODE_OR:
    case PLX_NODE_XOR: {
      struct plx_node *left, *right;
      plx_extract_children(node, &left, &right);

      // Type check the left operand.
      if (!plx_type_check(left, return_type)) result = false;
      if (left->type != NULL && !plx_is_logical_type(left->type)) {
        plx_unexpected_type(left, /*expected=*/"an integer or bool");
        result = false;
        break;
      }

      // Type check the right operand.
      if (!plx_type_check(right, return_type)) result = false;
      if (right->type != NULL && !plx_is_logical_type(right->type)) {
        plx_unexpected_type(right, /*expected=*/"an integer or bool");
        result = false;
        break;
      }

      // Check for matching types.
      if (!plx_type_eq(left, right)) {
        plx_operand_type_mismatch(node);
        result = false;
        break;
      }

      // Set the type.
      if (left->type != NULL && right->type != NULL) {
        node->type = left->type;
      }
      break;
    }
    case PLX_NODE_EQ:
    case PLX_NODE_NEQ: {
      struct plx_node *left, *right;
      plx_extract_children(node, &left, &right);

      // Set the type.
      node->type = &bool_type;

      // Type check the left operand.
      if (!plx_type_check(left, return_type)) result = false;
      if (left->type != NULL && !plx_is_equality_type(left->type)) {
        plx_unexpected_type(left, /*expected=*/"an integer, bool, or string");
        result = false;
        break;
      }

      // Type check the right operand.
      if (!plx_type_check(right, return_type)) result = false;
      if (right->type != NULL && !plx_is_equality_type(right->type)) {
        plx_unexpected_type(right, /*expected=*/"an integer, bool, or string");
        result = false;
        break;
      }

      // Check for matching types.
      if (!plx_type_eq(left, right)) {
        plx_operand_type_mismatch(node);
        result = false;
      }
      break;
    }
    case PLX_NODE_LTE:
    case PLX_NODE_LT:
    case PLX_NODE_GTE:
    case PLX_NODE_GT: {
      struct plx_node *left, *right;
      plx_extract_children(node, &left, &right);

      // Set the type.
      node->type = &bool_type;

      // Type check the left operand.
      if (!plx_type_check(left, return_type)) result = false;
      if (left->type != NULL && !plx_is_numeric_type(left->type)) {
        plx_unexpected_type(left, /*expected=*/"a number");
        result = false;
        break;
      }

      // Type check the right operand.
      if (!plx_type_check(right, return_type)) result = false;
      if (right->type != NULL && !plx_is_numeric_type(right->type)) {
        plx_unexpected_type(right, /*expected=*/"a number");
        result = false;
        break;
      }

      // Check for matching types.
      if (!plx_type_eq(left->type, right->type)) {
        plx_operand_type_mismatch(node);
        result = false;
      }
      break;
    }
    case PLX_NODE_ADD:
    case PLX_NODE_SUB:
    case PLX_NODE_MUL:
    case PLX_NODE_DIV: {
      struct plx_node *left, *right;
      plx_extract_children(node, &left, &right);

      // Type check the left operand.
      if (!plx_type_check(left, return_type)) result = false;
      if (left->type != NULL && !plx_is_numeric_type(left->type)) {
        plx_unexpected_type(left, /*expected=*/"a number");
        result = false;
        break;
      }

      // Type check the right operand.
      if (!plx_type_check(right, return_type)) result = false;
      if (right->type != NULL && !plx_is_numeric_type(right->type)) {
        plx_unexpected_type(right, /*expected=*/"a number");
        result = false;
        break;
      }

      // Check for matching types.
      if (!plx_type_eq(left->type, right->type)) {
        plx_operand_type_mismatch(node);
        result = false;
        break;
      }

      // Set the type.
      if (left->type != NULL && right->type != NULL) {
        node->type = left->type;
      }
      break;
    }
    case PLX_NODE_REM:
    case PLX_NODE_LSHIFT:
    case PLX_NODE_RSHIFT: {
      struct plx_node *left, *right;
      plx_extract_children(node, &left, &right);

      // Type check the left operand.
      if (!plx_type_check(left, return_type)) result = false;
      if (left->type != NULL && !plx_is_int_type(left->type)) {
        plx_unexpected_type(left, /*expected=*/"an integer");
        result = false;
        break;
      }

      // Type check the right operand.
      if (!plx_type_check(right, return_type)) result = false;
      if (right->type != NULL && !plx_is_int_type(right->type)) {
        plx_unexpected_type(right, /*expected=*/"an integer");
        result = false;
        break;
      }

      // Check for matching types.
      if (!plx_type_eq(left->type, right->type)) {
        plx_operand_type_mismatch(node);
        result = false;
        break;
      }

      // Set the type.
      if (left->type != NULL && right->type != NULL) {
        node->type = left->type;
      }
      break;
    }
    case PLX_NODE_NOT: {
      struct plx_node* operand;
      plx_extract_children(node, &operand);

      // Type check the operand.
      if (!plx_type_check(operand, return_type)) result = false;
      if (operand->type != NULL && !plx_is_logical_type(operand->type)) {
        plx_unexpected_type(operand, /*expected=*/"an integer or bool");
        result = false;
        break;
      }

      // Set the type.
      node->type = operand->type;
      break;
    }
    case PLX_NODE_NEG: {
      struct plx_node* operand;
      plx_extract_children(node, &operand);

      // Type check the operand.
      if (!plx_type_check(operand, return_type)) result = false;
      if (operand->type != NULL && !plx_is_numeric_type(operand->type)) {
        plx_unexpected_type(operand->type, /*expected=*/"a number");
        result = false;
        break;
      }

      // Set the type.
      node->type = operand->type;
      break;
    }
    case PLX_NODE_REF: {
      struct plx_node* operand;
      plx_extract_children(node, &operand);

      // Type check the operand.
      if (!plx_type_check(operand, return_type)) result = false;
      if (operand->type == NULL) break;

      // Set the type.
      struct plx_node* const type =
          plx_new_node(PLX_NODE_REF_TYPE, /*loc=*/NULL);
      type->children = plx_copy_node(operand->type);
      node->type = type;
      break;
    }
    case PLX_NODE_DEREF: {
      struct plx_node* operand;
      plx_extract_children(node, &operand);

      // Type check the operand.
      if (!plx_type_check(operand, return_type)) result = false;
      if (operand->type != NULL && operand->type->kind != PLX_NODE_REF_TYPE) {
        plx_unexpected_type(operand, /*expected=*/"a reference");
        result = false;
        break;
      }

      // Set the type.
      node->type = operand->type->children;
      break;
    }
    case PLX_NODE_CALL: {
      struct plx_node *func, *args;
      plx_extract_children(node, &func, &args);

      // Type check the function.
      if (!plx_type_check(func, return_type)) result = false;
      if (func->type != NULL && func->type->kind != PLX_NODE_FUNC_TYPE) {
        plx_unexpected_type(func, /*expected=*/"a function");
        result = false;
        break;
      }

      // Set the type.
      struct plx_node *param_types, *return_type;
      plx_extract_children(func->type, &param_types, &return_type);
      node->type = return_type;

      // Type check the arguments.
      for (struct plx_node* arg = args->children; arg != NULL;
           arg = arg->next) {
        if (!plx_type_check(arg, return_type)) result = false;
      }

      // Check the number of arguments.
      struct plx_node* arg = args->children;
      struct plx_node* param_type = param_types->children;
      while (arg != NULL && param_type != NULL) {
        arg = arg->next;
        param_type = param_type->next;
      }
      if (param_type != NULL) {
        plx_too_few_arguments(node);
        result = false;
        break;
      }
      if (arg != NULL) {
        plx_too_many_arguments(node);
        result = false;
        break;
      }

      // Check that the argument types match the parameter types.
      arg = args->children;
      param_type = param_types->children;
      while (arg != NULL && param_type != NULL) {
        if (arg->type != NULL && !plx_type_eq(arg->type, param_type)) {
          plx_argument_type_mismatch(arg, param_type);
          result = false;
        }
      }
      break;
    }
    case PLX_NODE_INDEX: {
      struct plx_node *value, *index;
      plx_extract_children(node, &value, &index);

      // Type check the value.
      if (!plx_type_check(value, return_type)) result = false;
      if (value->type != NULL) {
        if (value->type->kind == PLX_NODE_ARRAY_TYPE ||
            value->type->kind == PLX_NODE_SLICE_TYPE) {
          // Set the type.
          node->type = value->type->children;
        } else {
          plx_unexpected_type(value, /*expected=*/"an array or slice");
          result = false;
        }
      }

      // Type check the index.
      if (!plx_type_check(index, return_type)) result = false;
      if (index->type != NULL && !plx_is_int_type(index->type)) {
        plx_unexpected_type(index, /*expected=*/"an integer");
        result = false;
      }
      break;
    }
    case PLX_NODE_SLICE: {
      struct plx_node *value, *start, *end;
      plx_extract_children(node, &value, &start, &end);

      // Type check the value.
      if (!plx_type_check(value, return_type)) result = false;
      if (value->type != NULL) {
        if (value->type->kind == PLX_NODE_ARRAY_TYPE ||
            value->type->kind == PLX_NODE_SLICE_TYPE) {
          // Set the type.
          node->type = value->type->children;
        } else {
          plx_unexpected_type(value, /*expected=*/"an array or slice");
          result = false;
        }
      }

      // Type check the start index.
      if (!plx_type_check(start, return_type)) result = false;
      if (start->type != NULL && !plx_is_int_type(start->type)) {
        plx_unexpected_type(start, /*expected=*/"an integer");
        result = false;
      }

      // Type check the end index.
      if (!plx_type_check(end, return_type)) result = false;
      if (end->type != NULL && !plx_is_int_type(end->type)) {
        plx_unexpected_type(end, /*expected=*/"an integer");
        result = false;
      }
      break;
    }
    case PLX_NODE_FIELD: {
      struct plx_node *value, *name;
      plx_extract_children(node, &value, &name);

      if (!plx_type_check(value, return_type)) result = false;
      // TODO
      break;
    }
    case PLX_NODE_IDENTIFIER:
      if (node->entry != NULL) node->type = node->entry->type;
      break;
    case PLX_NODE_S8:
      node->type = &s8_type;
      break;
    case PLX_NODE_S16:
      node->type = &s16_type;
      break;
    case PLX_NODE_S32:
      node->type = &s32_type;
      break;
    case PLX_NODE_S64:
      node->type = &s64_type;
      break;
    case PLX_NODE_U8:
      node->type = &u8_type;
      break;
    case PLX_NODE_U16:
      node->type = &u16_type;
      break;
    case PLX_NODE_U32:
      node->type = &u32_type;
      break;
    case PLX_NODE_U64:
      node->type = &u64_type;
      break;
    case PLX_NODE_F16:
      node->type = &f16_type;
      break;
    case PLX_NODE_F32:
      node->type = &f32_type;
      break;
    case PLX_NODE_F64:
      node->type = &f64_type;
      break;
    case PLX_NODE_BOOL:
      node->type = &bool_type;
      break;
    case PLX_NODE_STRING:
      node->type = &string_type;
      break;
    default:
      for (struct plx_node* child = node->children; child != NULL;
           child = child->next) {
        if (!plx_type_check(child, return_type)) result = false;
      }
  }
  return result;
}
