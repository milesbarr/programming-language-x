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

#include "llvm_ir_generator.h"

#include <assert.h>
#include <stdarg.h>

#include "symbol_table_entry.h"
#include "types.h"

static void plx_generate_llvm_ir_constant(const struct plx_node* const node,
                                          FILE* const stream) {
  switch (node->kind) {
    case PLX_NODE_S8:
    case PLX_NODE_S16:
    case PLX_NODE_S32:
    case PLX_NODE_S64:
      fprintf(stream, "%lld", node->sint);
      break;
    case PLX_NODE_U8:
    case PLX_NODE_U16:
    case PLX_NODE_U32:
    case PLX_NODE_U64:
      fprintf(stream, "%llu", node->uint);
      break;
    case PLX_NODE_F16:
    case PLX_NODE_F32:
    case PLX_NODE_F64:
      fprintf(stream, "%f", node->f);
      break;
    case PLX_NODE_BOOL:
      fputs(node->b ? "true" : "false", stream);
      break;
    default:
      assert(false);
  }
}

static void plx_generate_llvm_ir_type(const struct plx_node* const type,
                                      FILE* const stream) {
  switch (type->kind) {
    case PLX_NODE_VOID_TYPE:
      fputs("void", stream);
      break;
    case PLX_NODE_S8_TYPE:
    case PLX_NODE_U8_TYPE:
      fputs("i8", stream);
      break;
    case PLX_NODE_S16_TYPE:
    case PLX_NODE_U16_TYPE:
      fputs("i16", stream);
      break;
    case PLX_NODE_S32_TYPE:
    case PLX_NODE_U32_TYPE:
      fputs("i32", stream);
      break;
    case PLX_NODE_S64_TYPE:
    case PLX_NODE_U64_TYPE:
      fputs("i64", stream);
      break;
    case PLX_NODE_F16_TYPE:
      fputs("half", stream);
      break;
    case PLX_NODE_F32_TYPE:
      fputs("float", stream);
      break;
    case PLX_NODE_F64_TYPE:
      fputs("double", stream);
      break;
    case PLX_NODE_BOOL_TYPE:
      fputs("i1", stream);
      break;
    case PLX_NODE_STRING_TYPE:
      assert(false);
      break;
    case PLX_NODE_FUNC_TYPE:
    case PLX_NODE_REF_TYPE:
      fputs("ptr", stream);
      break;
    case PLX_NODE_ARRAY_TYPE: {
      const struct plx_node *len, *element_type;
      plx_extract_children(type, &len, &element_type);
      fputc('[', stream);
      plx_generate_llvm_ir_constant(len, stream);
      fputs(" x ", stream);
      plx_generate_llvm_ir_type(element_type, stream);
      fputc(']', stream);
      break;
    }
    case PLX_NODE_SLICE_TYPE:
      fputs("{ i64, ptr }", stream);
      break;
    default:
      assert(false);
  }
}

// Writes the format string to the output stream with custom format specifiers
// for LLVM IR.
static void plx_llvm_ir_fprintf(FILE* const stream, const char* format, ...) {
  va_list arg;
  va_start(arg, format);
  for (; format[0] != '\0'; ++format) {
    if (format[0] != '%') {
      fputc(format[0], stream);
      continue;
    }
    switch (format[1]) {
      // Constant
      case 'c':
        plx_generate_llvm_ir_constant(va_arg(arg, const struct plx_node*),
                                      stream);
        break;
      // String
      case 's':
        fputs(va_arg(arg, const char*), stream);
        break;
      // Type
      case 't':
        plx_generate_llvm_ir_type(va_arg(arg, const struct plx_node*), stream);
        break;
      // Unnamed identifier
      case 'u':
        fprintf(stream, "%u", va_arg(arg, plx_llvm_unnamed_identifier));
        break;
      // Escape
      case '%':
        fputc('%', stream);
        break;
      default:
        assert(false);
    }
    ++format;
  }
  va_end(arg);
}

static plx_llvm_unnamed_identifier plx_generate_llvm_ir_ptr(
    const struct plx_node* const node, FILE* const stream,
    plx_llvm_unnamed_identifier* const locals) {
  switch (node->kind) {
    case PLX_NODE_INDEX: {
      const struct plx_node *value, *index;
      plx_extract_children(node, &value, &index);
      const plx_llvm_unnamed_identifier value_var =
          plx_generate_llvm_ir_expr(value, stream, locals);
      const plx_llvm_unnamed_identifier result_var = (*locals)++;
      plx_llvm_ir_fprintf(
          stream, "  %%%u = getelementptr inbounds %t, ptr %%%u, %t %c\n",
          result_var, value->type, value_var, index->type, index);
      return result_var;
    }
    case PLX_NODE_FIELD:
      // TODO
      assert(false);
      break;
    case PLX_NODE_IDENTIFIER:
      switch (node->entry->scope) {
        case PLX_SYMBOL_SCOPE_LOCAL:
          return node->entry->llvm_local_var;
        case PLX_SYMBOL_SCOPE_GLOBAL: {
          const plx_llvm_unnamed_identifier result_var = (*locals)++;
          plx_llvm_ir_fprintf(stream,
                              "  %%%u = alloca ptr\n"
                              "  store ptr @%s, ptr %%%u\n",
                              result_var, node->name, result_var);
          return result_var;
        }
      }
      break;
    default:
      assert(false);
  }
  return 0;
}

void plx_generate_llvm_ir(const struct plx_node* const node,
                          FILE* const stream) {
  switch (node->kind) {
    case PLX_NODE_MODULE:
      for (const struct plx_node* def = node->children; def != NULL;
           def = def->next) {
        plx_generate_llvm_ir(def, stream);
      }
      break;
    case PLX_NODE_VAR_DEF: {
      const struct plx_node *name, *value;
      plx_extract_children(node, &name, &value);
      plx_llvm_ir_fprintf(stream, "@%s = global %t %c\n", name->name,
                          value->type, value);
      break;
    }
    case PLX_NODE_VAR_DECL: {
      const struct plx_node *name, *type;
      plx_extract_children(node, &name, &type);
      plx_llvm_ir_fprintf(stream, "@%s = global %t\n", name->name, type);
      break;
    }
    case PLX_NODE_STRUCT_DEF: {
      const struct plx_node *name, *members;
      plx_extract_children(node, &name, &members);
      fprintf(stream, "%%%s = type { ", name->name);
      for (const struct plx_node* member = members->children; member != NULL;
           member = member->next) {
        const struct plx_node *member_name, *member_type;
        plx_extract_children(member, &member_name, &member_type);
        plx_generate_llvm_ir_type(member_type, stream);
        if (member->next != NULL) fputs(", ", stream);
      }
      fputs(" }\n\n", stream);
      break;
    }
    case PLX_NODE_FUNC_DEF: {
      const struct plx_node *name, *params, *return_type, *body;
      plx_extract_children(node, &name, &params, &return_type, &body);

      plx_llvm_ir_fprintf(stream, "define %t @%s(", return_type, name->name);
      plx_llvm_unnamed_identifier locals = 0;
      for (const struct plx_node* param = params->children; param != NULL;
           param = param->next) {
        const struct plx_node *param_name, *param_type;
        plx_extract_children(param, &param_name, &param_type);
        plx_llvm_ir_fprintf(stream, "%t %%%u", param_type, locals++);
        if (param->next != NULL) fputs(", ", stream);
      }
      fputs(") {\n", stream);
      ++locals;  // Implicit numbered label.

      plx_llvm_unnamed_identifier param_var = 0;
      for (const struct plx_node* param = params->children; param != NULL;
           param = param->next) {
        const struct plx_node *param_name, *param_type;
        plx_extract_children(param, &param_name, &param_type);
        plx_llvm_ir_fprintf(stream,
                            "  %%%u = alloca %t\n"
                            "  store %t %%%u, ptr %%%u\n",
                            locals, param_type, param_type, param_var++,
                            locals);
        param_name->entry->llvm_local_var = locals++;
      }
      plx_generate_llvm_ir_stmt(body, stream, &locals,
                                /*loop_enter_label=*/0,
                                /*loop_exit_label=*/0);
      fputs("}\n\n", stream);
      break;
    }
    case PLX_NODE_NOP:
      break;
    default:
      assert(false);
  }
}

