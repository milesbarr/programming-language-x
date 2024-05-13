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

#ifndef PLX_COMPILER_H
#define PLX_COMPILER_H

#include <stdbool.h>

enum plx_compile_mode {
  PLX_COMPILE_MODE_RELEASE,
  PLX_COMPILE_MODE_DEBUG,
};

enum plx_back_end {
  PLX_BACK_END_LLVM,
  PLX_BACK_END_WASM,
};

bool plx_compile(const char* input_dir, const char* output_dir,
                 enum plx_compile_mode mode, enum plx_back_end back_end);

#endif  // PLX_COMPILER_H
