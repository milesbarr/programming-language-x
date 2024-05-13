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

#include "tokenizer.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void plx_test_tokenizer_keywords(void) {
  FILE* const stream = tmpfile();
  assert(stream != NULL);
  fputs(
      "const var struct func if else defer loop while for continue break "
      "return and or xor s8 s16 s32 s64 u8 u16 u32 u64 f16 f32 f64 bool true "
      "false",
      stream);
  fseek(stream, 0, SEEK_SET);

  struct plx_tokenizer tokenizer;
  plx_tokenizer_init(&tokenizer, /*filename=*/"<test>", stream);
  assert(plx_read_token(&tokenizer) == PLX_TOKEN_CONST);
  assert(plx_read_token(&tokenizer) == PLX_TOKEN_VAR);
  assert(plx_read_token(&tokenizer) == PLX_TOKEN_STRUCT);
  assert(plx_read_token(&tokenizer) == PLX_TOKEN_FUNC);
  assert(plx_read_token(&tokenizer) == PLX_TOKEN_IF);
  assert(plx_read_token(&tokenizer) == PLX_TOKEN_ELSE);
  assert(plx_read_token(&tokenizer) == PLX_TOKEN_DEFER);
  assert(plx_read_token(&tokenizer) == PLX_TOKEN_LOOP);
  assert(plx_read_token(&tokenizer) == PLX_TOKEN_WHILE);
  assert(plx_read_token(&tokenizer) == PLX_TOKEN_FOR);
  assert(plx_read_token(&tokenizer) == PLX_TOKEN_CONTINUE);
  assert(plx_read_token(&tokenizer) == PLX_TOKEN_BREAK);
  assert(plx_read_token(&tokenizer) == PLX_TOKEN_RETURN);
  assert(plx_read_token(&tokenizer) == PLX_TOKEN_AND);
  assert(plx_read_token(&tokenizer) == PLX_TOKEN_OR);
  assert(plx_read_token(&tokenizer) == PLX_TOKEN_XOR);
  assert(plx_read_token(&tokenizer) == PLX_TOKEN_S8);
  assert(plx_read_token(&tokenizer) == PLX_TOKEN_S16);
  assert(plx_read_token(&tokenizer) == PLX_TOKEN_S32);
  assert(plx_read_token(&tokenizer) == PLX_TOKEN_S64);
  assert(plx_read_token(&tokenizer) == PLX_TOKEN_U8);
  assert(plx_read_token(&tokenizer) == PLX_TOKEN_U16);
  assert(plx_read_token(&tokenizer) == PLX_TOKEN_U32);
  assert(plx_read_token(&tokenizer) == PLX_TOKEN_U64);
  assert(plx_read_token(&tokenizer) == PLX_TOKEN_F16);
  assert(plx_read_token(&tokenizer) == PLX_TOKEN_F32);
  assert(plx_read_token(&tokenizer) == PLX_TOKEN_F64);
  assert(plx_read_token(&tokenizer) == PLX_TOKEN_BOOL);
  assert(plx_read_token(&tokenizer) == PLX_TOKEN_TRUE);
  assert(plx_read_token(&tokenizer) == PLX_TOKEN_FALSE);
  assert(plx_read_token(&tokenizer) == PLX_TOKEN_EOF);
  fclose(stream);
}

static void plx_test_tokenizer_identifiers(void) {
  FILE* const stream = tmpfile();
  assert(stream != NULL);
  fputs("foo bar", stream);
  fseek(stream, 0, SEEK_SET);

  struct plx_tokenizer tokenizer;
  plx_tokenizer_init(&tokenizer, /*filename=*/"<test>", stream);
  assert(tokenizer.token == PLX_TOKEN_IDENTIFIER);
  assert(strcmp(tokenizer.str, "foo") == 0);
  plx_next_token(&tokenizer);
  assert(tokenizer.token == PLX_TOKEN_IDENTIFIER);
  assert(strcmp(tokenizer.str, "bar") == 0);
  plx_next_token(&tokenizer);
  assert(tokenizer.token == PLX_TOKEN_EOF);
}

