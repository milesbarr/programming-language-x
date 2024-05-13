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

#ifndef PLX_WASM_H
#define PLX_WASM_H

#include <stddef.h>
#include <stdio.h>

// WebAssembly value types
// https://webassembly.github.io/spec/core/binary/types.html
enum plx_wasm_value_type {
  // Number types
  PLX_WASM_I32 = 0x7F,
  PLX_WASM_I64 = 0x7E,
  PLX_WASM_F32 = 0x7D,
  PLX_WASM_F64 = 0x7C,

  // Vector types
  PLX_WASM_V128 = 0x7B,

  // Reference types
  PLX_WASM_FUNC_REF = 0x70,
  PLX_WASM_EXTERN_REF = 0x6F,
};

// https://webassembly.github.io/spec/core/binary/instructions.html#control-instructions
enum plx_wasm_block_type {
  PLX_WASM_BLOCK_TYPE_EMPTY = 0x40,
};

// WebAssembly instructions
// https://webassembly.github.io/spec/core/binary/instructions.html
enum plx_wasm_instruction {
  // Control instructions
  PLX_WASM_UNREACHABLE = 0x00,
  PLX_WASM_NOP = 0x01,
  PLX_WASM_BLOCK = 0x02,
  PLX_WASM_LOOP = 0x03,
  PLX_WASM_IF = 0x04,
  PLX_WASM_ELSE = 0x05,
  PLX_WASM_END = 0x0B,
  PLX_WASM_BR = 0x0C,
  PLX_WASM_BR_IF = 0x0D,
  PLX_WASM_BR_TABLE = 0x0E,
  PLX_WASM_RETURN = 0x0F,
  PLX_WASM_CALL = 0x10,
  PLX_WASM_CALL_INDIRECT = 0x11,

  // Reference instructions
  PLX_WASM_REF_NULL = 0xD0,
  PLX_WASM_REF_IS_NULL = 0xD1,
  PLX_WASM_REF_FUNC = 0xD2,

  // Parametric instructions
  PLX_WASM_DROP = 0x1A,
  PLX_WASM_SELECT = 0x1B,
  PLX_WASM_SELECT_TYPE = 0x1C,

  // Variable instructions
  PLX_WASM_LOCAL_GET = 0x20,
  PLX_WASM_LOCAL_SET = 0x21,
  PLX_WASM_LOCAL_TEE = 0x22,
  PLX_WASM_GLOBAL_GET = 0x23,
  PLX_WASM_GLOBAL_SET = 0x24,

  // Table instructions
  PLX_WASM_TABLE_GET = 0x25,
  PLX_WASM_TABLE_SET = 0x26,

  // Memory instructions
  PLX_WASM_I32_LOAD = 0x28,
  PLX_WASM_I64_LOAD = 0x29,
  PLX_WASM_F32_LOAD = 0x2A,
  PLX_WASM_F64_LOAD = 0x2B,
  PLX_WASM_I32_LOAD8S = 0x2C,
  PLX_WASM_I32_LOAD8U = 0x2D,
  PLX_WASM_I32_LOAD16S = 0x2E,
  PLX_WASM_I32_LOAD16U = 0x2F,
  PLX_WASM_I64_LOAD8S = 0x30,
  PLX_WASM_I64_LOAD8U = 0x31,
  PLX_WASM_I64_LOAD16S = 0x32,
  PLX_WASM_I64_LOAD16U = 0x33,
  PLX_WASM_I64_LOAD32S = 0x34,
  PLX_WASM_I64_LOAD32U = 0x35,
  PLX_WASM_I32_STORE = 0x36,
  PLX_WASM_I64_STORE = 0x37,
  PLX_WASM_F32_STORE = 0x38,
  PLX_WASM_F64_STORE = 0x39,
  PLX_WASM_I32_STORE8 = 0x3A,
  PLX_WASM_I32_STORE16 = 0x3B,
  PLX_WASM_I64_STORE8 = 0x3C,
  PLX_WASM_I64_STORE16 = 0x3D,
  PLX_WASM_I64_STORE32 = 0x3E,

