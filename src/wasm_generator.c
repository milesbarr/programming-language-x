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

#include "wasm_generator.h"

#include <assert.h>

#include "macros.h"
#include "wasm.h"

static void plx_generate_wasm_type(const struct plx_node* const type,
                                   FILE* const stream) {
  switch (type->kind) {
    case PLX_NODE_S8_TYPE:
    case PLX_NODE_S16_TYPE:
    case PLX_NODE_S32_TYPE:
    case PLX_NODE_U8_TYPE:
    case PLX_NODE_U16_TYPE:
    case PLX_NODE_U32_TYPE:
    case PLX_NODE_BOOL_TYPE:
      fputc(PLX_WASM_I32, stream);
      break;
    case PLX_NODE_S64_TYPE:
    case PLX_NODE_U64_TYPE:
      fputc(PLX_WASM_I64, stream);
      break;
    case PLX_NODE_F16_TYPE:
    case PLX_NODE_F32_TYPE:
      fputc(PLX_WASM_F32, stream);
      break;
    case PLX_NODE_F64_TYPE:
      fputc(PLX_WASM_F64, stream);
      break;
    case PLX_NODE_STRING_TYPE:
    case PLX_NODE_FUNC_TYPE:
    case PLX_NODE_REF_TYPE:
    case PLX_NODE_ARRAY_TYPE:
    case PLX_NODE_SLICE_TYPE:
      assert(false);
      break;
    default:
      assert(false);
  }
}

static void plx_generate_wasm_type_section(const struct plx_node* const module,
                                           FILE* const stream) {
  assert(module->kind == PLX_NODE_MODULE);

  // Write the type count.
  size_t type_count = 0;
  for (const struct plx_node* def = module->children; def != NULL;
       def = def->next) {
    if (def->kind != PLX_NODE_FUNC_DEF) continue;
    ++type_count;
  }
  plx_wasm_write_ull(stream, type_count);

  // Write the types.
  for (const struct plx_node* def = module->children; def != NULL;
       def = def->next) {
    if (def->kind != PLX_NODE_FUNC_DEF) continue;
    const struct plx_node *name, *params, *return_type, *body;
    plx_extract_children(def, &name, &params, &return_type, &body);
    fputc(0x60, stream);
    plx_wasm_write_ull(stream, plx_count_children(params));
    for (const struct plx_node* param = params->children; param != NULL;
         param = param->next) {
      const struct plx_node *param_name, *param_type;
      plx_extract_children(param, &param_name, &param_type);
      plx_generate_wasm_type(param_type, stream);
    }
    plx_wasm_write_ull(stream, 1);
    plx_generate_wasm_type(return_type, stream);
  }
}

static void plx_generate_wasm_function_section(
    const struct plx_node* const module, FILE* const stream) {
  assert(module->kind == PLX_NODE_MODULE);

  // Write the type count.
  size_t type_count = 0;
  for (const struct plx_node* def = module->children; def != NULL;
       def = def->next) {
    if (def->kind != PLX_NODE_FUNC_DEF) continue;
    ++type_count;
  }
  plx_wasm_write_ull(stream, type_count);

  // Write the type indices.
  size_t type_index = 0;
  for (const struct plx_node* def = module->children; def != NULL;
       def = def->next) {
    if (def->kind != PLX_NODE_FUNC_DEF) continue;
    plx_wasm_write_ull(stream, type_index++);
  }
}