void plx_generate_llvm_ir_stmt(const struct plx_node* const node,
                               FILE* const stream,
                               plx_llvm_unnamed_identifier* const locals,
                               plx_llvm_unnamed_identifier loop_enter_label,
                               plx_llvm_unnamed_identifier loop_exit_label) {
  switch (node->kind) {
    case PLX_NODE_VAR_DEF: {
      const struct plx_node *name, *value;
      plx_extract_children(node, &name, &value);
      const plx_llvm_unnamed_identifier value_var =
          plx_generate_llvm_ir_expr(value, stream, locals);
      const plx_llvm_unnamed_identifier result_var = (*locals)++;
      plx_llvm_ir_fprintf(stream,
                          "  %%%u = alloca %t\n"
                          "  store %t %%%u, ptr %%%u\n",
                          result_var, value->type, value->type, result_var,
                          value_var);
      name->entry->llvm_local_var = result_var;
      break;
    }
    case PLX_NODE_VAR_DECL: {
      const struct plx_node *name, *type;
      plx_extract_children(node, &name, &type);
      const plx_llvm_unnamed_identifier result_var = (*locals)++;
      plx_llvm_ir_fprintf(stream, "  %%%u = alloca %t\n", result_var, type);
      name->entry->llvm_local_var = result_var;
      break;
    }
    case PLX_NODE_NOP:
      break;
    case PLX_NODE_BLOCK:
      for (const struct plx_node* stmt = node->children; stmt != NULL;
           stmt = stmt->next) {
        plx_generate_llvm_ir_stmt(stmt, stream, locals, loop_enter_label,
                                  loop_exit_label);
      }
      break;
    case PLX_NODE_IF_THEN_ELSE: {
      const struct plx_node *cond, *then, *els;
      plx_extract_children(node, &cond, &then, &els);
      const plx_llvm_unnamed_identifier cond_var =
          plx_generate_llvm_ir_expr(cond, stream, locals);
      const plx_llvm_unnamed_identifier then_label = (*locals)++;
      const plx_llvm_unnamed_identifier else_label = (*locals)++;
      const plx_llvm_unnamed_identifier end_label = (*locals)++;
      fprintf(stream,
              "  br i1 %%%u, label %%%u, label %%%u\n"
              "%u:\n",
              cond_var, then_label, else_label, then_label);
      plx_generate_llvm_ir_stmt(then, stream, locals, loop_enter_label,
                                loop_exit_label);
      fprintf(stream,
              "  br label %%%u\n"
              "%u:\n",
              end_label, else_label);
      plx_generate_llvm_ir_stmt(els, stream, locals, loop_enter_label,
                                loop_exit_label);
      fprintf(stream,
              "  br label %%%u\n"
              "%u:\n",
              end_label, end_label);
      break;
    }
    case PLX_NODE_LOOP: {
      const struct plx_node* body;
      plx_extract_children(node, &body);
      loop_enter_label = (*locals)++;
      loop_exit_label = (*locals)++;
      fprintf(stream, "%u:\n", loop_enter_label);
      plx_generate_llvm_ir_stmt(body, stream, locals, loop_enter_label,
                                loop_exit_label);
      fprintf(stream, "%u:\n", loop_exit_label);
      break;
    }
    case PLX_NODE_WHILE_LOOP: {
      const struct plx_node *cond, *body;
      plx_extract_children(node, &cond, &body);
      loop_enter_label = (*locals)++;
      const plx_llvm_unnamed_identifier loop_mid_label = (*locals)++;
      loop_exit_label = (*locals)++;
      fprintf(stream, "%u:\n", loop_enter_label);
      const plx_llvm_unnamed_identifier cond_var =
          plx_generate_llvm_ir_expr(cond, stream, locals);
      fprintf(stream,
              "  br i1 %%%u, label %%%u, label %%%u\n"
              "%u:\n",
              cond_var, loop_mid_label, loop_exit_label, loop_mid_label);
      plx_generate_llvm_ir_stmt(body, stream, locals, loop_enter_label,
                                loop_exit_label);
      fprintf(stream, "%u:\n", loop_exit_label);
      break;
    }
    case PLX_NODE_CONTINUE:
      fprintf(stream, "  br label %%%u\n", loop_enter_label);
      break;
    case PLX_NODE_BREAK:
      fprintf(stream, "  br label %%%u\n", loop_exit_label);
      break;
    case PLX_NODE_RETURN: {
      const struct plx_node* const return_value = node->children;
      if (return_value == NULL) {
        fputs("  ret void\n", stream);
        break;
      }
      const plx_llvm_unnamed_identifier return_value_var =
          plx_generate_llvm_ir_expr(return_value, stream, locals);
      plx_llvm_ir_fprintf(stream, "  ret %t %%%u\n", return_value->type,
                          return_value_var);
      break;
    }
    case PLX_NODE_ASSIGN: {
      const struct plx_node *assignee, *value;
      plx_extract_children(node, &assignee, &value);
      const plx_llvm_unnamed_identifier assignee_var =
          plx_generate_llvm_ir_ptr(assignee, stream, locals);
      const plx_llvm_unnamed_identifier value_var =
          plx_generate_llvm_ir_expr(value, stream, locals);
      plx_llvm_ir_fprintf(stream, "  store %t %%%u, ptr %%%u\n", value->type,
                          value_var, assignee_var);
      break;
    }
    case PLX_NODE_ADD_ASSIGN: {
      const struct plx_node *assignee, *value;
      plx_extract_children(node, &assignee, &value);
      const plx_llvm_unnamed_identifier assignee_var =
          plx_generate_llvm_ir_ptr(assignee, stream, locals);
      const plx_llvm_unnamed_identifier left_var = (*locals)++;
      const plx_llvm_unnamed_identifier right_var =
          plx_generate_llvm_ir_expr(value, stream, locals);
      const plx_llvm_unnamed_identifier result_var = (*locals)++;
      assert(assignee->type->kind == value->type->kind);
      switch (assignee->type->kind) {
        case PLX_NODE_S8_TYPE:
        case PLX_NODE_U8_TYPE:
          fprintf(stream,
                  "  %%%u = load i8, ptr %%%u\n"
                  "  %%%u = add i8 %%%u, %%%u\n"
                  "  store i8 %%%u, ptr %%%u\n",
                  left_var, assignee_var, result_var, left_var, right_var,
                  result_var, assignee_var);
          break;
        case PLX_NODE_S16_TYPE:
        case PLX_NODE_U16_TYPE:
          fprintf(stream,
                  "  %%%u = load i16, ptr %%%u\n"
                  "  %%%u = add i16 %%%u, %%%u\n"
                  "  store i16 %%%u, ptr %%%u\n",
                  left_var, assignee_var, result_var, left_var, right_var,
                  result_var, assignee_var);
          break;
        case PLX_NODE_S32_TYPE:
        case PLX_NODE_U32_TYPE:
          fprintf(stream,
                  "  %%%u = load i32, ptr %%%u\n"
                  "  %%%u = add i32 %%%u, %%%u\n"
                  "  store i32 %%%u, ptr %%%u\n",
                  left_var, assignee_var, result_var, left_var, right_var,
                  result_var, assignee_var);
          break;
        case PLX_NODE_S64_TYPE:
        case PLX_NODE_U64_TYPE:
          fprintf(stream,
                  "  %%%u = load i64, ptr %%%u\n"
                  "  %%%u = add i64 %%%u, %%%u\n"
                  "  store i64 %%%u, ptr %%%u\n",
                  left_var, assignee_var, result_var, left_var, right_var,
                  result_var, assignee_var);
          break;
        case PLX_NODE_F16_TYPE:
          fprintf(stream,
                  "  %%%u = load half, ptr %%%u\n"
                  "  %%%u = fadd fast half %%%u, %%%u\n"
                  "  store half %%%u, ptr %%%u\n",
                  left_var, assignee_var, result_var, left_var, right_var,
                  result_var, assignee_var);
          break;
        case PLX_NODE_F32_TYPE:
          fprintf(stream,
                  "  %%%u = load float, ptr %%%u\n"
                  "  %%%u = fadd fast float %%%u, %%%u\n"
                  "  store float %%%u, ptr %%%u\n",
                  left_var, assignee_var, result_var, left_var, right_var,
                  result_var, assignee_var);
          break;
        case PLX_NODE_F64_TYPE:
          fprintf(stream,
                  "  %%%u = load double, ptr %%%u\n"
                  "  %%%u = fadd fast double %%%u, %%%u\n"
                  "  store double %%%u, ptr %%%u\n",
                  left_var, assignee_var, result_var, left_var, right_var,
                  result_var, assignee_var);
          break;
        default:
          assert(false);
      }
      break;
    }
    case PLX_NODE_SUB_ASSIGN: {
      const struct plx_node *assignee, *value;
      plx_extract_children(node, &assignee, &value);
      const plx_llvm_unnamed_identifier assignee_var =
          plx_generate_llvm_ir_ptr(assignee, stream, locals);
      const plx_llvm_unnamed_identifier left_var = (*locals)++;
      const plx_llvm_unnamed_identifier right_var =
          plx_generate_llvm_ir_expr(value, stream, locals);
      const plx_llvm_unnamed_identifier result_var = (*locals)++;
      assert(assignee->type->kind == value->type->kind);
      switch (assignee->type->kind) {
        case PLX_NODE_S8_TYPE:
        case PLX_NODE_U8_TYPE:
          fprintf(stream,
                  "  %%%u = load i8, ptr %%%u\n"
                  "  %%%u = sub i8 %%%u, %%%u\n"
                  "  store i8 %%%u, ptr %%%u\n",
                  left_var, assignee_var, result_var, left_var, right_var,
                  result_var, assignee_var);
          break;
        case PLX_NODE_S16_TYPE:
        case PLX_NODE_U16_TYPE:
          fprintf(stream,
                  "  %%%u = load i16, ptr %%%u\n"
                  "  %%%u = sub i16 %%%u, %%%u\n"
                  "  store i16 %%%u, ptr %%%u\n",
                  left_var, assignee_var, result_var, left_var, right_var,
                  result_var, assignee_var);
          break;
        case PLX_NODE_S32_TYPE:
        case PLX_NODE_U32_TYPE:
          fprintf(stream,
                  "  %%%u = load i32, ptr %%%u\n"
                  "  %%%u = sub i32 %%%u, %%%u\n"
                  "  store i32 %%%u, ptr %%%u\n",
                  left_var, assignee_var, result_var, left_var, right_var,
                  result_var, assignee_var);
          break;
        case PLX_NODE_S64_TYPE:
        case PLX_NODE_U64_TYPE:
          fprintf(stream,
                  "  %%%u = load i64, ptr %%%u\n"
                  "  %%%u = sub i64 %%%u, %%%u\n"
                  "  store i64 %%%u, ptr %%%u\n",
                  left_var, assignee_var, result_var, left_var, right_var,
                  result_var, assignee_var);
          break;
        case PLX_NODE_F16_TYPE:
          fprintf(stream,
                  "  %%%u = load half, ptr %%%u\n"
                  "  %%%u = fsub fast half %%%u, %%%u\n"
                  "  store half %%%u, ptr %%%u\n",
                  left_var, assignee_var, result_var, left_var, right_var,
                  result_var, assignee_var);
          break;
        case PLX_NODE_F32_TYPE:
          fprintf(stream,
                  "  %%%u = load float, ptr %%%u\n"
                  "  %%%u = fsub fast float %%%u, %%%u\n"
                  "  store float %%%u, ptr %%%u\n",
                  left_var, assignee_var, result_var, left_var, right_var,
                  result_var, assignee_var);
          break;
        case PLX_NODE_F64_TYPE:
          fprintf(stream,
                  "  %%%u = load double, ptr %%%u\n"
                  "  %%%u = fsub fast double %%%u, %%%u\n"
                  "  store double %%%u, ptr %%%u\n",
                  left_var, assignee_var, result_var, left_var, right_var,
                  result_var, assignee_var);
          break;
        default:
          assert(false);
      }
      break;
    }
    case PLX_NODE_MUL_ASSIGN: {
      const struct plx_node *assignee, *value;
      plx_extract_children(node, &assignee, &value);
      const plx_llvm_unnamed_identifier assignee_var =
          plx_generate_llvm_ir_ptr(assignee, stream, locals);
      const plx_llvm_unnamed_identifier left_var = (*locals)++;
      const plx_llvm_unnamed_identifier right_var =
          plx_generate_llvm_ir_expr(value, stream, locals);
      const plx_llvm_unnamed_identifier result_var = (*locals)++;
      assert(assignee->type->kind == value->type->kind);
      switch (assignee->type->kind) {
        case PLX_NODE_S8_TYPE:
        case PLX_NODE_U8_TYPE:
          fprintf(stream,
                  "  %%%u = load i8, ptr %%%u\n"
                  "  %%%u = mul i8 %%%u, %%%u\n"
                  "  store i8 %%%u, ptr %%%u\n",
                  left_var, assignee_var, result_var, left_var, right_var,
                  result_var, assignee_var);
          break;
        case PLX_NODE_S16_TYPE:
        case PLX_NODE_U16_TYPE:
          fprintf(stream,
                  "  %%%u = load i16, ptr %%%u\n"
                  "  %%%u = mul i16 %%%u, %%%u\n"
                  "  store i16 %%%u, ptr %%%u\n",
                  left_var, assignee_var, result_var, left_var, right_var,
                  result_var, assignee_var);
          break;
        case PLX_NODE_S32_TYPE:
        case PLX_NODE_U32_TYPE:
          fprintf(stream,
                  "  %%%u = load i32, ptr %%%u\n"
                  "  %%%u = mul i32 %%%u, %%%u\n"
                  "  store i32 %%%u, ptr %%%u\n",
                  left_var, assignee_var, result_var, left_var, right_var,
                  result_var, assignee_var);
          break;
        case PLX_NODE_S64_TYPE:
        case PLX_NODE_U64_TYPE:
          fprintf(stream,
                  "  %%%u = load i64, ptr %%%u\n"
                  "  %%%u = mul i64 %%%u, %%%u\n"
                  "  store i64 %%%u, ptr %%%u\n",
                  left_var, assignee_var, result_var, left_var, right_var,
                  result_var, assignee_var);
          break;
        case PLX_NODE_F16_TYPE:
          fprintf(stream,
                  "  %%%u = load half, ptr %%%u\n"
                  "  %%%u = fmul fast half %%%u, %%%u\n"
                  "  store half %%%u, ptr %%%u\n",
                  left_var, assignee_var, result_var, left_var, right_var,
                  result_var, assignee_var);
          break;
        case PLX_NODE_F32_TYPE:
          fprintf(stream,
                  "  %%%u = load float, ptr %%%u\n"
                  "  %%%u = fmul fast float %%%u, %%%u\n"
                  "  store float %%%u, ptr %%%u\n",
                  left_var, assignee_var, result_var, left_var, right_var,
                  result_var, assignee_var);
          break;
        case PLX_NODE_F64_TYPE:
          fprintf(stream,
                  "  %%%u = load double, ptr %%%u\n"
                  "  %%%u = fmul fast double %%%u, %%%u\n"
                  "  store double %%%u, ptr %%%u\n",
                  left_var, assignee_var, result_var, left_var, right_var,
                  result_var, assignee_var);
          break;
        default:
          assert(false);
      }
      break;
    }
    case PLX_NODE_DIV_ASSIGN: {
      const struct plx_node *assignee, *value;
      plx_extract_children(node, &assignee, &value);
      const plx_llvm_unnamed_identifier assignee_var =
          plx_generate_llvm_ir_ptr(assignee, stream, locals);
      const plx_llvm_unnamed_identifier left_var = (*locals)++;
      const plx_llvm_unnamed_identifier right_var =
          plx_generate_llvm_ir_expr(value, stream, locals);
      const plx_llvm_unnamed_identifier result_var = (*locals)++;
      assert(assignee->type->kind == value->type->kind);
      switch (assignee->type->kind) {
        case PLX_NODE_S8_TYPE:
          fprintf(stream,
                  "  %%%u = load i8, ptr %%%u\n"
                  "  %%%u = sdiv i8 %%%u, %%%u\n"
                  "  store i8 %%%u, ptr %%%u\n",
                  left_var, assignee_var, result_var, left_var, right_var,
                  result_var, assignee_var);
          break;
        case PLX_NODE_S16_TYPE:
          fprintf(stream,
                  "  %%%u = load i16, ptr %%%u\n"
                  "  %%%u = sdiv i16 %%%u, %%%u\n"
                  "  store i16 %%%u, ptr %%%u\n",
                  left_var, assignee_var, result_var, left_var, right_var,
                  result_var, assignee_var);
          break;
        case PLX_NODE_S32_TYPE:
          fprintf(stream,
                  "  %%%u = load i32, ptr %%%u\n"
                  "  %%%u = sdiv i32 %%%u, %%%u\n"
                  "  store i32 %%%u, ptr %%%u\n",
                  left_var, assignee_var, result_var, left_var, right_var,
                  result_var, assignee_var);
          break;
        case PLX_NODE_S64_TYPE:
          fprintf(stream,
                  "  %%%u = load i64, ptr %%%u\n"
                  "  %%%u = sdiv i64 %%%u, %%%u\n"
                  "  store i64 %%%u, ptr %%%u\n",
                  left_var, assignee_var, result_var, left_var, right_var,
                  result_var, assignee_var);
          break;
        case PLX_NODE_U8_TYPE:
          fprintf(stream,
                  "  %%%u = load i8, ptr %%%u\n"
                  "  %%%u = udiv i8 %%%u, %%%u\n"
                  "  store i8 %%%u, ptr %%%u\n",
                  left_var, assignee_var, result_var, left_var, right_var,
                  result_var, assignee_var);
          break;
        case PLX_NODE_U16_TYPE:
          fprintf(stream,
                  "  %%%u = load i16, ptr %%%u\n"
                  "  %%%u = udiv i16 %%%u, %%%u\n"
                  "  store i16 %%%u, ptr %%%u\n",
                  left_var, assignee_var, result_var, left_var, right_var,
                  result_var, assignee_var);
          break;
        case PLX_NODE_U32_TYPE:
          fprintf(stream,
                  "  %%%u = load i32, ptr %%%u\n"
                  "  %%%u = udiv i32 %%%u, %%%u\n"
                  "  store i32 %%%u, ptr %%%u\n",
                  left_var, assignee_var, result_var, left_var, right_var,
                  result_var, assignee_var);
          break;
        case PLX_NODE_U64_TYPE:
          fprintf(stream,
                  "  %%%u = load i64, ptr %%%u\n"
                  "  %%%u = udiv i64 %%%u, %%%u\n"
                  "  store i64 %%%u, ptr %%%u\n",
                  left_var, assignee_var, result_var, left_var, right_var,
                  result_var, assignee_var);
          break;
        case PLX_NODE_F16_TYPE:
          fprintf(stream,
                  "  %%%u = load half, ptr %%%u\n"
                  "  %%%u = fdiv fast half %%%u, %%%u\n"
                  "  store half %%%u, ptr %%%u\n",
                  left_var, assignee_var, result_var, left_var, right_var,
                  result_var, assignee_var);
          break;
        case PLX_NODE_F32_TYPE:
          fprintf(stream,
                  "  %%%u = load float, ptr %%%u\n"
                  "  %%%u = fdiv fast float %%%u, %%%u\n"
                  "  store float %%%u, ptr %%%u\n",
                  left_var, assignee_var, result_var, left_var, right_var,
                  result_var, assignee_var);
          break;
        case PLX_NODE_F64_TYPE:
          fprintf(stream,
                  "  %%%u = load double, ptr %%%u\n"
                  "  %%%u = fdiv fast double %%%u, %%%u\n"
                  "  store double %%%u, ptr %%%u\n",
                  left_var, assignee_var, result_var, left_var, right_var,
                  result_var, assignee_var);
          break;
        default:
          assert(false);
      }
      break;
    }
    case PLX_NODE_REM_ASSIGN: {
      const struct plx_node *assignee, *value;
      plx_extract_children(node, &assignee, &value);
      const plx_llvm_unnamed_identifier assignee_var =
          plx_generate_llvm_ir_ptr(assignee, stream, locals);
      const plx_llvm_unnamed_identifier left_var = (*locals)++;
      const plx_llvm_unnamed_identifier right_var =
          plx_generate_llvm_ir_expr(value, stream, locals);
      const plx_llvm_unnamed_identifier result_var = (*locals)++;
      assert(assignee->type->kind == value->type->kind);
      switch (assignee->type->kind) {
        case PLX_NODE_S8_TYPE:
          fprintf(stream,
                  "  %%%u = load i8, ptr %%%u\n"
                  "  %%%u = srem i8 %%%u, %%%u\n"
                  "  store i8 %%%u, ptr %%%u\n",
                  left_var, assignee_var, result_var, left_var, right_var,
                  result_var, assignee_var);
          break;
        case PLX_NODE_S16_TYPE:
          fprintf(stream,
                  "  %%%u = load i16, ptr %%%u\n"
                  "  %%%u = srem i16 %%%u, %%%u\n"
                  "  store i16 %%%u, ptr %%%u\n",
                  left_var, assignee_var, result_var, left_var, right_var,
                  result_var, assignee_var);
          break;
        case PLX_NODE_S32_TYPE:
          fprintf(stream,
                  "  %%%u = load i32, ptr %%%u\n"
                  "  %%%u = srem i32 %%%u, %%%u\n"
                  "  store i32 %%%u, ptr %%%u\n",
                  left_var, assignee_var, result_var, left_var, right_var,
                  result_var, assignee_var);
          break;
        case PLX_NODE_S64_TYPE:
          fprintf(stream,
                  "  %%%u = load i64, ptr %%%u\n"
                  "  %%%u = srem i64 %%%u, %%%u\n"
                  "  store i64 %%%u, ptr %%%u\n",
                  left_var, assignee_var, result_var, left_var, right_var,
                  result_var, assignee_var);
          break;
        case PLX_NODE_U8_TYPE:
          fprintf(stream,
                  "  %%%u = load i8, ptr %%%u\n"
                  "  %%%u = urem i8 %%%u, %%%u\n"
                  "  store i8 %%%u, ptr %%%u\n",
                  left_var, assignee_var, result_var, left_var, right_var,
                  result_var, assignee_var);
          break;
        case PLX_NODE_U16_TYPE:
          fprintf(stream,
                  "  %%%u = load i16, ptr %%%u\n"
                  "  %%%u = urem i16 %%%u, %%%u\n"
                  "  store i16 %%%u, ptr %%%u\n",
                  left_var, assignee_var, result_var, left_var, right_var,
                  result_var, assignee_var);
          break;
        case PLX_NODE_U32_TYPE:
          fprintf(stream,
                  "  %%%u = load i32, ptr %%%u\n"
                  "  %%%u = urem i32 %%%u, %%%u\n"
                  "  store i32 %%%u, ptr %%%u\n",
                  left_var, assignee_var, result_var, left_var, right_var,
                  result_var, assignee_var);
          break;
        case PLX_NODE_U64_TYPE:
          fprintf(stream,
                  "  %%%u = load i64, ptr %%%u\n"
                  "  %%%u = urem i64 %%%u, %%%u\n"
                  "  store i64 %%%u, ptr %%%u\n",
                  left_var, assignee_var, result_var, left_var, right_var,
                  result_var, assignee_var);
          break;
        default:
          assert(false);
      }
      break;
    }
    case PLX_NODE_LSHIFT_ASSIGN: {
      const struct plx_node *assignee, *value;
      plx_extract_children(node, &assignee, &value);
      const plx_llvm_unnamed_identifier assignee_var =
          plx_generate_llvm_ir_ptr(assignee, stream, locals);
      const plx_llvm_unnamed_identifier left_var = (*locals)++;
      const plx_llvm_unnamed_identifier right_var =
          plx_generate_llvm_ir_expr(value, stream, locals);
      const plx_llvm_unnamed_identifier result_var = (*locals)++;
      assert(assignee->type->kind == value->type->kind);
      switch (assignee->type->kind) {
        case PLX_NODE_S8_TYPE:
        case PLX_NODE_U8_TYPE:
          fprintf(stream,
                  "  %%%u = load i8, ptr %%%u\n"
                  "  %%%u = shl i8 %%%u, %%%u\n"
                  "  store i8 %%%u, ptr %%%u\n",
                  left_var, assignee_var, result_var, left_var, right_var,
                  result_var, assignee_var);
          break;
        case PLX_NODE_S16_TYPE:
        case PLX_NODE_U16_TYPE:
          fprintf(stream,
                  "  %%%u = load i16, ptr %%%u\n"
                  "  %%%u = shl i16 %%%u, %%%u\n"
                  "  store i16 %%%u, ptr %%%u\n",
                  left_var, assignee_var, result_var, left_var, right_var,
                  result_var, assignee_var);
          break;
        case PLX_NODE_S32_TYPE:
        case PLX_NODE_U32_TYPE:
          fprintf(stream,
                  "  %%%u = load i32, ptr %%%u\n"
                  "  %%%u = shl i32 %%%u, %%%u\n"
                  "  store i32 %%%u, ptr %%%u\n",
                  left_var, assignee_var, result_var, left_var, right_var,
                  result_var, assignee_var);
          break;
        case PLX_NODE_S64_TYPE:
        case PLX_NODE_U64_TYPE:
          fprintf(stream,
                  "  %%%u = load i64, ptr %%%u\n"
                  "  %%%u = shl i64 %%%u, %%%u\n"
                  "  store i64 %%%u, ptr %%%u\n",
                  left_var, assignee_var, result_var, left_var, right_var,
                  result_var, assignee_var);
          break;
        default:
          assert(false);
      }
      break;
    }
    case PLX_NODE_RSHIFT_ASSIGN: {
      const struct plx_node *assignee, *value;
      plx_extract_children(node, &assignee, &value);
      const plx_llvm_unnamed_identifier assignee_var =
          plx_generate_llvm_ir_ptr(assignee, stream, locals);
      const plx_llvm_unnamed_identifier left_var = (*locals)++;
      const plx_llvm_unnamed_identifier right_var =
          plx_generate_llvm_ir_expr(value, stream, locals);
      const plx_llvm_unnamed_identifier result_var = (*locals)++;
      assert(assignee->type->kind == value->type->kind);
      switch (assignee->type->kind) {
        case PLX_NODE_S8_TYPE:
        case PLX_NODE_U8_TYPE:
          fprintf(stream,
                  "  %%%u = load i8, ptr %%%u\n"
                  "  %%%u = lshr i8 %%%u, %%%u\n"
                  "  store i8 %%%u, ptr %%%u\n",
                  left_var, assignee_var, result_var, left_var, right_var,
                  result_var, assignee_var);
          break;
        case PLX_NODE_S16_TYPE:
        case PLX_NODE_U16_TYPE:
          fprintf(stream,
                  "  %%%u = load i16, ptr %%%u\n"
                  "  %%%u = lshr i16 %%%u, %%%u\n"
                  "  store i16 %%%u, ptr %%%u\n",
                  left_var, assignee_var, result_var, left_var, right_var,
                  result_var, assignee_var);
          break;
        case PLX_NODE_S32_TYPE:
        case PLX_NODE_U32_TYPE:
          fprintf(stream,
                  "  %%%u = load i32, ptr %%%u\n"
                  "  %%%u = lshr i32 %%%u, %%%u\n"
                  "  store i32 %%%u, ptr %%%u\n",
                  left_var, assignee_var, result_var, left_var, right_var,
                  result_var, assignee_var);
          break;
        case PLX_NODE_S64_TYPE:
        case PLX_NODE_U64_TYPE:
          fprintf(stream,
                  "  %%%u = load i64, ptr %%%u\n"
                  "  %%%u = lshr i64 %%%u, %%%u\n"
                  "  store i64 %%%u, ptr %%%u\n",
                  left_var, assignee_var, result_var, left_var, right_var,
                  result_var, assignee_var);
          break;
        default:
          assert(false);
      }
      break;
    }
    default: {
      assert(false);
    }
  }
}

