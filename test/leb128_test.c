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

#include "leb128.h"

#include <assert.h>
#include <stdio.h>

// Tests writing an unsigned integer in LEB128 format using an example sourced
// from Wikipedia.
// https://en.wikipedia.org/wiki/LEB128#Unsigned_LEB128
static void plx_test_write_leb128_ull(void) {
  FILE* const stream = tmpfile();
  assert(stream != NULL);
  plx_write_leb128_ull(stream, 624485U);
  fseek(stream, 0, SEEK_SET);
  assert(fgetc(stream) == 0xE5);
  assert(fgetc(stream) == 0x8E);
  assert(fgetc(stream) == 0x26);
  assert(fgetc(stream) == EOF);
  fclose(stream);
}

// Tests writing an signed integer in LEB128 format using an example sourced
// from Wikipedia.
// https://en.wikipedia.org/wiki/LEB128#Signed_LEB128
static void plx_test_write_leb128_ll(void) {
  FILE* const stream = tmpfile();
  assert(stream != NULL);
  plx_write_leb128_ll(stream, -123456);
  fseek(stream, 0, SEEK_SET);
  assert(fgetc(stream) == 0xC0);
  assert(fgetc(stream) == 0xBB);
  assert(fgetc(stream) == 0x78);
  assert(fgetc(stream) == EOF);
  fclose(stream);
}

void plx_test_leb128(void) {
  plx_test_write_leb128_ull();
  plx_test_write_leb128_ll();
}