static void plx_generate_wasm_code(const struct plx_node* const node,
                                   FILE* const stream) {
  switch (node->kind) {
    case PLX_NODE_MODULE:
      break;
    case PLX_NODE_CONST_DEF:
    case PLX_NODE_VAR_DEF:
      break;
    case PLX_NODE_VAR_DECL:
      break;
    case PLX_NODE_STRUCT_DEF:
    case PLX_NODE_FUNC_DEF:
    case PLX_NODE_NOP:
      break;
    case PLX_NODE_BLOCK:
      for (const struct plx_node* stmt = node->children; stmt != NULL;
           stmt = stmt->next) {
        plx_generate_wasm(stmt, stream);
      }
      break;
    case PLX_NODE_IF_THEN_ELSE: {
      const struct plx_node *cond, *then, *els;
      plx_extract_children(node, &cond, &then, &els);
      plx_generate_wasm(cond, stream);
      fputc(PLX_WASM_IF, stream);
      fputc(PLX_WASM_BLOCK_TYPE_EMPTY, stream);
      plx_generate_wasm(then, stream);
      if (els->children != NULL) {
        fputc(PLX_WASM_ELSE, stream);
        plx_generate_wasm(els, stream);
      }
      fputc(PLX_WASM_END, stream);
      break;
    }
    case PLX_NODE_LOOP: {
      const struct plx_node* body;
      plx_extract_children(node, &body);
      fputc(PLX_WASM_LOOP, stream);
      fputc(PLX_WASM_BLOCK_TYPE_EMPTY, stream);
      plx_generate_wasm(body, stream);
      fputc(PLX_WASM_END, stream);
      break;
    }
    case PLX_NODE_WHILE_LOOP: {
      const struct plx_node *cond, *body;
      plx_extract_children(node, &cond, &body);
      fputc(PLX_WASM_LOOP, stream);
      fputc(PLX_WASM_BLOCK_TYPE_EMPTY, stream);

      // Write the condition.
      plx_generate_wasm(cond, stream);
      fputc(PLX_WASM_BR_IF, stream);
      plx_wasm_write_ull(stream, 0);

      // Write the body.
      plx_generate_wasm(body, stream);

      fputc(PLX_WASM_END, stream);
      break;
    }
    case PLX_NODE_CONTINUE:
      break;
    case PLX_NODE_BREAK:
      fputc(PLX_WASM_BR, stream);
      plx_wasm_write_ull(stream, 0);
      break;
    case PLX_NODE_RETURN: {
      const struct plx_node* const return_value = node->children;
      if (return_value != NULL) plx_generate_wasm(return_value, stream);
      fputc(PLX_WASM_RETURN, stream);
      break;
    }
    case PLX_NODE_ASSIGN:
      break;
    case PLX_NODE_ADD_ASSIGN:
      break;
    case PLX_NODE_SUB_ASSIGN:
      break;
    case PLX_NODE_MUL_ASSIGN:
      break;
    case PLX_NODE_DIV_ASSIGN:
      break;
    case PLX_NODE_REM_ASSIGN:
      break;
    case PLX_NODE_LSHIFT_ASSIGN:
      break;
    case PLX_NODE_RSHIFT_ASSIGN:
      break;
    case PLX_NODE_AND: {
      const struct plx_node *left, *right;
      plx_extract_children(node, &left, &right);
      plx_generate_wasm(left, stream);
      plx_generate_wasm(right, stream);
      assert(left->type->kind == right->type->kind);
      switch (left->type->kind) {
        case PLX_NODE_S8_TYPE:
        case PLX_NODE_S16_TYPE:
        case PLX_NODE_S32_TYPE:
        case PLX_NODE_U8_TYPE:
        case PLX_NODE_U16_TYPE:
        case PLX_NODE_U32_TYPE:
        case PLX_NODE_BOOL_TYPE:
          fputc(PLX_WASM_I32_AND, stream);
          break;
        case PLX_NODE_S64_TYPE:
        case PLX_NODE_U64_TYPE:
          fputc(PLX_WASM_I64_AND, stream);
          break;
        default:
          assert(false);
      }
      break;
    }
    case PLX_NODE_OR: {
      const struct plx_node *left, *right;
      plx_extract_children(node, &left, &right);
      plx_generate_wasm(left, stream);
      plx_generate_wasm(right, stream);
      assert(left->type->kind == right->type->kind);
      switch (left->type->kind) {
        case PLX_NODE_S8_TYPE:
        case PLX_NODE_S16_TYPE:
        case PLX_NODE_S32_TYPE:
        case PLX_NODE_U8_TYPE:
        case PLX_NODE_U16_TYPE:
        case PLX_NODE_U32_TYPE:
        case PLX_NODE_BOOL_TYPE:
          fputc(PLX_WASM_I32_OR, stream);
          break;
        case PLX_NODE_S64_TYPE:
        case PLX_NODE_U64_TYPE:
          fputc(PLX_WASM_I64_OR, stream);
          break;
        default:
          assert(false);
      }
      break;
    }
    case PLX_NODE_XOR: {
      const struct plx_node *left, *right;
      plx_extract_children(node, &left, &right);
      plx_generate_wasm(left, stream);
      plx_generate_wasm(right, stream);
      assert(left->type->kind == right->type->kind);
      switch (left->type->kind) {
        case PLX_NODE_S8_TYPE:
        case PLX_NODE_S16_TYPE:
        case PLX_NODE_S32_TYPE:
        case PLX_NODE_U8_TYPE:
        case PLX_NODE_U16_TYPE:
        case PLX_NODE_U32_TYPE:
        case PLX_NODE_BOOL_TYPE:
          fputc(PLX_WASM_I32_XOR, stream);
          break;
        case PLX_NODE_S64_TYPE:
        case PLX_NODE_U64_TYPE:
          fputc(PLX_WASM_I64_XOR, stream);
          break;
        default:
          assert(false);
      }
      break;
    }
    case PLX_NODE_EQ: {
      const struct plx_node *left, *right;
      plx_extract_children(node, &left, &right);
      plx_generate_wasm(left, stream);
      plx_generate_wasm(right, stream);
      assert(left->type->kind == right->type->kind);
      switch (left->type->kind) {
        case PLX_NODE_S8_TYPE:
        case PLX_NODE_S16_TYPE:
        case PLX_NODE_S32_TYPE:
        case PLX_NODE_U8_TYPE:
        case PLX_NODE_U16_TYPE:
        case PLX_NODE_U32_TYPE:
        case PLX_NODE_BOOL_TYPE:
          fputc(PLX_WASM_I32_EQ, stream);
          break;
        case PLX_NODE_S64_TYPE:
        case PLX_NODE_U64_TYPE:
          fputc(PLX_WASM_I64_EQ, stream);
          break;
        case PLX_NODE_F16_TYPE:
        case PLX_NODE_F32_TYPE:
          fputc(PLX_WASM_F32_EQ, stream);
          break;
        case PLX_NODE_F64_TYPE:
          fputc(PLX_WASM_F64_EQ, stream);
          break;
        default:
          assert(false);
      }
      break;
    }
    case PLX_NODE_NEQ: {
      const struct plx_node *left, *right;
      plx_extract_children(node, &left, &right);
      plx_generate_wasm(left, stream);
      plx_generate_wasm(right, stream);
      assert(left->type->kind == right->type->kind);
      switch (left->type->kind) {
        case PLX_NODE_S8_TYPE:
        case PLX_NODE_S16_TYPE:
        case PLX_NODE_S32_TYPE:
        case PLX_NODE_U8_TYPE:
        case PLX_NODE_U16_TYPE:
        case PLX_NODE_U32_TYPE:
        case PLX_NODE_BOOL_TYPE:
          fputc(PLX_WASM_I32_NE, stream);
          break;
        case PLX_NODE_S64_TYPE:
        case PLX_NODE_U64_TYPE:
          fputc(PLX_WASM_I64_NE, stream);
          break;
        case PLX_NODE_F16_TYPE:
        case PLX_NODE_F32_TYPE:
          fputc(PLX_WASM_F32_NE, stream);
          break;
        case PLX_NODE_F64_TYPE:
          fputc(PLX_WASM_F64_NE, stream);
          break;
        default:
          assert(false);
      }
      break;
    }
    case PLX_NODE_ADD: {
      const struct plx_node *left, *right;
      plx_extract_children(node, &left, &right);
      plx_generate_wasm(left, stream);
      plx_generate_wasm(right, stream);
      assert(left->type->kind == right->type->kind);
      switch (left->type->kind) {
        case PLX_NODE_S8_TYPE:
        case PLX_NODE_S16_TYPE:
        case PLX_NODE_S32_TYPE:
        case PLX_NODE_U8_TYPE:
        case PLX_NODE_U16_TYPE:
        case PLX_NODE_U32_TYPE:
          fputc(PLX_WASM_I32_ADD, stream);
          break;
        case PLX_NODE_S64_TYPE:
        case PLX_NODE_U64_TYPE:
          fputc(PLX_WASM_I64_ADD, stream);
          break;
        case PLX_NODE_F16_TYPE:
        case PLX_NODE_F32_TYPE:
          fputc(PLX_WASM_F32_ADD, stream);
          break;
        case PLX_NODE_F64_TYPE:
          fputc(PLX_WASM_F64_ADD, stream);
          break;
        default:
          assert(false);
      }
      break;
    }
    case PLX_NODE_SUB: {
      const struct plx_node *left, *right;
      plx_extract_children(node, &left, &right);
      plx_generate_wasm(left, stream);
      plx_generate_wasm(right, stream);
      assert(left->type->kind == right->type->kind);
      switch (left->type->kind) {
        case PLX_NODE_S8_TYPE:
        case PLX_NODE_S16_TYPE:
        case PLX_NODE_S32_TYPE:
        case PLX_NODE_U8_TYPE:
        case PLX_NODE_U16_TYPE:
        case PLX_NODE_U32_TYPE:
          fputc(PLX_WASM_I32_SUB, stream);
          break;
        case PLX_NODE_S64_TYPE:
        case PLX_NODE_U64_TYPE:
          fputc(PLX_WASM_I64_SUB, stream);
          break;
        case PLX_NODE_F16_TYPE:
        case PLX_NODE_F32_TYPE:
          fputc(PLX_WASM_F32_SUB, stream);
          break;
        case PLX_NODE_F64_TYPE:
          fputc(PLX_WASM_F64_SUB, stream);
          break;
        default:
          assert(false);
      }
      break;
    }
    case PLX_NODE_MUL: {
      const struct plx_node *left, *right;
      plx_extract_children(node, &left, &right);
      plx_generate_wasm(left, stream);
      plx_generate_wasm(right, stream);
      assert(left->type->kind == right->type->kind);
      switch (left->type->kind) {
        case PLX_NODE_S8_TYPE:
        case PLX_NODE_S16_TYPE:
        case PLX_NODE_S32_TYPE:
        case PLX_NODE_U8_TYPE:
        case PLX_NODE_U16_TYPE:
        case PLX_NODE_U32_TYPE:
          fputc(PLX_WASM_I32_MUL, stream);
          break;
        case PLX_NODE_S64_TYPE:
        case PLX_NODE_U64_TYPE:
          fputc(PLX_WASM_I64_MUL, stream);
          break;
        case PLX_NODE_F16_TYPE:
        case PLX_NODE_F32_TYPE:
          fputc(PLX_WASM_F32_MUL, stream);
          break;
        case PLX_NODE_F64_TYPE:
          fputc(PLX_WASM_F64_MUL, stream);
          break;
        default:
          assert(false);
      }
      break;
    }
    case PLX_NODE_DIV: {
      const struct plx_node *left, *right;
      plx_extract_children(node, &left, &right);
      plx_generate_wasm(left, stream);
      plx_generate_wasm(right, stream);
      assert(left->type->kind == right->type->kind);
      switch (node->type->kind) {
        case PLX_NODE_S8_TYPE:
        case PLX_NODE_S16_TYPE:
        case PLX_NODE_S32_TYPE:
          fputc(PLX_WASM_I32_DIV_S, stream);
          break;
        case PLX_NODE_S64_TYPE:
          fputc(PLX_WASM_I64_DIV_S, stream);
          break;
        case PLX_NODE_U8_TYPE:
        case PLX_NODE_U16_TYPE:
        case PLX_NODE_U32_TYPE:
          fputc(PLX_WASM_I32_DIV_U, stream);
          break;
        case PLX_NODE_U64_TYPE:
          fputc(PLX_WASM_I64_DIV_U, stream);
          break;
        case PLX_NODE_F16_TYPE:
        case PLX_NODE_F32_TYPE:
          fputc(PLX_WASM_F32_DIV, stream);
          break;
        case PLX_NODE_F64_TYPE:
          fputc(PLX_WASM_F64_DIV, stream);
          break;
        default:
          assert(false);
      }
      break;
    }
    case PLX_NODE_REM: {
      const struct plx_node *left, *right;
      plx_extract_children(node, &left, &right);
      plx_generate_wasm(left, stream);
      plx_generate_wasm(right, stream);
      switch (node->type->kind) {
        case PLX_NODE_S8_TYPE:
        case PLX_NODE_S16_TYPE:
        case PLX_NODE_S32_TYPE:
          fputc(PLX_WASM_I32_REM_S, stream);
          break;
        case PLX_NODE_S64_TYPE:
          fputc(PLX_WASM_I64_REM_S, stream);
          break;
        case PLX_NODE_U8_TYPE:
        case PLX_NODE_U16_TYPE:
        case PLX_NODE_U32_TYPE:
          fputc(PLX_WASM_I32_REM_U, stream);
          break;
        case PLX_NODE_U64_TYPE:
          fputc(PLX_WASM_I64_REM_U, stream);
          break;
        default:
          assert(false);
      }
      break;
    }
    case PLX_NODE_LSHIFT: {
      const struct plx_node *left, *right;
      plx_extract_children(node, &left, &right);
      plx_generate_wasm(left, stream);
      plx_generate_wasm(right, stream);
      switch (node->type->kind) {
        case PLX_NODE_S8_TYPE:
        case PLX_NODE_S16_TYPE:
        case PLX_NODE_S32_TYPE:
          fputc(PLX_WASM_I32_SHL, stream);
          break;
        case PLX_NODE_U8_TYPE:
        case PLX_NODE_U16_TYPE:
        case PLX_NODE_U32_TYPE:
          fputc(PLX_WASM_I32_SHL, stream);
          break;
        case PLX_NODE_S64_TYPE:
        case PLX_NODE_U64_TYPE:
          fputc(PLX_WASM_I64_SHL, stream);
          break;
        default:
          assert(false);
      }
      break;
    }
    case PLX_NODE_RSHIFT: {
      const struct plx_node *left, *right;
      plx_extract_children(node, &left, &right);
      plx_generate_wasm(left, stream);
      plx_generate_wasm(right, stream);
      switch (node->type->kind) {
        case PLX_NODE_S8_TYPE:
        case PLX_NODE_S16_TYPE:
        case PLX_NODE_S32_TYPE:
          fputc(PLX_WASM_I32_SHR_S, stream);
          break;
        case PLX_NODE_S64_TYPE:
          fputc(PLX_WASM_I64_SHR_S, stream);
          break;
        case PLX_NODE_U8_TYPE:
        case PLX_NODE_U16_TYPE:
        case PLX_NODE_U32_TYPE:
          fputc(PLX_WASM_I32_SHR_U, stream);
          break;
        case PLX_NODE_U64_TYPE:
          fputc(PLX_WASM_I64_SHR_U, stream);
          break;
        default:
          assert(false);
      }
      break;
    }
    case PLX_NODE_NOT:
      break;
    case PLX_NODE_NEG:
      break;
    case PLX_NODE_REF:
      break;
    case PLX_NODE_DEREF:
      break;
    case PLX_NODE_CALL:
      break;
    case PLX_NODE_INDEX:
      break;
    case PLX_NODE_SLICE:
      break;
    case PLX_NODE_FIELD:
      break;
    case PLX_NODE_IDENTIFIER:
      break;
    case PLX_NODE_S8:
    case PLX_NODE_S16:
    case PLX_NODE_S32:
      fputc(PLX_WASM_I32_CONST, stream);
      plx_wasm_write_ll(stream, node->sint);
      break;
    case PLX_NODE_S64:
      fputc(PLX_WASM_I64_CONST, stream);
      plx_wasm_write_ll(stream, node->sint);
      break;
    case PLX_NODE_U8:
    case PLX_NODE_U16:
    case PLX_NODE_U32:
      fputc(PLX_WASM_I32_CONST, stream);
      plx_wasm_write_ull(stream, node->uint);
      break;
    case PLX_NODE_U64:
      fputc(PLX_WASM_I64_CONST, stream);
      plx_wasm_write_ull(stream, node->uint);
      break;
    case PLX_NODE_F16:
    case PLX_NODE_F32:
    case PLX_NODE_F64:
      assert(false);
      break;
    case PLX_NODE_BOOL:
      fputc(PLX_WASM_I32_CONST, stream);
      plx_wasm_write_ll(stream, node->b ? 1 : 0);
      break;
    case PLX_NODE_STRING:
      break;
    case PLX_NODE_VOID_TYPE:
    case PLX_NODE_S8_TYPE:
    case PLX_NODE_S16_TYPE:
    case PLX_NODE_S32_TYPE:
    case PLX_NODE_S64_TYPE:
    case PLX_NODE_U8_TYPE:
    case PLX_NODE_U16_TYPE:
    case PLX_NODE_U32_TYPE:
    case PLX_NODE_U64_TYPE:
    case PLX_NODE_F16_TYPE:
    case PLX_NODE_F32_TYPE:
    case PLX_NODE_F64_TYPE:
    case PLX_NODE_BOOL_TYPE:
    case PLX_NODE_STRING_TYPE:
    case PLX_NODE_FUNC_TYPE:
    case PLX_NODE_REF_TYPE:
    case PLX_NODE_ARRAY_TYPE:
    case PLX_NODE_SLICE_TYPE:
      assert(false);
      break;
  }
}

