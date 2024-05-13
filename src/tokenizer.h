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

#ifndef PLX_TOKENIZER_H
#define PLX_TOKENIZER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

#include "reader.h"
#include "source_code_location.h"

enum plx_token {
  PLX_TOKEN_EOF,
  PLX_TOKEN_ERROR,

  // Keywords
  PLX_TOKEN_CONST,
  PLX_TOKEN_VAR,
  PLX_TOKEN_STRUCT,
  PLX_TOKEN_FUNC,
  PLX_TOKEN_IF,
  PLX_TOKEN_ELSE,
  PLX_TOKEN_DEFER,
  PLX_TOKEN_LOOP,
  PLX_TOKEN_WHILE,
  PLX_TOKEN_FOR,
  PLX_TOKEN_CONTINUE,
  PLX_TOKEN_BREAK,
  PLX_TOKEN_RETURN,
  PLX_TOKEN_AND,
  PLX_TOKEN_OR,
  PLX_TOKEN_XOR,
  PLX_TOKEN_S8,
  PLX_TOKEN_S16,
  PLX_TOKEN_S32,
  PLX_TOKEN_S64,
  PLX_TOKEN_U8,
  PLX_TOKEN_U16,
  PLX_TOKEN_U32,
  PLX_TOKEN_U64,
  PLX_TOKEN_F16,
  PLX_TOKEN_F32,
  PLX_TOKEN_F64,
  PLX_TOKEN_BOOL,
  PLX_TOKEN_TRUE,
  PLX_TOKEN_FALSE,

  // Identifiers
  PLX_TOKEN_IDENTIFIER,

  // Literals
  PLX_TOKEN_INT,
  PLX_TOKEN_FLOAT,
  PLX_TOKEN_STRING,

  // Characters
  PLX_TOKEN_PERIOD,
  PLX_TOKEN_COMMA,
  PLX_TOKEN_COLON,
  PLX_TOKEN_SEMICOLON,
  PLX_TOKEN_OPEN_PAREN,
  PLX_TOKEN_CLOSE_PAREN,
  PLX_TOKEN_OPEN_SQUARE_BRACKET,
  PLX_TOKEN_CLOSE_SQUARE_BRACKET,
  PLX_TOKEN_OPEN_CURLY_BRACE,
  PLX_TOKEN_CLOSE_CURLY_BRACE,

  // Operators
  PLX_TOKEN_EQ,
  PLX_TOKEN_ASSIGN,
  PLX_TOKEN_NEQ,
  PLX_TOKEN_NOT,
  PLX_TOKEN_LSHIFT_ASSIGN,
  PLX_TOKEN_LSHIFT,
  PLX_TOKEN_LTE,
  PLX_TOKEN_LT,
  PLX_TOKEN_RSHIFT_ASSIGN,
  PLX_TOKEN_RSHIFT,
  PLX_TOKEN_GTE,
  PLX_TOKEN_GT,
  PLX_TOKEN_ADD_ASSIGN,
  PLX_TOKEN_ADD,
  PLX_TOKEN_ARROW,
  PLX_TOKEN_SUB_ASSIGN,
  PLX_TOKEN_SUB,
  PLX_TOKEN_MUL_ASSIGN,
  PLX_TOKEN_MUL,
  PLX_TOKEN_DIV_ASSIGN,
  PLX_TOKEN_DIV,
  PLX_TOKEN_REM_ASSIGN,
  PLX_TOKEN_REM,
  PLX_TOKEN_REF,
};

struct plx_tokenizer {
  struct plx_reader reader;
  struct plx_source_code_location loc;
  enum plx_token token;
  union {
    unsigned long long uint;
    double f;
    size_t len;
  };
  size_t cap;
  char* str;
};

void plx_tokenizer_init(struct plx_tokenizer* tokenizer, const char* filename,
                        FILE* stream);
void plx_next_token(struct plx_tokenizer* tokenizer);
enum plx_token plx_read_token(struct plx_tokenizer* const tokenizer);
char* plx_read_identifier_or_string(struct plx_tokenizer* tokenizer);
bool plx_accept_token(struct plx_tokenizer* tokenizer,
                      const enum plx_token token);
void plx_unexpected_token(const struct plx_tokenizer* tokenizer);

#endif  // PLX_TOKENIZER_H