static void plx_test_tokenizer_hex_literals(void) {
  FILE* const stream = tmpfile();
  assert(stream != NULL);
  fputs("0xAB 0xCD", stream);
  fseek(stream, 0, SEEK_SET);

  struct plx_tokenizer tokenizer;
  plx_tokenizer_init(&tokenizer, /*filename=*/"<test>", stream);
  assert(tokenizer.token == PLX_TOKEN_INT);
  assert(tokenizer.uint == 0xAB);
  plx_next_token(&tokenizer);
  assert(tokenizer.token == PLX_TOKEN_INT);
  assert(tokenizer.uint == 0xCD);
  plx_next_token(&tokenizer);
  assert(tokenizer.token == PLX_TOKEN_EOF);
}

static void plx_test_tokenizer_binary_literals(void) {
  FILE* const stream = tmpfile();
  assert(stream != NULL);
  fputs("0b00 0b11", stream);
  fseek(stream, 0, SEEK_SET);

  struct plx_tokenizer tokenizer;
  plx_tokenizer_init(&tokenizer, /*filename=*/"<test>", stream);
  assert(tokenizer.token == PLX_TOKEN_INT);
  assert(tokenizer.uint == 0b00);
  plx_next_token(&tokenizer);
  assert(tokenizer.token == PLX_TOKEN_INT);
  assert(tokenizer.uint == 0b11);
  plx_next_token(&tokenizer);
  assert(tokenizer.token == PLX_TOKEN_EOF);
}

static void plx_test_tokenizer_float_literals(void) {
  FILE* const stream = tmpfile();
  assert(stream != NULL);
  fputs("0.0 1.0", stream);
  fseek(stream, 0, SEEK_SET);

  struct plx_tokenizer tokenizer;
  plx_tokenizer_init(&tokenizer, /*filename=*/"<test>", stream);
  assert(tokenizer.token == PLX_TOKEN_FLOAT);
  assert(tokenizer.f == 0.0);
  plx_next_token(&tokenizer);
  assert(tokenizer.token == PLX_TOKEN_FLOAT);
  assert(tokenizer.f == 1.0);
  plx_next_token(&tokenizer);
  assert(tokenizer.token == PLX_TOKEN_EOF);
}

static void plx_test_tokenizer_decimal_literals(void) {
  FILE* const stream = tmpfile();
  assert(stream != NULL);
  fputs("0 1", stream);
  fseek(stream, 0, SEEK_SET);

  struct plx_tokenizer tokenizer;
  plx_tokenizer_init(&tokenizer, /*filename=*/"<test>", stream);
  assert(tokenizer.token == PLX_TOKEN_INT);
  assert(tokenizer.uint == 0);
  plx_next_token(&tokenizer);
  assert(tokenizer.token == PLX_TOKEN_INT);
  assert(tokenizer.uint == 1);
  plx_next_token(&tokenizer);
  assert(tokenizer.token == PLX_TOKEN_EOF);
}

static void plx_test_tokenizer_strings(void) {
  FILE* const stream = tmpfile();
  assert(stream != NULL);
  fputs("\"Hello, World!\"", stream);
  fseek(stream, 0, SEEK_SET);

  struct plx_tokenizer tokenizer;
  plx_tokenizer_init(&tokenizer, /*filename=*/"<test>", stream);
  assert(tokenizer.token == PLX_TOKEN_STRING);
  assert(strncmp(tokenizer.str, "Hello, World!", tokenizer.len) == 0);
  plx_next_token(&tokenizer);
  assert(tokenizer.token == PLX_TOKEN_EOF);
}

static void plx_test_tokenizer_string_double_quote_escapes(void) {
  FILE* const stream = tmpfile();
  assert(stream != NULL);
  fputs("\"\\\"\"", stream);
  fseek(stream, 0, SEEK_SET);

  struct plx_tokenizer tokenizer;
  plx_tokenizer_init(&tokenizer, /*filename=*/"<test>", stream);
  assert(tokenizer.token == PLX_TOKEN_STRING);
  assert(strncmp(tokenizer.str, "\"", tokenizer.len) == 0);
  plx_next_token(&tokenizer);
  assert(tokenizer.token == PLX_TOKEN_EOF);
}