  // Numeric instructions
  PLX_WASM_I32_CONST = 0x41,
  PLX_WASM_I64_CONST = 0x42,
  PLX_WASM_F32_CONST = 0x43,
  PLX_WASM_F64_CONST = 0x44,

  // i32
  PLX_WASM_I32_EQZ = 0x45,
  PLX_WASM_I32_EQ = 0x46,
  PLX_WASM_I32_NE = 0x47,
  PLX_WASM_I32_LT_S = 0x48,
  PLX_WASM_I32_LT_U = 0x49,
  PLX_WASM_I32_GT_S = 0x4A,
  PLX_WASM_I32_GT_U = 0x4B,
  PLX_WASM_I32_LE_S = 0x4C,
  PLX_WASM_I32_LE_U = 0x4D,
  PLX_WASM_I32_GE_S = 0x4E,
  PLX_WASM_I32_GE_U = 0x4F,

  // i64
  PLX_WASM_I64_EQZ = 0x50,
  PLX_WASM_I64_EQ = 0x51,
  PLX_WASM_I64_NE = 0x52,
  PLX_WASM_I64_LT_S = 0x53,
  PLX_WASM_I64_LT_U = 0x54,
  PLX_WASM_I64_GT_S = 0x55,
  PLX_WASM_I64_GT_U = 0x56,
  PLX_WASM_I64_LE_S = 0x57,
  PLX_WASM_I64_LE_U = 0x58,
  PLX_WASM_I64_GE_S = 0x59,
  PLX_WASM_I64_GE_U = 0x5A,

  // f32
  PLX_WASM_F32_EQ = 0x5B,
  PLX_WASM_F32_NE = 0x5C,
  PLX_WASM_F32_LT = 0x5D,
  PLX_WASM_F32_GT = 0x5E,
  PLX_WASM_F32_LE = 0x5F,
  PLX_WASM_F32_GE = 0x60,

  // f64
  PLX_WASM_F64_EQ = 0x61,
  PLX_WASM_F64_NE = 0x62,
  PLX_WASM_F64_LT = 0x63,
  PLX_WASM_F64_GT = 0x64,
  PLX_WASM_F64_LE = 0x65,
  PLX_WASM_F64_GE = 0x66,

  // i32
  PLX_WASM_I32_CLZ = 0x67,
  PLX_WASM_I32_CTZ = 0x68,
  PLX_WASM_I32_POPCNT = 0x69,
  PLX_WASM_I32_ADD = 0x6A,
  PLX_WASM_I32_SUB = 0x6B,
  PLX_WASM_I32_MUL = 0x6C,
  PLX_WASM_I32_DIV_S = 0x6D,
  PLX_WASM_I32_DIV_U = 0x6E,
  PLX_WASM_I32_REM_S = 0x6F,
  PLX_WASM_I32_REM_U = 0x70,
  PLX_WASM_I32_AND = 0x71,
  PLX_WASM_I32_OR = 0x72,
  PLX_WASM_I32_XOR = 0x73,
  PLX_WASM_I32_SHL = 0x74,
  PLX_WASM_I32_SHR_S = 0x75,
  PLX_WASM_I32_SHR_U = 0x76,
  PLX_WASM_I32_ROTL = 0x77,
  PLX_WASM_I32_ROTR = 0x78,

  // i64
  PLX_WASM_I64_CLS = 0x79,
  PLX_WASM_I64_CTZ = 0x7A,
  PLX_WASM_I64_POPCNT = 0x7B,
  PLX_WASM_I64_ADD = 0x7C,
  PLX_WASM_I64_SUB = 0x7D,
  PLX_WASM_I64_MUL = 0x7E,
  PLX_WASM_I64_DIV_S = 0x7F,
  PLX_WASM_I64_DIV_U = 0x80,
  PLX_WASM_I64_REM_S = 0x81,
  PLX_WASM_I64_REM_U = 0x82,
  PLX_WASM_I64_AND = 0x83,
  PLX_WASM_I64_OR = 0x84,
  PLX_WASM_I64_XOR = 0x85,
  PLX_WASM_I64_SHL = 0x86,
  PLX_WASM_I64_SHR_S = 0x87,
  PLX_WASM_I64_SHR_U = 0x88,
  PLX_WASM_I64_ROTL = 0x89,
  PLX_WASM_I64_ROTR = 0x8A,

