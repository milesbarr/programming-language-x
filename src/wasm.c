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

#include "wasm.h"

#include <string.h>

#include "leb128.h"

void plx_wasm_write_module_preamble(FILE* const stream) {
  const char preamble[] = {// Magic
                           '\0', 'a', 's', 'm',
                           // Version
                           1, 0, 0, 0};
  fwrite(preamble, sizeof(*preamble), sizeof(preamble), stream);
}

void plx_wasm_write_section_header(FILE* const stream,
                                   const enum plx_wasm_section_id id,
                                   const size_t size) {
  fputc(id, stream);
  plx_wasm_write_ull(stream, size);
}

void plx_wasm_write_ull(FILE* const stream, const unsigned long long value) {
  plx_write_leb128_ull(stream, value);
}

void plx_wasm_write_ll(FILE* const stream, const long long value) {
  plx_write_leb128_ll(stream, value);
}

void plx_wasm_write_name(FILE* const stream, const char* const name) {
  const size_t len = strlen(name);
  plx_wasm_write_ull(stream, len);
  fwrite(name, sizeof(*name), len, stream);
}
