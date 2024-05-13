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

#ifndef PLX_LEB128_H
#define PLX_LEB128_H

#include <stdio.h>

// Writes an unsigned integer to an output stream in LEB128 format.
// https://en.wikipedia.org/wiki/LEB128#Encode_unsigned_integer
void plx_write_leb128_ull(FILE* stream, unsigned long long value);

// Writes a signed integer to an output stream in LEB128 format.
// https://en.wikipedia.org/wiki/LEB128#Encode_signed_integer
void plx_write_leb128_ll(FILE* stream, long long value);

#endif  // PLX_LEB128_H
