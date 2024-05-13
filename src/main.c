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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "compiler.h"
#include "error.h"

static void plx_version(void) { fputs("Programming Language X v1\n", stderr); }

static void plx_usage(const char* const prog) {
  fprintf(stderr,
          "Usage: %s [-h | --help] [-v | --version] [path] [-o <path> | "
          "--output <path>] [-d | --debug] [-b <back-end> | --back-end "
          "<back-end>]\n",
          prog);
}

int main(const int argc, const char* argv[]) {
  const char* input_dir = NULL;
  const char* output_dir = ".";
  enum plx_compile_mode mode = PLX_COMPILE_MODE_RELEASE;
  enum plx_back_end back_end = PLX_BACK_END_LLVM;
  for (int i = 1; i < argc; ++i) {
    const char* const arg = argv[i];
    if (strcmp(arg, "-v") == 0 || strcmp(arg, "--version") == 0) {
      plx_version();
      return EXIT_SUCCESS;
    }
    if (strcmp(arg, "-h") == 0 || strcmp(arg, "--help") == 0) {
      plx_usage(argv[0]);
      return EXIT_SUCCESS;
    }
    if ((strcmp(arg, "-o") == 0 || strcmp(arg, "--output") == 0) &&
        i + 1 < argc) {
      output_dir = argv[++i];
      continue;
    }
    if (strcmp(arg, "-d") == 0 || strcmp(arg, "--debug") == 0) {
      mode = PLX_COMPILE_MODE_DEBUG;
      continue;
    }
    if ((strcmp(arg, "-b") == 0 || strcmp(arg, "--back-end") == 0) &&
        i + 1 < argc) {
      const char* const s = argv[++i];
      if (strcmp(s, "llvm") == 0) {
        back_end = PLX_BACK_END_LLVM;
      } else if (strcmp(s, "wasm") == 0) {
        back_end = PLX_BACK_END_WASM;
      } else {
        plx_error("unknown back end `%s`", s);
        return EXIT_FAILURE;
      }
      continue;
    }
    if (arg[0] == '-' || input_dir != NULL) {
      plx_error("unexpected argument `%s`", arg);
      return EXIT_FAILURE;
    }
    input_dir = argv[i];
  }
  input_dir = input_dir != NULL ? input_dir : ".";
  return plx_compile(input_dir, output_dir, mode, back_end) ? EXIT_SUCCESS
                                                            : EXIT_FAILURE;
}
