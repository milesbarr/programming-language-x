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

#include "compiler.h"

#include <stdio.h>
#include <stdlib.h>

#include "ast.h"
#include "ast_validator.h"
#include "constant_folder.h"
#include "dir.h"
#include "error.h"
#include "llvm_ir_generator.h"
#include "macros.h"
#include "name_resolver.h"
#include "parser.h"
#include "path.h"
#include "print.h"
#include "return_checker.h"
#include "symbol_table.h"
#include "tokenizer.h"
#include "type_checker.h"
#include "wasm_generator.h"

#define PLX_FILE_EXT ".plx"

static bool plx_clang(const char* const input_filename,
                      const char* const output_filename,
                      const enum plx_compile_mode mode) {
  // Check for Clang.
  if (system("clang --version") == -1) {
    plx_error("clang is required to use the LLVM back end");
    return false;
  }

  // Run Clang.
  switch (mode) {
    case PLX_COMPILE_MODE_RELEASE:
      return execlp("clang", "-Wall", input_filename, "-o", output_filename,
                    "-O3", "-ffast-math", (char*)NULL) != -1;
    case PLX_COMPILE_MODE_DEBUG:
      return execlp("clang", "-Wall", input_filename, "-o", output_filename,
                    "-O0", (char*)NULL) != -1;
  }
  return false;
}

bool plx_compile(const char* const input_dir, const char* const output_dir,
                 const enum plx_compile_mode mode,
                 const enum plx_back_end back_end) {
  struct plx_node* const module = plx_new_node(PLX_NODE_MODULE, /*loc=*/NULL);
  struct plx_node** next = &module->children;
  bool result = true;

  // Iterate over files in the input directory.
  struct plx_dir dir;
  if (!plx_dir_open(&dir, input_dir)) {
    plx_error("could not open directory `%s`", input_dir);
    return false;
  }
  const char* input_base_name;
  bool is_dir;
  while ((input_base_name = plx_dir_read(&dir, &is_dir)) != NULL) {
    // Skip directories.
    if (is_dir) continue;

    // Skip non-PLX files.
    const char* const ext = plx_path_ext(input_base_name);
    if (strcmp(ext, PLX_FILE_EXT) != 0) continue;

    // Create the filename.
    char input_filename[PLX_PATH_MAX];
    if (plx_unlikely(snprintf(input_filename, sizeof(input_filename), "%s/%s",
                              input_dir, input_base_name) < 0)) {
      plx_dir_close(&dir);
      return false;
    }

    // Open the file.
    FILE* const stream = fopen(input_filename, "rb");
    if (plx_unlikely(stream == NULL)) continue;

    // Parse the file.
    struct plx_tokenizer tokenizer;
    plx_tokenizer_init(&tokenizer, input_filename, stream);
    struct plx_node* submodule = plx_parse_module(&tokenizer);
    if (plx_unlikely(submodule == NULL)) {
      plx_unexpected_token(&tokenizer);
      fclose(stream);
      result = false;
      continue;
    }

    // Merge the module.
    *next = submodule->children;
    while (*next != NULL) next = &(*next)->next;

    // Close the file.
    fclose(stream);
  }
  plx_dir_close(&dir);

  // Stop on error.
  if (!result) return false;

  // Name resolution
  struct plx_symbol_table symbol_table = PLX_SYMBOL_TABLE_INIT;
  if (!plx_resolve_names(module, &symbol_table)) result = false;

  // Type checking
  if (!plx_type_check(module, /*return_type=*/NULL)) result = false;

  // Return checking
  if (!plx_check_returns(module)) result = false;

  // Stop on error.
  if (!result) return false;

  // Constant folding
  while (plx_fold_constants(module)) {
  }

  // AST validation
  if (!plx_validate_ast(module)) return false;

  // Determine the output name.
  char full_output_dir[PLX_PATH_MAX];
  if (!plx_path_full(output_dir, full_output_dir)) {
    plx_error("could not find directory `%s`", output_dir);
    return false;
  }
  const char* const output_name = plx_path_base(full_output_dir);

  // Code generation
  switch (back_end) {
    case PLX_BACK_END_LLVM: {
      char tmp_filename[PLX_PATH_MAX];
      if (plx_unlikely(snprintf(tmp_filename, sizeof(tmp_filename), "%s/%s.ll",
                                output_dir, output_name) < 0)) {
        return false;
      }
      FILE* const stream = fopen(tmp_filename, "wb");
      if (plx_unlikely(stream == NULL)) {
        plx_error("could not open file `%s`", tmp_filename);
        return false;
      }
      plx_generate_llvm_ir(module, stream);
      fclose(stream);
      char output_filename[PLX_PATH_MAX];
      if (plx_unlikely(snprintf(output_filename, sizeof(output_filename),
                                "%s/%s.exe", output_dir, output_name) < 0)) {
        return false;
      }
      return plx_clang(tmp_filename, output_filename, mode);
    }
    case PLX_BACK_END_WASM: {
      char output_filename[PLX_PATH_MAX];
      if (plx_unlikely(snprintf(output_filename, sizeof(output_filename),
                                "%s/%s.wasm", output_dir, output_name) < 0)) {
        return false;
      }
      FILE* const stream = fopen(output_filename, "wb");
      if (plx_unlikely(stream == NULL)) {
        plx_error("could not open file `%s`", output_filename);
        return false;
      }
      const bool result = plx_generate_wasm(module, stream);
      fclose(stream);
      return result;
    }
  }

  return false;
}
