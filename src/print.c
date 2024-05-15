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

#include "print.h"

#ifndef NDEBUG

#include <stddef.h>

void plx_print(const struct plx_node* const node, FILE* const stream) {
  switch (node->kind) {
    case PLX_NODE_MODULE:
      for (const struct plx_node* def = node->children; def != NULL;
           def = def->next) {
        plx_print(def, stream);
      }
      break;
    case PLX_NODE_CONST_DEF: {
      const struct plx_node *name, *value;
      plx_extract_children(node, &name, &value);
      fputs("const ", stream);
      plx_print(name, stream);
      fputs(" = ", stream);
      plx_print(value, stream);
      fputs(";\n", stream);
      break;
    }
    case PLX_NODE_VAR_DEF: {
      const struct plx_node *name, *value;
      plx_extract_children(node, &name, &value);
      fputs("var ", stream);
      plx_print(name, stream);
      fputs(" = ", stream);
      plx_print(value, stream);
      fputs(";\n", stream);
      break;
    }
    case PLX_NODE_VAR_DECL: {
      const struct plx_node *name, *type;
      plx_extract_children(node, &name, &type);
      fputs("var ", stream);
      plx_print(name, stream);
      fputs(": ", stream);
      plx_print(type, stream);
      fputs(";\n", stream);
      break;
    }
    case PLX_NODE_STRUCT_DEF: {
      const struct plx_node *name, *members;
      plx_extract_children(node, &name, &members);
      fputs("struct ", stream);
      plx_print(name, stream);
      fputc(' ', stream);
      for (const struct plx_node* member = members->children; member != NULL;
           member = member->next) {
        const struct plx_node *member_name, *member_type;
        plx_extract_children(node, &member_name, &member_type);
        plx_print(member_name, stream);
        fputs(": ", stream);
        plx_print(member_type, stream);
        fputs(";\n", stream);
      }
      fputs(";\n", stream);
      break;
    }
    case PLX_NODE_FUNC_DEF: {
      const struct plx_node *name, *params, *return_type, *body;
      plx_extract_children(node, &name, &params, &return_type, &body);
      fputs("func ", stream);
      plx_print(name, stream);
      fputc('(', stream);
      for (const struct plx_node* param = params->children; param != NULL;
           param = param->next) {
        const struct plx_node *param_name, *param_type;
        plx_extract_children(node, &param_name, &param_type);
        plx_print(param_name, stream);
        fputs(": ", stream);
        plx_print(param_type, stream);
        if (param->next != NULL) fputs(", ", stream);
      }
      fputs(") -> ", stream);
      plx_print(return_type, stream);
      fputc(' ', stream);
      plx_print(body, stream);
      break;
    }
    case PLX_NODE_NOP:
      break;
    case PLX_NODE_BLOCK:
      fputs("{\n", stream);
      for (const struct plx_node* stmt = node->children; stmt != NULL;
           stmt = stmt->next) {
        plx_print(stmt, stream);
      }
      fputs("}\n", stream);
      break;
    case PLX_NODE_IF_THEN_ELSE: {
      const struct plx_node *cond, *then, *els;
      plx_extract_children(node, &cond, &then, &els);
      fputs("if ", stream);
      plx_print(cond, stream);
      fputc(' ', stream);
      plx_print(then, stream);
      fputs(" else ", stream);
      plx_print(els, stream);
      break;
    }
    case PLX_NODE_LOOP: {
      const struct plx_node* body;
      plx_extract_children(node, &body);
      fputs("loop ", stream);
      plx_print(body, stream);
      break;
    }
    case PLX_NODE_WHILE_LOOP: {
      const struct plx_node *cond, *body;
      plx_extract_children(node, &cond, &body);
      fputs("while ", stream);
      plx_print(cond, stream);
      fputc(' ', stream);
      plx_print(body, stream);
      break;
    }
    case PLX_NODE_CONTINUE:
      fputs("continue;\n", stream);
      break;
    case PLX_NODE_BREAK:
      fputs("break;\n", stream);
      break;
    case PLX_NODE_RETURN: {
      const struct plx_node* return_type;
      plx_extract_children(node, &return_type);
      fputs("return", stream);
      if (return_type != NULL) {
        fputc(' ', stream);
        plx_print(return_type, stream);
      }
      fputs(";\n", stream);
      break;
    }
    case PLX_NODE_ASSIGN: {
      const struct plx_node *assignee, *value;
      plx_extract_children(node, &assignee, &value);
      fputc('(', stream);
      plx_print(assignee, stream);
      fputs(") = (", stream);
      plx_print(value, stream);
      fputs(");\n", stream);
      break;
    }
    case PLX_NODE_ADD_ASSIGN: {
      const struct plx_node *assignee, *value;
      plx_extract_children(node, &assignee, &value);
      fputc('(', stream);
      plx_print(assignee, stream);
      fputs(") += (", stream);
      plx_print(value, stream);
      fputs(");\n", stream);
      break;
    }
    case PLX_NODE_SUB_ASSIGN: {
      const struct plx_node *assignee, *value;
      plx_extract_children(node, &assignee, &value);
      fputc('(', stream);
      plx_print(assignee, stream);
      fputs(") -= (", stream);
      plx_print(value, stream);
      fputs(");\n", stream);
      break;
    }
    case PLX_NODE_MUL_ASSIGN: {
      const struct plx_node *assignee, *value;
      plx_extract_children(node, &assignee, &value);
      fputc('(', stream);
      plx_print(assignee, stream);
      fputs(") *= (", stream);
      plx_print(value, stream);
      fputs(");\n", stream);
      break;
    }
    case PLX_NODE_DIV_ASSIGN: {
      const struct plx_node *assignee, *value;
      plx_extract_children(node, &assignee, &value);
      fputc('(', stream);
      plx_print(assignee, stream);
      fputs(") /= (", stream);
      plx_print(value, stream);
      fputs(");\n", stream);
      break;
    }
    case PLX_NODE_REM_ASSIGN: {
      const struct plx_node *assignee, *value;
      plx_extract_children(node, &assignee, &value);
      fputc('(', stream);
      plx_print(assignee, stream);
      fputs(") %= (", stream);
      plx_print(value, stream);
      fputs(");\n", stream);
      break;
    }
    case PLX_NODE_LSHIFT_ASSIGN: {
      const struct plx_node *assignee, *value;
      plx_extract_children(node, &assignee, &value);
      fputc('(', stream);
      plx_print(assignee, stream);
      fputs(") <<= (", stream);
      plx_print(value, stream);
      fputs(");\n", stream);
      break;
    }
    case PLX_NODE_RSHIFT_ASSIGN: {
      const struct plx_node *assignee, *value;
      plx_extract_children(node, &assignee, &value);
      fputc('(', stream);
      plx_print(assignee, stream);
      fputs(") >>= (", stream);
      plx_print(value, stream);
      fputs(");\n", stream);
      break;
    }
    case PLX_NODE_AND: {
      const struct plx_node *left, *right;
      plx_extract_children(node, &left, &right);
      fputc('(', stream);
      plx_print(left, stream);
      fputs(") and (", stream);
      plx_print(right, stream);
      fputs(");\n", stream);
      break;
    }
    case PLX_NODE_OR: {
      const struct plx_node *left, *right;
      plx_extract_children(node, &left, &right);
      fputc('(', stream);
      plx_print(left, stream);
      fputs(") or (", stream);
      plx_print(right, stream);
      fputs(");\n", stream);
      break;
    }
    case PLX_NODE_XOR: {
      const struct plx_node *left, *right;
      plx_extract_children(node, &left, &right);
      fputc('(', stream);
      plx_print(left, stream);
      fputs(") xor (", stream);
      plx_print(right, stream);
      fputs(");\n", stream);
      break;
    }
    case PLX_NODE_EQ: {
      const struct plx_node *left, *right;
      plx_extract_children(node, &left, &right);
      fputc('(', stream);
      plx_print(left, stream);
      fputs(") == (", stream);
      plx_print(right, stream);
      fputc(')', stream);
      break;
    }
    case PLX_NODE_NEQ: {
      const struct plx_node *left, *right;
      plx_extract_children(node, &left, &right);
      fputc('(', stream);
      plx_print(left, stream);
      fputs(") != (", stream);
      plx_print(right, stream);
      fputc(')', stream);
      break;
    }
    case PLX_NODE_LTE: {
      const struct plx_node *left, *right;
      plx_extract_children(node, &left, &right);
      fputc('(', stream);
      plx_print(left, stream);
      fputs(") <= (", stream);
      plx_print(right, stream);
      fputc(')', stream);
      break;
    }
    case PLX_NODE_LT: {
      const struct plx_node *left, *right;
      plx_extract_children(node, &left, &right);
      fputc('(', stream);
      plx_print(left, stream);
      fputs(") < (", stream);
      plx_print(right, stream);
      fputc(')', stream);
      break;
    }
    case PLX_NODE_GTE: {
      const struct plx_node *left, *right;
      plx_extract_children(node, &left, &right);
      fputc('(', stream);
      plx_print(left, stream);
      fputs(") >= (", stream);
      plx_print(right, stream);
      fputc(')', stream);
      break;
    }
    case PLX_NODE_GT: {
      const struct plx_node *left, *right;
      plx_extract_children(node, &left, &right);
      fputc('(', stream);
      plx_print(left, stream);
      fputs(") > (", stream);
      plx_print(right, stream);
      fputc(')', stream);
      break;
    }
    case PLX_NODE_ADD: {
      const struct plx_node *left, *right;
      plx_extract_children(node, &left, &right);
      fputc('(', stream);
      plx_print(left, stream);
      fputs(") + (", stream);
      plx_print(right, stream);
      fputc(')', stream);
      break;
    }
    case PLX_NODE_SUB: {
      const struct plx_node *left, *right;
      plx_extract_children(node, &left, &right);
      fputc('(', stream);
      plx_print(left, stream);
      fputs(") - (", stream);
      plx_print(right, stream);
      fputc(')', stream);
      break;
    }
    case PLX_NODE_MUL: {
      const struct plx_node *left, *right;
      plx_extract_children(node, &left, &right);
      fputc('(', stream);
      plx_print(left, stream);
      fputs(") * (", stream);
      plx_print(right, stream);
      fputc(')', stream);
      break;
    }
    case PLX_NODE_DIV: {
      const struct plx_node *left, *right;
      plx_extract_children(node, &left, &right);
      fputc('(', stream);
      plx_print(left, stream);
      fputs(") / (", stream);
      plx_print(right, stream);
      fputc(')', stream);
      break;
    }
    case PLX_NODE_REM: {
      const struct plx_node *left, *right;
      plx_extract_children(node, &left, &right);
      fputc('(', stream);
      plx_print(left, stream);
      fputs(") % (", stream);
      plx_print(right, stream);
      fputc(')', stream);
      break;
    }
    case PLX_NODE_LSHIFT: {
      const struct plx_node *left, *right;
      plx_extract_children(node, &left, &right);
      fputc('(', stream);
      plx_print(left, stream);
      fputs(") << (", stream);
      plx_print(right, stream);
      fputc(')', stream);
      break;
    }
    case PLX_NODE_RSHIFT: {
      const struct plx_node *left, *right;
      plx_extract_children(node, &left, &right);
      fputc('(', stream);
      plx_print(left, stream);
      fputs(") >> (", stream);
      plx_print(right, stream);
      fputc(')', stream);
      break;
    }
    case PLX_NODE_NOT: {
      const struct plx_node* operand;
      plx_extract_children(node, &operand);
      fputs("!(", stream);
      plx_print(operand, stream);
      fputc(')', stream);
      break;
    }
    case PLX_NODE_NEG: {
      const struct plx_node* operand;
      plx_extract_children(node, &operand);
      fputs("-(", stream);
      plx_print(operand, stream);
      fputc(')', stream);
      break;
    }
    case PLX_NODE_REF: {
      const struct plx_node* operand;
      plx_extract_children(node, &operand);
      fputs("&(", stream);
      plx_print(operand, stream);
      fputc(')', stream);
      break;
    }
    case PLX_NODE_DEREF: {
      const struct plx_node* operand;
      plx_extract_children(node, &operand);
      fputs("*(", stream);
      plx_print(operand, stream);
      fputc(')', stream);
      break;
    }
    case PLX_NODE_CALL: {
      const struct plx_node *func, *args;
      plx_extract_children(node, &func, &args);
      fputc('(', stream);
      plx_print(func, stream);
      fputs(")(", stream);
      for (const struct plx_node* arg = args->children; arg != NULL;
           arg = arg->next) {
        plx_print(arg, stream);
        if (arg->next != NULL) fputs(", ", stream);
      }
      fputc(')', stream);
      break;
    }
    case PLX_NODE_INDEX: {
      const struct plx_node *value, *index;
      plx_extract_children(node, &value, &index);
      fputc('(', stream);
      plx_print(value, stream);
      fputs(")[", stream);
      plx_print(index, stream);
      fputc(']', stream);
      break;
    }
    case PLX_NODE_FIELD:
      break;
    case PLX_NODE_IDENTIFIER:
      fputs(node->name, stream);
      break;
    case PLX_NODE_STRUCT: {
      const struct plx_node *name, *members;
      plx_extract_children(node, &name, &members);
      plx_print(name, stream);
      fputs(" {\n", stream);
      for (const struct plx_node* member = members->children; member != NULL;
           member = member->next) {
        const struct plx_node *member_name, *member_value;
        plx_extract_children(member, &member_name, &member_value);
        plx_print(member_name, stream);
        fputs(": ", stream);
        plx_print(member_value, stream);
        fputs(";\n", stream);
      }
      fputs("}\n", stream);
      break;
    }
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
    case PLX_NODE_STRING:
      fputc('"', stream);
      for (size_t i = 0; i < node->len; ++i) {
        const char c = node->str[i];
        switch (c) {
          case '\0':
            fputs("\\0", stream);
            break;
          case '\\':
          case '"':
            fputc('\\', stream);
          default:
            fputc(c, stream);
        }
      }
      fputc('"', stream);
      break;
    case PLX_NODE_VOID_TYPE:
      fputs("void", stream);
      break;
    case PLX_NODE_S8_TYPE:
      fputs("s8", stream);
      break;
    case PLX_NODE_S16_TYPE:
      fputs("s16", stream);
      break;
    case PLX_NODE_S32_TYPE:
      fputs("s32", stream);
      break;
    case PLX_NODE_S64_TYPE:
      fputs("s64", stream);
      break;
    case PLX_NODE_U8_TYPE:
      fputs("u8", stream);
      break;
    case PLX_NODE_U16_TYPE:
      fputs("u16", stream);
      break;
    case PLX_NODE_U32_TYPE:
      fputs("u32", stream);
      break;
    case PLX_NODE_U64_TYPE:
      fputs("u64", stream);
      break;
    case PLX_NODE_F16_TYPE:
      fputs("f16", stream);
      break;
    case PLX_NODE_F32_TYPE:
      fputs("f32", stream);
      break;
    case PLX_NODE_F64_TYPE:
      fputs("f64", stream);
      break;
    case PLX_NODE_BOOL_TYPE:
      fputs("bool", stream);
      break;
    case PLX_NODE_STRING_TYPE:
      fputs("string", stream);
      break;
    case PLX_NODE_FUNC_TYPE: {
      const struct plx_node *param_types, *return_type;
      plx_extract_children(node, &param_types, &return_type);
      fputs("func (", stream);
      for (const struct plx_node* param_type = param_types->children;
           param_type != NULL; param_type = param_type->next) {
        plx_print(param_type, stream);
        if (param_type->next != NULL) fputs(", ", stream);
      }
      fputs(") -> ", stream);
      plx_print(return_type, stream);
      break;
    }
    case PLX_NODE_REF_TYPE: {
      const struct plx_node* type;
      plx_extract_children(node, &type);
      fputc('&', stream);
      plx_print(type, stream);
      break;
    }
    case PLX_NODE_ARRAY_TYPE: {
      const struct plx_node *len, *element_type;
      plx_extract_children(node, &len, &element_type);
      fputc('[', stream);
      plx_print(len, stream);
      fputc(']', stream);
      plx_print(element_type, stream);
      break;
    }
    case PLX_NODE_SLICE_TYPE: {
      const struct plx_node* element_type;
      plx_extract_children(node, &element_type);
      fputs("[]", stream);
      plx_print(element_type, stream);
      break;
    }
    case PLX_NODE_OTHER:
      break;
  }
}

#endif  // NDEBUG