static void plx_test_tokenizer_backslash_escapes(void) {
  FILE* const stream = tmpfile();
  assert(stream != NULL);
  fputs("\"\\\\\"", stream);
  fseek(stream, 0, SEEK_SET);

  struct plx_tokenizer tokenizer;
  plx_tokenizer_init(&tokenizer, /*filename=*/"<test>", stream);
  assert(tokenizer.token == PLX_TOKEN_STRING);
  assert(strncmp(tokenizer.str, "\\", tokenizer.len) == 0);
  plx_next_token(&tokenizer);
  assert(tokenizer.token == PLX_TOKEN_EOF);
}

static void plx_test_tokenizer_string_newline_escapes(void) {
  FILE* const stream = tmpfile();
  assert(stream != NULL);
  fputs("\"\n\"", stream);
  fseek(stream, 0, SEEK_SET);

  struct plx_tokenizer tokenizer;
  plx_tokenizer_init(&tokenizer, /*filename=*/"<test>", stream);
  assert(tokenizer.token == PLX_TOKEN_STRING);
  assert(strncmp(tokenizer.str, "\n", tokenizer.len) == 0);
  plx_next_token(&tokenizer);
  assert(tokenizer.token == PLX_TOKEN_EOF);
}

static void plx_test_tokenizer_string_carriage_return_escapes(void) {
  FILE* const stream = tmpfile();
  assert(stream != NULL);
  fputs("\"\\r\"", stream);
  fseek(stream, 0, SEEK_SET);

  struct plx_tokenizer tokenizer;
  plx_tokenizer_init(&tokenizer, /*filename=*/"<test>", stream);
  assert(tokenizer.token == PLX_TOKEN_STRING);
  assert(strncmp(tokenizer.str, "\r", tokenizer.len) == 0);
  plx_next_token(&tokenizer);
  assert(tokenizer.token == PLX_TOKEN_EOF);
}

static void plx_test_tokenizer_string_tab_escapes(void) {
  FILE* const stream = tmpfile();
  assert(stream != NULL);
  fputs("\"\\t\"", stream);
  fseek(stream, 0, SEEK_SET);

  struct plx_tokenizer tokenizer;
  plx_tokenizer_init(&tokenizer, /*filename=*/"<test>", stream);
  assert(tokenizer.token == PLX_TOKEN_STRING);
  assert(strncmp(tokenizer.str, "\t", tokenizer.len) == 0);
  plx_next_token(&tokenizer);
  assert(tokenizer.token == PLX_TOKEN_EOF);
}

static void plx_test_tokenizer_string_null_escapes(void) {
  FILE* const stream = tmpfile();
  assert(stream != NULL);
  fputs("\"\\0\"", stream);
  fseek(stream, 0, SEEK_SET);

  struct plx_tokenizer tokenizer;
  plx_tokenizer_init(&tokenizer, /*filename=*/"<test>", stream);
  assert(tokenizer.token == PLX_TOKEN_STRING);
  assert(strncmp(tokenizer.str, "\0", tokenizer.len) == 0);
  plx_next_token(&tokenizer);
  assert(tokenizer.token == PLX_TOKEN_EOF);
}

static void plx_test_tokenizer_string_whitespace_escapes(void) {
  FILE* const stream = tmpfile();
  assert(stream != NULL);
  fputs("\"\\\nHello, World!\"", stream);
  fseek(stream, 0, SEEK_SET);

  struct plx_tokenizer tokenizer;
  plx_tokenizer_init(&tokenizer, /*filename=*/"<test>", stream);
  assert(tokenizer.token == PLX_TOKEN_STRING);
  assert(strncmp(tokenizer.str, "Hello, World!", tokenizer.len) == 0);
  plx_next_token(&tokenizer);
  assert(tokenizer.token == PLX_TOKEN_EOF);
}

void plx_test_tokenizer(void) {
  plx_test_tokenizer_keywords();
  plx_test_tokenizer_identifiers();
  plx_test_tokenizer_hex_literals();
  plx_test_tokenizer_binary_literals();
  plx_test_tokenizer_float_literals();
  plx_test_tokenizer_decimal_literals();
  plx_test_tokenizer_strings();
  plx_test_tokenizer_string_double_quote_escapes();
  plx_test_tokenizer_backslash_escapes();
  plx_test_tokenizer_string_newline_escapes();
  plx_test_tokenizer_string_carriage_return_escapes();
  plx_test_tokenizer_string_tab_escapes();
  plx_test_tokenizer_string_null_escapes();
  plx_test_tokenizer_string_whitespace_escapes();
}