static bool plx_copy_file(FILE* const in, FILE* const out, size_t size) {
#ifdef _WIN32
  char buf[1024];
  while (size > 0) {
    const size_t bytes_read =
        fread(buf, 1, size < sizeof(buf) ? size : sizeof(buf), in);
    if (bytes_read == 0) return ferror(in) == 0;
    if (plx_unlikely(bytes_read > size)) return false;
    const size_t bytes_written = fwrite(buf, 1, bytes_read, out);
    if (plx_unlikely(bytes_written < bytes_read)) return false;
    size -= bytes_written;
  }
#else
  while (size > 0) {
    const ssize_t bytes_written = sendfile(fileno(in), fileno(out), NULL, size);
    if (plx_unlikely(bytes_written < 0)) return false;
    if (bytes_written == 0) return true;
    if (plx_unlikely(bytes_written > size)) return false;
    size -= bytes_written;
  }
#endif  // _WIN32
  return true;
}

bool plx_generate_wasm(const struct plx_node* const module,
                       FILE* const stream) {
  assert(module->kind == PLX_NODE_MODULE);

  // Write the module preamble.
  plx_wasm_write_module_preamble(stream);

  // Generate the type section.
  FILE* const tmp = tmpfile();
  plx_generate_wasm_type_section(module, tmp);
  long size = ftell(tmp);
  plx_wasm_write_section_header(stream, PLX_WASM_SECTION_TYPE, size);
  fseek(tmp, 0, SEEK_SET);
  if (plx_unlikely(!plx_copy_file(tmp, stream, size))) {
    fclose(tmp);
    return false;
  }

  // Generate the function section.
  fseek(tmp, 0, SEEK_SET);
  plx_generate_wasm_function_section(module, tmp);
  size = ftell(tmp);
  plx_wasm_write_section_header(stream, PLX_WASM_SECTION_FUNCTION, size);
  fseek(tmp, 0, SEEK_SET);
  if (plx_unlikely(!plx_copy_file(tmp, stream, size))) {
    fclose(tmp);
    return false;
  }

  // Generate the export section.

  // Generate the code section.
  fseek(tmp, 0, SEEK_SET);
  plx_generate_wasm_code(module, tmp);
  size = ftell(tmp);
  plx_wasm_write_section_header(stream, PLX_WASM_SECTION_CODE, size);
  fseek(tmp, 0, SEEK_SET);
  if (plx_unlikely(!plx_copy_file(tmp, stream, size))) {
    fclose(tmp);
    return false;
  }

  fclose(tmp);
  return true;
}
