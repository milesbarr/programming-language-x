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
#include <limits.h>
#include <stdbool.h>

void plx_write_leb128_ull(FILE* const stream, unsigned long long value) {
  do {
    unsigned char byte = (unsigned char)(value & 0b01111111);
    value >>= 7;
    if (value != 0) byte |= 0b10000000;
    fwrite(&byte, sizeof(byte), 1, stream);
  } while (value != 0);
}

void plx_write_leb128_ll(FILE* const stream, long long value) {
  // Requires two's complement.
  assert(-1 == ~0);
  const bool IS_ARITHMETIC_RIGHT_SHIFT = -1LL >> 1 == -1LL;
  bool more = true;
  const bool negative = value < 0;
  do {
    unsigned char byte = (unsigned char)(value & 0b01111111);
    value >>= 7;
    if (!IS_ARITHMETIC_RIGHT_SHIFT) {
      if (negative) value |= ~0LL << (sizeof(value) * CHAR_BIT - 7);
    }
    if ((value == 0 && (byte & 0b01000000) == 0) ||
        (value == -1 && (byte & 0b01000000) != 0)) {
      more = false;
    } else {
      byte |= 0b10000000;
    }
    fwrite(&byte, sizeof(byte), 1, stream);
  } while (more);
}