plx_llvm_unnamed_identifier plx_generate_llvm_ir_expr(
    const struct plx_node* const node, FILE* const stream,
    plx_llvm_unnamed_identifier* const locals) {
  switch (node->kind) {
    case PLX_NODE_AND: {
      const struct plx_node *left, *right;
      plx_extract_children(node, &left, &right);
      const plx_llvm_unnamed_identifier left_var =
          plx_generate_llvm_ir_expr(left, stream, locals);
      const plx_llvm_unnamed_identifier right_var =
          plx_generate_llvm_ir_expr(right, stream, locals);
      const plx_llvm_unnamed_identifier result_var = (*locals)++;
      switch (node->type->kind) {
        case PLX_NODE_S8_TYPE:
        case PLX_NODE_U8_TYPE:
          fprintf(stream, "  %%%u = and i8 %%%u, %%%u\n", result_var, left_var,
                  right_var);
          break;
        case PLX_NODE_S16_TYPE:
        case PLX_NODE_U16_TYPE:
          fprintf(stream, "  %%%u = and i16 %%%u, %%%u\n", result_var, left_var,
                  right_var);
          break;
        case PLX_NODE_S32_TYPE:
        case PLX_NODE_U32_TYPE:
          fprintf(stream, "  %%%u = and i32 %%%u, %%%u\n", result_var, left_var,
                  right_var);
          break;
        case PLX_NODE_S64_TYPE:
        case PLX_NODE_U64_TYPE:
          fprintf(stream, "  %%%u = and i64 %%%u, %%%u\n", result_var, left_var,
                  right_var);
          break;
        case PLX_NODE_BOOL_TYPE:
          fprintf(stream, "  %%%u = and i1 %%%u, %%%u\n", result_var, left_var,
                  right_var);
          break;
        default:
          assert(false);
      }
      return result_var;
    }
    case PLX_NODE_OR: {
      const struct plx_node *left, *right;
      plx_extract_children(node, &left, &right);
      const plx_llvm_unnamed_identifier left_var =
          plx_generate_llvm_ir_expr(left, stream, locals);
      const plx_llvm_unnamed_identifier right_var =
          plx_generate_llvm_ir_expr(right, stream, locals);
      const plx_llvm_unnamed_identifier result_var = (*locals)++;
      switch (node->type->kind) {
        case PLX_NODE_S8_TYPE:
        case PLX_NODE_U8_TYPE:
          fprintf(stream, "  %%%u = or i8 %%%u, %%%u\n", result_var, left_var,
                  right_var);
          break;
        case PLX_NODE_S16_TYPE:
        case PLX_NODE_U16_TYPE:
          fprintf(stream, "  %%%u = or i16 %%%u, %%%u\n", result_var, left_var,
                  right_var);
          break;
        case PLX_NODE_S32_TYPE:
        case PLX_NODE_U32_TYPE:
          fprintf(stream, "  %%%u = or i32 %%%u, %%%u\n", result_var, left_var,
                  right_var);
          break;
        case PLX_NODE_S64_TYPE:
        case PLX_NODE_U64_TYPE:
          fprintf(stream, "  %%%u = or i64 %%%u, %%%u\n", result_var, left_var,
                  right_var);
          break;
        case PLX_NODE_BOOL_TYPE:
          fprintf(stream, "  %%%u = or i1 %%%u, %%%u\n", result_var, left_var,
                  right_var);
          break;
        default:
          assert(false);
      }
      return result_var;
    }
    case PLX_NODE_XOR: {
      const struct plx_node *left, *right;
      plx_extract_children(node, &left, &right);
      const plx_llvm_unnamed_identifier left_var =
          plx_generate_llvm_ir_expr(left, stream, locals);
      const plx_llvm_unnamed_identifier right_var =
          plx_generate_llvm_ir_expr(right, stream, locals);
      const plx_llvm_unnamed_identifier result_var = (*locals)++;
      switch (node->type->kind) {
        case PLX_NODE_S8_TYPE:
        case PLX_NODE_U8_TYPE:
          fprintf(stream, "  %%%u = xor i8 %%%u, %%%u\n", result_var, left_var,
                  right_var);
          break;
        case PLX_NODE_S16_TYPE:
        case PLX_NODE_U16_TYPE:
          fprintf(stream, "  %%%u = xor i16 %%%u, %%%u\n", result_var, left_var,
                  right_var);
          break;
        case PLX_NODE_S32_TYPE:
        case PLX_NODE_U32_TYPE:
          fprintf(stream, "  %%%u = xor i32 %%%u, %%%u\n", result_var, left_var,
                  right_var);
          break;
        case PLX_NODE_S64_TYPE:
        case PLX_NODE_U64_TYPE:
          fprintf(stream, "  %%%u = xor i64 %%%u, %%%u\n", result_var, left_var,
                  right_var);
          break;
        case PLX_NODE_BOOL_TYPE:
          fprintf(stream, "  %%%u = xor i1 %%%u, %%%u\n", result_var, left_var,
                  right_var);
          break;
        default:
          assert(false);
      }
      return result_var;
    }
    case PLX_NODE_EQ: {
      const struct plx_node *left, *right;
      plx_extract_children(node, &left, &right);
      const plx_llvm_unnamed_identifier left_var =
          plx_generate_llvm_ir_expr(left, stream, locals);
      const plx_llvm_unnamed_identifier right_var =
          plx_generate_llvm_ir_expr(right, stream, locals);
      const plx_llvm_unnamed_identifier result_var = (*locals)++;
      assert(left->type->kind == right->type->kind);
      switch (left->type->kind) {
        case PLX_NODE_S8_TYPE:
        case PLX_NODE_U8_TYPE:
          fprintf(stream, "  %%%u = icmp eq i8 %%%u, %%%u\n", result_var,
                  left_var, right_var);
          break;
        case PLX_NODE_S16_TYPE:
        case PLX_NODE_U16_TYPE:
          fprintf(stream, "  %%%u = icmp eq i16 %%%u, %%%u\n", result_var,
                  left_var, right_var);
          break;
        case PLX_NODE_S32_TYPE:
        case PLX_NODE_U32_TYPE:
          fprintf(stream, "  %%%u = icmp eq i32 %%%u, %%%u\n", result_var,
                  left_var, right_var);
          break;
        case PLX_NODE_S64_TYPE:
        case PLX_NODE_U64_TYPE:
          fprintf(stream, "  %%%u = icmp eq i64 %%%u, %%%u\n", result_var,
                  left_var, right_var);
          break;
        case PLX_NODE_F16_TYPE:
          fprintf(stream, "  %%%u = fcmp oeq half %%%u, %%%u\n", result_var,
                  left_var, right_var);
          break;
        case PLX_NODE_F32_TYPE:
          fprintf(stream, "  %%%u = fcmp oeq float %%%u, %%%u\n", result_var,
                  left_var, right_var);
          break;
        case PLX_NODE_F64_TYPE:
          fprintf(stream, "  %%%u = fcmp oeq double %%%u, %%%u\n", result_var,
                  left_var, right_var);
          break;
        case PLX_NODE_BOOL_TYPE:
          fprintf(stream, "  %%%u = icmp eq i1 %%%u, %%%u\n", result_var,
                  left_var, right_var);
          break;
        default:
          assert(false);
      }
      return result_var;
    }
    case PLX_NODE_NEQ: {
      const struct plx_node *left, *right;
      plx_extract_children(node, &left, &right);
      const plx_llvm_unnamed_identifier left_var =
          plx_generate_llvm_ir_expr(left, stream, locals);
      const plx_llvm_unnamed_identifier right_var =
          plx_generate_llvm_ir_expr(right, stream, locals);
      const plx_llvm_unnamed_identifier result_var = (*locals)++;
      assert(left->type->kind == right->type->kind);
      switch (left->type->kind) {
        case PLX_NODE_S8_TYPE:
        case PLX_NODE_U8_TYPE:
          fprintf(stream, "  %%%u = icmp ne i8 %%%u, %%%u\n", result_var,
                  left_var, right_var);
          break;
        case PLX_NODE_S16_TYPE:
        case PLX_NODE_U16_TYPE:
          fprintf(stream, "  %%%u = icmp ne i16 %%%u, %%%u\n", result_var,
                  left_var, right_var);
          break;
        case PLX_NODE_S32_TYPE:
        case PLX_NODE_U32_TYPE:
          fprintf(stream, "  %%%u = icmp ne i32 %%%u, %%%u\n", result_var,
                  left_var, right_var);
          break;
        case PLX_NODE_S64_TYPE:
        case PLX_NODE_U64_TYPE:
          fprintf(stream, "  %%%u = icmp ne i64 %%%u, %%%u\n", result_var,
                  left_var, right_var);
          break;
        case PLX_NODE_F16_TYPE:
          fprintf(stream, "  %%%u = fcmp one half %%%u, %%%u\n", result_var,
                  left_var, right_var);
          break;
        case PLX_NODE_F32_TYPE:
          fprintf(stream, "  %%%u = fcmp one float %%%u, %%%u\n", result_var,
                  left_var, right_var);
          break;
        case PLX_NODE_F64_TYPE:
          fprintf(stream, "  %%%u = fcmp one double %%%u, %%%u\n", result_var,
                  left_var, right_var);
          break;
        case PLX_NODE_BOOL_TYPE:
          fprintf(stream, "  %%%u = icmp ne i1 %%%u, %%%u\n", result_var,
                  left_var, right_var);
          break;
        default:
          assert(false);
      }
      return result_var;
    }
    case PLX_NODE_LTE: {
      const struct plx_node *left, *right;
      plx_extract_children(node, &left, &right);
      const plx_llvm_unnamed_identifier left_var =
          plx_generate_llvm_ir_expr(left, stream, locals);
      const plx_llvm_unnamed_identifier right_var =
          plx_generate_llvm_ir_expr(right, stream, locals);
      const plx_llvm_unnamed_identifier result_var = (*locals)++;
      assert(left->type->kind == right->type->kind);
      switch (left->type->kind) {
        case PLX_NODE_S8_TYPE:
          fprintf(stream, "  %%%u = icmp sle i8 %%%u, %%%u\n", result_var,
                  left_var, right_var);
          break;
        case PLX_NODE_S16_TYPE:
          fprintf(stream, "  %%%u = icmp sle i16 %%%u, %%%u\n", result_var,
                  left_var, right_var);
          break;
        case PLX_NODE_S32_TYPE:
          fprintf(stream, "  %%%u = icmp sle i32 %%%u, %%%u\n", result_var,
                  left_var, right_var);
          break;
        case PLX_NODE_S64_TYPE:
          fprintf(stream, "  %%%u = icmp sle i64 %%%u, %%%u\n", result_var,
                  left_var, right_var);
          break;
        case PLX_NODE_U8_TYPE:
          fprintf(stream, "  %%%u = icmp ule i8 %%%u, %%%u\n", result_var,
                  left_var, right_var);
          break;
        case PLX_NODE_U16_TYPE:
          fprintf(stream, "  %%%u = icmp ule i16 %%%u, %%%u\n", result_var,
                  left_var, right_var);
          break;
        case PLX_NODE_U32_TYPE:
          fprintf(stream, "  %%%u = icmp ule i32 %%%u, %%%u\n", result_var,
                  left_var, right_var);
          break;
        case PLX_NODE_U64_TYPE:
          fprintf(stream, "  %%%u = icmp ule i64 %%%u, %%%u\n", result_var,
                  left_var, right_var);
          break;
        case PLX_NODE_F16_TYPE:
          fprintf(stream, "  %%%u = fcmp ole half %%%u, %%%u\n", result_var,
                  left_var, right_var);
          break;
        case PLX_NODE_F32_TYPE:
          fprintf(stream, "  %%%u = fcmp ole float %%%u, %%%u\n", result_var,
                  left_var, right_var);
          break;
        case PLX_NODE_F64_TYPE:
          fprintf(stream, "  %%%u = fcmp ole double %%%u, %%%u\n", result_var,
                  left_var, right_var);
          break;
        default:
          assert(false);
      }
      return result_var;
    }
    case PLX_NODE_LT: {
      const struct plx_node *left, *right;
      plx_extract_children(node, &left, &right);
      const plx_llvm_unnamed_identifier left_var =
          plx_generate_llvm_ir_expr(left, stream, locals);
      const plx_llvm_unnamed_identifier right_var =
          plx_generate_llvm_ir_expr(right, stream, locals);
      const plx_llvm_unnamed_identifier result_var = (*locals)++;
      assert(left->type->kind == right->type->kind);
      switch (left->type->kind) {
        case PLX_NODE_S8_TYPE:
          fprintf(stream, "  %%%u = icmp slt i8 %%%u, %%%u\n", result_var,
                  left_var, right_var);
          break;
        case PLX_NODE_S16_TYPE:
          fprintf(stream, "  %%%u = icmp slt i16 %%%u, %%%u\n", result_var,
                  left_var, right_var);
          break;
        case PLX_NODE_S32_TYPE:
          fprintf(stream, "  %%%u = icmp slt i32 %%%u, %%%u\n", result_var,
                  left_var, right_var);
          break;
        case PLX_NODE_S64_TYPE:
          fprintf(stream, "  %%%u = icmp slt i64 %%%u, %%%u\n", result_var,
                  left_var, right_var);
          break;
        case PLX_NODE_U8_TYPE:
          fprintf(stream, "  %%%u = icmp ult i8 %%%u, %%%u\n", result_var,
                  left_var, right_var);
          break;
        case PLX_NODE_U16_TYPE:
          fprintf(stream, "  %%%u = icmp ult i16 %%%u, %%%u\n", result_var,
                  left_var, right_var);
          break;
        case PLX_NODE_U32_TYPE:
          fprintf(stream, "  %%%u = icmp ult i32 %%%u, %%%u\n", result_var,
                  left_var, right_var);
          break;
        case PLX_NODE_U64_TYPE:
          fprintf(stream, "  %%%u = icmp ult i64 %%%u, %%%u\n", result_var,
                  left_var, right_var);
          break;
        case PLX_NODE_F16_TYPE:
          fprintf(stream, "  %%%u = fcmp olt half %%%u, %%%u\n", result_var,
                  left_var, right_var);
          break;
        case PLX_NODE_F32_TYPE:
          fprintf(stream, "  %%%u = fcmp olt float %%%u, %%%u\n", result_var,
                  left_var, right_var);
          break;
        case PLX_NODE_F64_TYPE:
          fprintf(stream, "  %%%u = fcmp olt double %%%u, %%%u\n", result_var,
                  left_var, right_var);
          break;
        default:
          assert(false);
      }
      return result_var;
    }
    case PLX_NODE_GTE: {
      const struct plx_node *left, *right;
      plx_extract_children(node, &left, &right);
      const plx_llvm_unnamed_identifier left_var =
          plx_generate_llvm_ir_expr(left, stream, locals);
      const plx_llvm_unnamed_identifier right_var =
          plx_generate_llvm_ir_expr(right, stream, locals);
      const plx_llvm_unnamed_identifier result_var = (*locals)++;
      assert(left->type->kind == right->type->kind);
      switch (left->type->kind) {
        case PLX_NODE_S8_TYPE:
          fprintf(stream, "  %%%u = icmp sge i8 %%%u, %%%u\n", result_var,
                  left_var, right_var);
          break;
        case PLX_NODE_S16_TYPE:
          fprintf(stream, "  %%%u = icmp sge i16 %%%u, %%%u\n", result_var,
                  left_var, right_var);
          break;
        case PLX_NODE_S32_TYPE:
          fprintf(stream, "  %%%u = icmp sge i32 %%%u, %%%u\n", result_var,
                  left_var, right_var);
          break;
        case PLX_NODE_S64_TYPE:
          fprintf(stream, "  %%%u = icmp sge i64 %%%u, %%%u\n", result_var,
                  left_var, right_var);
          break;
        case PLX_NODE_U8_TYPE:
          fprintf(stream, "  %%%u = icmp uge i8 %%%u, %%%u\n", result_var,
                  left_var, right_var);
          break;
        case PLX_NODE_U16_TYPE:
          fprintf(stream, "  %%%u = icmp uge i16 %%%u, %%%u\n", result_var,
                  left_var, right_var);
          break;
        case PLX_NODE_U32_TYPE:
          fprintf(stream, "  %%%u = icmp uge i32 %%%u, %%%u\n", result_var,
                  left_var, right_var);
          break;
        case PLX_NODE_U64_TYPE:
          fprintf(stream, "  %%%u = icmp uge i64 %%%u, %%%u\n", result_var,
                  left_var, right_var);
          break;
        case PLX_NODE_F16_TYPE:
          fprintf(stream, "  %%%u = fcmp oge half %%%u, %%%u\n", result_var,
                  left_var, right_var);
          break;
        case PLX_NODE_F32_TYPE:
          fprintf(stream, "  %%%u = fcmp oge float %%%u, %%%u\n", result_var,
                  left_var, right_var);
          break;
        case PLX_NODE_F64_TYPE:
          fprintf(stream, "  %%%u = fcmp oge double %%%u, %%%u\n", result_var,
                  left_var, right_var);
          break;
        default:
          assert(false);
      }
      return result_var;
    }
    case PLX_NODE_GT: {
      const struct plx_node *left, *right;
      plx_extract_children(node, &left, &right);
      const plx_llvm_unnamed_identifier left_var =
          plx_generate_llvm_ir_expr(left, stream, locals);
      const plx_llvm_unnamed_identifier right_var =
          plx_generate_llvm_ir_expr(right, stream, locals);
      const plx_llvm_unnamed_identifier result_var = (*locals)++;
      assert(left->type->kind == right->type->kind);
      switch (left->type->kind) {
        case PLX_NODE_S8_TYPE:
          fprintf(stream, "  %%%u = icmp sgt i8 %%%u, %%%u\n", result_var,
                  left_var, right_var);
          break;
        case PLX_NODE_S16_TYPE:
          fprintf(stream, "  %%%u = icmp sgt i16 %%%u, %%%u\n", result_var,
                  left_var, right_var);
          break;
        case PLX_NODE_S32_TYPE:
          fprintf(stream, "  %%%u = icmp sgt i32 %%%u, %%%u\n", result_var,
                  left_var, right_var);
          break;
        case PLX_NODE_S64_TYPE:
          fprintf(stream, "  %%%u = icmp sgt i64 %%%u, %%%u\n", result_var,
                  left_var, right_var);
          break;
        case PLX_NODE_U8_TYPE:
          fprintf(stream, "  %%%u = icmp ugt i8 %%%u, %%%u\n", result_var,
                  left_var, right_var);
          break;
        case PLX_NODE_U16_TYPE:
          fprintf(stream, "  %%%u = icmp ugt i16 %%%u, %%%u\n", result_var,
                  left_var, right_var);
          break;
        case PLX_NODE_U32_TYPE:
          fprintf(stream, "  %%%u = icmp ugt i32 %%%u, %%%u\n", result_var,
                  left_var, right_var);
          break;
        case PLX_NODE_U64_TYPE:
          fprintf(stream, "  %%%u = icmp ugt i64 %%%u, %%%u\n", result_var,
                  left_var, right_var);
          break;
        case PLX_NODE_F16_TYPE:
          fprintf(stream, "  %%%u = fcmp ogt half %%%u, %%%u\n", result_var,
                  left_var, right_var);
          break;
        case PLX_NODE_F32_TYPE:
          fprintf(stream, "  %%%u = fcmp ogt float %%%u, %%%u\n", result_var,
                  left_var, right_var);
          break;
        case PLX_NODE_F64_TYPE:
          fprintf(stream, "  %%%u = fcmp ogt double %%%u, %%%u\n", result_var,
                  left_var, right_var);
          break;
        default:
          assert(false);
      }
      return result_var;
    }
    case PLX_NODE_ADD: {
      const struct plx_node *left, *right;
      plx_extract_children(node, &left, &right);
      const plx_llvm_unnamed_identifier left_var =
          plx_generate_llvm_ir_expr(left, stream, locals);
      const plx_llvm_unnamed_identifier right_var =
          plx_generate_llvm_ir_expr(right, stream, locals);
      const plx_llvm_unnamed_identifier result_var = (*locals)++;
      switch (node->type->kind) {
        case PLX_NODE_S8_TYPE:
        case PLX_NODE_U8_TYPE:
          fprintf(stream, "  %%%u = add i8 %%%u, %%%u\n", result_var, left_var,
                  right_var);
          break;
        case PLX_NODE_S16_TYPE:
        case PLX_NODE_U16_TYPE:
          fprintf(stream, "  %%%u = add i16 %%%u, %%%u\n", result_var, left_var,
                  right_var);
          break;
        case PLX_NODE_S32_TYPE:
        case PLX_NODE_U32_TYPE:
          fprintf(stream, "  %%%u = add i32 %%%u, %%%u\n", result_var, left_var,
                  right_var);
          break;
        case PLX_NODE_S64_TYPE:
        case PLX_NODE_U64_TYPE:
          fprintf(stream, "  %%%u = add i64 %%%u, %%%u\n", result_var, left_var,
                  right_var);
          break;
        case PLX_NODE_F16_TYPE:
          fprintf(stream, "  %%%u = fadd fast half %%%u, %%%u\n", result_var,
                  left_var, right_var);
          break;
        case PLX_NODE_F32_TYPE:
          fprintf(stream, "  %%%u = fadd fast float %%%u, %%%u\n", result_var,
                  left_var, right_var);
          break;
        case PLX_NODE_F64_TYPE:
          fprintf(stream, "  %%%u = fadd fast double %%%u, %%%u\n", result_var,
                  left_var, right_var);
          break;
        default:
          assert(false);
      }
      return result_var;
    }
    case PLX_NODE_SUB: {
      const struct plx_node *left, *right;
      plx_extract_children(node, &left, &right);
      const plx_llvm_unnamed_identifier left_var =
          plx_generate_llvm_ir_expr(left, stream, locals);
      const plx_llvm_unnamed_identifier right_var =
          plx_generate_llvm_ir_expr(right, stream, locals);
      const plx_llvm_unnamed_identifier result_var = (*locals)++;
      switch (node->type->kind) {
        case PLX_NODE_S8_TYPE:
        case PLX_NODE_U8_TYPE:
          fprintf(stream, "  %%%u = sub i8 %%%u, %%%u\n", result_var, left_var,
                  right_var);
          break;
        case PLX_NODE_S16_TYPE:
        case PLX_NODE_U16_TYPE:
          fprintf(stream, "  %%%u = sub i16 %%%u, %%%u\n", result_var, left_var,
                  right_var);
          break;
        case PLX_NODE_S32_TYPE:
        case PLX_NODE_U32_TYPE:
          fprintf(stream, "  %%%u = sub i32 %%%u, %%%u\n", result_var, left_var,
                  right_var);
          break;
        case PLX_NODE_S64_TYPE:
        case PLX_NODE_U64_TYPE:
          fprintf(stream, "  %%%u = sub i64 %%%u, %%%u\n", result_var, left_var,
                  right_var);
          break;
        case PLX_NODE_F16_TYPE:
          fprintf(stream, "  %%%u = fsub fast half %%%u, %%%u\n", result_var,
                  left_var, right_var);
          break;
        case PLX_NODE_F32_TYPE:
          fprintf(stream, "  %%%u = fsub fast float %%%u, %%%u\n", result_var,
                  left_var, right_var);
          break;
        case PLX_NODE_F64_TYPE:
          fprintf(stream, "  %%%u = fsub fast double %%%u, %%%u\n", result_var,
                  left_var, right_var);
          break;
        default:
          assert(false);
      }
      return result_var;
    }
    case PLX_NODE_MUL: {
      const struct plx_node *left, *right;
      plx_extract_children(node, &left, &right);
      const plx_llvm_unnamed_identifier left_var =
          plx_generate_llvm_ir_expr(left, stream, locals);
      const plx_llvm_unnamed_identifier right_var =
          plx_generate_llvm_ir_expr(right, stream, locals);
      const plx_llvm_unnamed_identifier result_var = (*locals)++;
      switch (node->type->kind) {
        case PLX_NODE_S8_TYPE:
        case PLX_NODE_U8_TYPE:
          fprintf(stream, "  %%%u = mul i8 %%%u, %%%u\n", result_var, left_var,
                  right_var);
          break;
        case PLX_NODE_S16_TYPE:
        case PLX_NODE_U16_TYPE:
          fprintf(stream, "  %%%u = mul i16 %%%u, %%%u\n", result_var, left_var,
                  right_var);
          break;
        case PLX_NODE_S32_TYPE:
        case PLX_NODE_U32_TYPE:
          fprintf(stream, "  %%%u = mul i32 %%%u, %%%u\n", result_var, left_var,
                  right_var);
          break;
        case PLX_NODE_S64_TYPE:
        case PLX_NODE_U64_TYPE:
          fprintf(stream, "  %%%u = mul i64 %%%u, %%%u\n", result_var, left_var,
                  right_var);
          break;
        case PLX_NODE_F16_TYPE:
          fprintf(stream, "  %%%u = fmul fast half %%%u, %%%u\n", result_var,
                  left_var, right_var);
          break;
        case PLX_NODE_F32_TYPE:
          fprintf(stream, "  %%%u = fmul fast float %%%u, %%%u\n", result_var,
                  left_var, right_var);
          break;
        case PLX_NODE_F64_TYPE:
          fprintf(stream, "  %%%u = fmul fast double %%%u, %%%u\n", result_var,
                  left_var, right_var);
          break;
        default:
          assert(false);
      }
      return result_var;
    }
    case PLX_NODE_DIV: {
      const struct plx_node *left, *right;
      plx_extract_children(node, &left, &right);
      const plx_llvm_unnamed_identifier left_var =
          plx_generate_llvm_ir_expr(left, stream, locals);
      const plx_llvm_unnamed_identifier right_var =
          plx_generate_llvm_ir_expr(right, stream, locals);
      const plx_llvm_unnamed_identifier result_var = (*locals)++;
      switch (node->type->kind) {
        case PLX_NODE_S8_TYPE:
          fprintf(stream, "  %%%u = sdiv i8 %%%u, %%%u\n", result_var, left_var,
                  right_var);
          break;
        case PLX_NODE_S16_TYPE:
          fprintf(stream, "  %%%u = sdiv i16 %%%u, %%%u\n", result_var,
                  left_var, right_var);
          break;
        case PLX_NODE_S32_TYPE:
          fprintf(stream, "  %%%u = sdiv i32 %%%u, %%%u\n", result_var,
                  left_var, right_var);
          break;
        case PLX_NODE_S64_TYPE:
          fprintf(stream, "  %%%u = sdiv i64 %%%u, %%%u\n", result_var,
                  left_var, right_var);
          break;
        case PLX_NODE_U8_TYPE:
          fprintf(stream, "  %%%u = udiv i8 %%%u, %%%u\n", result_var, left_var,
                  right_var);
          break;
        case PLX_NODE_U16_TYPE:
          fprintf(stream, "  %%%u = udiv i16 %%%u, %%%u\n", result_var,
                  left_var, right_var);
          break;
        case PLX_NODE_U32_TYPE:
          fprintf(stream, "  %%%u = udiv i32 %%%u, %%%u\n", result_var,
                  left_var, right_var);
          break;
        case PLX_NODE_U64_TYPE:
          fprintf(stream, "  %%%u = udiv i64 %%%u, %%%u\n", result_var,
                  left_var, right_var);
          break;
        case PLX_NODE_F16_TYPE:
          fprintf(stream, "  %%%u = fdiv fast half %%%u, %%%u\n", result_var,
                  left_var, right_var);
          break;
        case PLX_NODE_F32_TYPE:
          fprintf(stream, "  %%%u = fdiv fast float %%%u, %%%u\n", result_var,
                  left_var, right_var);
          break;
        case PLX_NODE_F64_TYPE:
          fprintf(stream, "  %%%u = fdiv fast double %%%u, %%%u\n", result_var,
                  left_var, right_var);
          break;
        default:
          assert(false);
      }
      return result_var;
    }
    case PLX_NODE_REM: {
      const struct plx_node *left, *right;
      plx_extract_children(node, &left, &right);
      const plx_llvm_unnamed_identifier left_var =
          plx_generate_llvm_ir_expr(left, stream, locals);
      const plx_llvm_unnamed_identifier right_var =
          plx_generate_llvm_ir_expr(right, stream, locals);
      const plx_llvm_unnamed_identifier result_var = (*locals)++;
      switch (node->type->kind) {
        case PLX_NODE_S8_TYPE:
          fprintf(stream, "  %%%u = srem i8 %%%u, %%%u\n", result_var, left_var,
                  right_var);
          break;
        case PLX_NODE_S16_TYPE:
          fprintf(stream, "  %%%u = srem i16 %%%u, %%%u\n", result_var,
                  left_var, right_var);
          break;
        case PLX_NODE_S32_TYPE:
          fprintf(stream, "  %%%u = srem i32 %%%u, %%%u\n", result_var,
                  left_var, right_var);
          break;
        case PLX_NODE_S64_TYPE:
          fprintf(stream, "  %%%u = srem i64 %%%u, %%%u\n", result_var,
                  left_var, right_var);
          break;
        case PLX_NODE_U8_TYPE:
          fprintf(stream, "  %%%u = urem i8 %%%u, %%%u\n", result_var, left_var,
                  right_var);
          break;
        case PLX_NODE_U16_TYPE:
          fprintf(stream, "  %%%u = urem i16 %%%u, %%%u\n", result_var,
                  left_var, right_var);
          break;
        case PLX_NODE_U32_TYPE:
          fprintf(stream, "  %%%u = urem i32 %%%u, %%%u\n", result_var,
                  left_var, right_var);
          break;
        case PLX_NODE_U64_TYPE:
          fprintf(stream, "  %%%u = urem i64 %%%u, %%%u\n", result_var,
                  left_var, right_var);
          break;
        default:
          assert(false);
      }
      return result_var;
    }
    case PLX_NODE_LSHIFT: {
      const struct plx_node *left, *right;
      plx_extract_children(node, &left, &right);
      const plx_llvm_unnamed_identifier left_var =
          plx_generate_llvm_ir_expr(left, stream, locals);
      const plx_llvm_unnamed_identifier right_var =
          plx_generate_llvm_ir_expr(right, stream, locals);
      const plx_llvm_unnamed_identifier result_var = (*locals)++;
      switch (node->type->kind) {
        case PLX_NODE_S8_TYPE:
        case PLX_NODE_U8_TYPE:
          fprintf(stream, "  %%%u = shl i8 %%%u, %%%u\n", result_var, left_var,
                  right_var);
          break;
        case PLX_NODE_S16_TYPE:
        case PLX_NODE_U16_TYPE:
          fprintf(stream, "  %%%u = shl i16 %%%u, %%%u\n", result_var, left_var,
                  right_var);
          break;
        case PLX_NODE_S32_TYPE:
        case PLX_NODE_U32_TYPE:
          fprintf(stream, "  %%%u = shl i32 %%%u, %%%u\n", result_var, left_var,
                  right_var);
          break;
        case PLX_NODE_S64_TYPE:
        case PLX_NODE_U64_TYPE:
          fprintf(stream, "  %%%u = shl i64 %%%u, %%%u\n", result_var, left_var,
                  right_var);
          break;
        default:
          assert(false);
      }
      return result_var;
    }
    case PLX_NODE_RSHIFT: {
      const struct plx_node *left, *right;
      plx_extract_children(node, &left, &right);
      const plx_llvm_unnamed_identifier left_var =
          plx_generate_llvm_ir_expr(left, stream, locals);
      const plx_llvm_unnamed_identifier right_var =
          plx_generate_llvm_ir_expr(right, stream, locals);
      const plx_llvm_unnamed_identifier result_var = (*locals)++;
      switch (node->type->kind) {
        case PLX_NODE_S8_TYPE:
        case PLX_NODE_U8_TYPE:
          fprintf(stream, "  %%%u = lshr i8 %%%u, %%%u\n", result_var, left_var,
                  right_var);
          break;
        case PLX_NODE_S16_TYPE:
        case PLX_NODE_U16_TYPE:
          fprintf(stream, "  %%%u = lshr i16 %%%u, %%%u\n", result_var,
                  left_var, right_var);
          break;
        case PLX_NODE_S32_TYPE:
        case PLX_NODE_U32_TYPE:
          fprintf(stream, "  %%%u = lshr i32 %%%u, %%%u\n", result_var,
                  left_var, right_var);
          break;
        case PLX_NODE_S64_TYPE:
        case PLX_NODE_U64_TYPE:
          fprintf(stream, "  %%%u = lshr i64 %%%u, %%%u\n", result_var,
                  left_var, right_var);
          break;
        default:
          assert(false);
      }
      return result_var;
    }
    case PLX_NODE_NOT: {
      const struct plx_node* operand;
      plx_extract_children(node, &operand);
      const plx_llvm_unnamed_identifier operand_var =
          plx_generate_llvm_ir_expr(operand, stream, locals);
      const plx_llvm_unnamed_identifier result_var = (*locals)++;
      switch (node->type->kind) {
        case PLX_NODE_S8_TYPE:
        case PLX_NODE_U8_TYPE:
          fprintf(stream, "  %%%u = xor i8 %%%u, -1\n", result_var,
                  operand_var);
          break;
        case PLX_NODE_S16_TYPE:
        case PLX_NODE_U16_TYPE:
          fprintf(stream, "  %%%u = xor i16 %%%u, -1\n", result_var,
                  operand_var);
          break;
        case PLX_NODE_S32_TYPE:
        case PLX_NODE_U32_TYPE:
          fprintf(stream, "  %%%u = xor i32 %%%u, -1\n", result_var,
                  operand_var);
          break;
        case PLX_NODE_S64_TYPE:
        case PLX_NODE_U64_TYPE:
          fprintf(stream, "  %%%u = xor i64 %%%u, -1\n", result_var,
                  operand_var);
          break;
        case PLX_NODE_BOOL_TYPE:
          fprintf(stream, "  %%%u = xor i1 %%%u, -1\n", result_var,
                  operand_var);
          break;
        default:
          assert(false);
      }
      return result_var;
    }
    case PLX_NODE_NEG: {
      const struct plx_node* operand;
      plx_extract_children(node, &operand);
      const plx_llvm_unnamed_identifier operand_var =
          plx_generate_llvm_ir_expr(operand, stream, locals);
      const plx_llvm_unnamed_identifier result_var = (*locals)++;
      switch (node->type->kind) {
        case PLX_NODE_U8_TYPE:
          fprintf(stream, "  %%%u = sub i8 0, %%%u\n", result_var, operand_var);
          break;
        case PLX_NODE_U16_TYPE:
          fprintf(stream, "  %%%u = sub i16 0, %%%u\n", result_var,
                  operand_var);
          break;
        case PLX_NODE_U32_TYPE:
          fprintf(stream, "  %%%u = sub i32 0, %%%u\n", result_var,
                  operand_var);
          break;
        case PLX_NODE_U64_TYPE:
          fprintf(stream, "  %%%u = sub i64 0, %%%u\n", result_var,
                  operand_var);
          break;
        case PLX_NODE_F16_TYPE:
          fprintf(stream, "  %%%u = fneg fast half %%%u\n", result_var,
                  operand_var);
          break;
        case PLX_NODE_F32_TYPE:
          fprintf(stream, "  %%%u = fneg fast float %%%u\n", result_var,
                  operand_var);
          break;
        case PLX_NODE_F64_TYPE:
          fprintf(stream, "  %%%u = fneg fast double %%%u\n", result_var,
                  operand_var);
          break;
        default:
          assert(false);
      }
      return result_var;
    }
    case PLX_NODE_REF: {
      const struct plx_node* operand;
      plx_extract_children(node, &operand);
      return plx_generate_llvm_ir_ptr(operand, stream, locals);
    }
    case PLX_NODE_DEREF: {
      const struct plx_node* operand;
      plx_extract_children(node, &operand);
      const plx_llvm_unnamed_identifier operand_var =
          plx_generate_llvm_ir_expr(operand, stream, locals);
      const plx_llvm_unnamed_identifier result_var = (*locals)++;
      plx_llvm_ir_fprintf(stream, "  %%%u = load %t, ptr %%%u\n", result_var,
                          node->type, operand_var);
      return result_var;
    }
    case PLX_NODE_CALL: {
      const struct plx_node *func, *args;
      plx_extract_children(node, &func, &args);
      const plx_llvm_unnamed_identifier func_var =
          plx_generate_llvm_ir_expr(func, stream, locals);
      const plx_llvm_unnamed_identifier arg_vars_begin = *locals;
      for (const struct plx_node* arg = args->children; arg != NULL;
           arg = arg->next) {
        plx_generate_llvm_ir_expr(arg, stream, locals);
      }
      const plx_llvm_unnamed_identifier result_var = (*locals)++;
      plx_llvm_ir_fprintf(stream, "  %%%u = call %t %%%u(", result_var,
                          func_var);
      plx_llvm_unnamed_identifier arg_var = arg_vars_begin;
      for (const struct plx_node* arg = args->children; arg != NULL;
           arg = arg->next) {
        plx_llvm_ir_fprintf(stream, "%t %%%u", arg->type, arg_var++);
        if (arg->next != NULL) fputs(", ", stream);
      }
      fputs(")\n", stream);
      break;
    }
    case PLX_NODE_INDEX:
    case PLX_NODE_SLICE:
    case PLX_NODE_FIELD: {
      const plx_llvm_unnamed_identifier ptr_var =
          plx_generate_llvm_ir_ptr(node, stream, locals);
      const plx_llvm_unnamed_identifier result_var = (*locals)++;
      plx_llvm_ir_fprintf(stream, "  %%%u = load %t, ptr %%%u\n", result_var,
                          node->type, ptr_var);
      return result_var;
    }
    case PLX_NODE_IDENTIFIER: {
      const plx_llvm_unnamed_identifier result_var = (*locals)++;
      switch (node->entry->scope) {
        case PLX_SYMBOL_SCOPE_LOCAL:
          plx_llvm_ir_fprintf(stream, "  %%%u = load %t, ptr %%%u\n",
                              result_var, node->type, node->entry->llvm_local_var);
          break;
        case PLX_SYMBOL_SCOPE_GLOBAL:
          plx_llvm_ir_fprintf(stream, "  %%%u = load %t, ptr @%s\n", result_var,
                              node->type, node->name);
          break;
      }
      return result_var;
    }
    case PLX_NODE_STRUCT:
      assert(false);
      break;
    case PLX_NODE_S8: {
      const plx_llvm_unnamed_identifier ptr_var = (*locals)++;
      const plx_llvm_unnamed_identifier result_var = (*locals)++;
      fprintf(stream,
              "  %%%u = alloca i8\n"
              "  store i8 %lld, ptr %%%u\n"
              "  %%%u = load i8, ptr %%%u\n",
              ptr_var, node->sint, ptr_var, result_var, ptr_var);
      return result_var;
    }
    case PLX_NODE_S16: {
      const plx_llvm_unnamed_identifier ptr_var = (*locals)++;
      const plx_llvm_unnamed_identifier result_var = (*locals)++;
      fprintf(stream,
              "  %%%u = alloca i16\n"
              "  store i16 %lld, ptr %%%u\n"
              "  %%%u = load i16, ptr %%%u\n",
              ptr_var, node->sint, ptr_var, result_var, ptr_var);
      return result_var;
    }
    case PLX_NODE_S32: {
      const plx_llvm_unnamed_identifier ptr_var = (*locals)++;
      const plx_llvm_unnamed_identifier result_var = (*locals)++;
      fprintf(stream,
              "  %%%u = alloca i32\n"
              "  store i32 %lld, ptr %%%u\n"
              "  %%%u = load i32, ptr %%%u\n",
              ptr_var, node->sint, ptr_var, result_var, ptr_var);
      return result_var;
    }
    case PLX_NODE_S64: {
      const plx_llvm_unnamed_identifier ptr_var = (*locals)++;
      const plx_llvm_unnamed_identifier result_var = (*locals)++;
      fprintf(stream,
              "  %%%u = alloca i64\n"
              "  store i64 %lld, ptr %%%u\n"
              "  %%%u = load i64, ptr %%%u\n",
              ptr_var, node->sint, ptr_var, result_var, ptr_var);
      return result_var;
    }
    case PLX_NODE_U8: {
      const plx_llvm_unnamed_identifier ptr_var = (*locals)++;
      const plx_llvm_unnamed_identifier result_var = (*locals)++;
      fprintf(stream,
              "  %%%u = alloca i8\n"
              "  store i8 %llu, ptr %%%u\n"
              "  %%%u = load i8, ptr %%%u\n",
              ptr_var, node->uint, ptr_var, result_var, ptr_var);
      return result_var;
    }
    case PLX_NODE_U16: {
      const plx_llvm_unnamed_identifier ptr_var = (*locals)++;
      const plx_llvm_unnamed_identifier result_var = (*locals)++;
      fprintf(stream,
              "  %%%u = alloca i16\n"
              "  store i16 %llu, ptr %%%u\n"
              "  %%%u = load i16, ptr %%%u\n",
              ptr_var, node->uint, ptr_var, result_var, ptr_var);
      return result_var;
    }
    case PLX_NODE_U32: {
      const plx_llvm_unnamed_identifier ptr_var = (*locals)++;
      const plx_llvm_unnamed_identifier result_var = (*locals)++;
      fprintf(stream,
              "  %%%u = alloca i32\n"
              "  store i32 %llu, ptr %%%u\n"
              "  %%%u = load i32, ptr %%%u\n",
              ptr_var, node->uint, ptr_var, result_var, ptr_var);
      return result_var;
    }
    case PLX_NODE_U64: {
      const plx_llvm_unnamed_identifier ptr_var = (*locals)++;
      const plx_llvm_unnamed_identifier result_var = (*locals)++;
      fprintf(stream,
              "  %%%u = alloca i64\n"
              "  store i64 %llu, ptr %%%u\n"
              "  %%%u = load i64, ptr %%%u\n",
              ptr_var, node->uint, ptr_var, result_var, ptr_var);
      return result_var;
    }
    case PLX_NODE_F16: {
      const plx_llvm_unnamed_identifier ptr_var = (*locals)++;
      const plx_llvm_unnamed_identifier result_var = (*locals)++;
      fprintf(stream,
              "  %%%u = alloca half\n"
              "  store half %f, ptr %%%u\n"
              "  %%%u = load half, ptr %%%u\n",
              ptr_var, node->f, ptr_var, result_var, ptr_var);
      return result_var;
    }
    case PLX_NODE_F32: {
      const plx_llvm_unnamed_identifier ptr_var = (*locals)++;
      const plx_llvm_unnamed_identifier result_var = (*locals)++;
      fprintf(stream,
              "  %%%u = alloca float\n"
              "  store float %f, ptr %%%u\n"
              "  %%%u = load float, ptr %%%u\n",
              ptr_var, node->f, ptr_var, result_var, ptr_var);
      return result_var;
    }
    case PLX_NODE_F64: {
      const plx_llvm_unnamed_identifier ptr_var = (*locals)++;
      const plx_llvm_unnamed_identifier result_var = (*locals)++;
      fprintf(stream,
              "  %%%u = alloca double\n"
              "  store double %f, ptr %%%u\n"
              "  %%%u = load double, ptr %%%u\n",
              ptr_var, node->f, ptr_var, result_var, ptr_var);
      return result_var;
    }
    case PLX_NODE_BOOL: {
      const plx_llvm_unnamed_identifier ptr_var = (*locals)++;
      const plx_llvm_unnamed_identifier result_var = (*locals)++;
      fprintf(stream,
              "  %%%u = alloca i1\n"
              "  store i1 %s, ptr %%%u\n"
              "  %%%u = load i1, ptr %%%u\n",
              ptr_var, node->b ? "true" : "false", ptr_var, result_var,
              ptr_var);
      return result_var;
    }
    default:
      assert(false);
  }
  return 0;
}