  // f32
  PLX_WASM_F32_ABS = 0x8B,
  PLX_WASM_F32_NEG = 0x8C,
  PLX_WASM_F32_CEIL = 0x8D,
  PLX_WASM_F32_FLOOR = 0x8E,
  PLX_WASM_F32_TRUNC = 0x8F,
  PLX_WASM_F32_NEAREST = 0x90,
  PLX_WASM_F32_SQRT = 0x91,
  PLX_WASM_F32_ADD = 0x92,
  PLX_WASM_F32_SUB = 0x93,
  PLX_WASM_F32_MUL = 0x94,
  PLX_WASM_F32_DIV = 0x95,
  PLX_WASM_F32_MIN = 0x96,
  PLX_WASM_F32_MAX = 0x97,
  PLX_WASM_F32_COPYSIGN = 0x98,

  // f64
  PLX_WASM_F64_ABS = 0x99,
  PLX_WASM_F64_NEG = 0x9A,
  PLX_WASM_F64_CEIL = 0x9B,
  PLX_WASM_F64_FLOOR = 0x9C,
  PLX_WASM_F64_TRUNC = 0x9D,
  PLX_WASM_F64_NEAREST = 0x9E,
  PLX_WASM_F64_SQRT = 0x9F,
  PLX_WASM_F64_ADD = 0xA0,
  PLX_WASM_F64_SUB = 0xA1,
  PLX_WASM_F64_MUL = 0xA2,
  PLX_WASM_F64_DIV = 0xA3,
  PLX_WASM_F64_MIN = 0xA4,
  PLX_WASM_F64_MAX = 0xA5,
  PLX_WASM_F64_COPYSIGN = 0xA6,
};

// WebAssembly sections
// https://webassembly.github.io/spec/core/binary/modules.html#sections
enum plx_wasm_section_id {
  PLX_WASM_SECTION_CUSTOM = 0,
  PLX_WASM_SECTION_TYPE = 1,
  PLX_WASM_SECTION_IMPORT = 2,
  PLX_WASM_SECTION_FUNCTION = 3,
  PLX_WASM_SECTION_TABLE = 4,
  PLX_WASM_SECTION_MEMORY = 5,
  PLX_WASM_SECTION_GLOBAL = 6,
  PLX_WASM_SECTION_EXPORT = 7,
  PLX_WASM_SECTION_START = 8,
  PLX_WASM_SECTION_ELEMENT = 9,
  PLX_WASM_SECTION_CODE = 10,
  PLX_WASM_SECTION_DATA = 11,
  PLX_WASM_SECTION_DATA_COUNT = 12,
};

// Writes a WebAssembly module preamble to the output stream.
// https://webassembly.github.io/spec/core/binary/modules.html#binary-module
void plx_wasm_write_module_preamble(FILE* stream);

// Writes a WebAssembly section header to the output stream.
// https://webassembly.github.io/spec/core/binary/modules.html#sections
void plx_wasm_write_section_header(FILE* stream, enum plx_wasm_section_id id,
                                   size_t size);

// Writes a WebAssembly unsigned integer to the output stream.
// https://webassembly.github.io/spec/core/binary/values.html#integers
void plx_wasm_write_ull(FILE* stream, unsigned long long value);

// Writes a WebAssembly signed integer to the output stream.
// https://webassembly.github.io/spec/core/binary/values.html#integers
void plx_wasm_write_ll(FILE* stream, long long value);

// Writes a WebAssembly name to the output stream.
// https://webassembly.github.io/spec/core/binary/values.html#names
void plx_wasm_write_name(FILE* stream, const char* name);

#endif  // PLX_WASM_H
