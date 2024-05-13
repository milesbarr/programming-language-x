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
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "error.h"
#include "macros.h"
#include "source_code_printer.h"

static void plx_skip_whitespace_and_comments(
    struct plx_tokenizer* const tokenizer) {
  struct plx_reader* const reader = &tokenizer->reader;
  while (plx_peek_char(reader) != EOF) {
    // Skip whitespace.
    if (isspace(plx_peek_char(reader))) {
      plx_next_char(reader);
      continue;
    }

    // Skip comments.
    if (plx_accept_char(reader, '#')) {
      while (plx_peek_char(reader) != EOF && plx_read_char(reader) != '\n') {
      }
      continue;
    }
    break;
  }
}

static void plx_append_char(struct plx_tokenizer* const tokenizer,
                            const char c) {
  assert(tokenizer->len <= tokenizer->cap);
  if (plx_unlikely(tokenizer->len == tokenizer->cap)) {
    size_t cap = tokenizer->cap * 2;
    if (plx_unlikely(cap == 0)) cap = 256;
    void* const str = realloc(tokenizer->str, cap * sizeof(char));
    if (plx_unlikely(str == NULL)) plx_oom();
    tokenizer->cap = cap;
    tokenizer->str = str;
  }
  tokenizer->str[tokenizer->len++] = c;
}

void plx_tokenizer_init(struct plx_tokenizer* const tokenizer,
                        const char* const filename, FILE* const stream) {
  plx_reader_init(&tokenizer->reader, filename, stream);
  tokenizer->token = PLX_TOKEN_EOF;
  tokenizer->loc = tokenizer->reader.loc;
  tokenizer->cap = 0;
  tokenizer->str = NULL;
  plx_next_token(tokenizer);
}

void plx_next_token(struct plx_tokenizer* const tokenizer) {
  if (tokenizer->token == PLX_TOKEN_ERROR) return;

  plx_skip_whitespace_and_comments(tokenizer);

  struct plx_reader* const reader = &tokenizer->reader;

  // Set the location.
  tokenizer->loc = reader->loc;

  // End of file
  if (reader->c == EOF) {
    tokenizer->token = PLX_TOKEN_EOF;
    return;
  }

  // Keywords and identifiers
  if (isalpha(reader->c) || reader->c == '_') {
    tokenizer->len = 0;
    do {
      plx_append_char(tokenizer, plx_read_char(reader));
    } while (isalnum(reader->c));
    plx_append_char(tokenizer, '\0');
    --tokenizer->len;

    switch (tokenizer->len) {
      case 2:
        if (strcmp(tokenizer->str, "if") == 0) {
          tokenizer->token = PLX_TOKEN_IF;
          return;
        }
        if (strcmp(tokenizer->str, "or") == 0) {
          tokenizer->token = PLX_TOKEN_OR;
          return;
        }
        if (strcmp(tokenizer->str, "s8") == 0) {
          tokenizer->token = PLX_TOKEN_S8;
          return;
        }
        if (strcmp(tokenizer->str, "u8") == 0) {
          tokenizer->token = PLX_TOKEN_U8;
          return;
        }
        break;
      case 3:
        if (strcmp(tokenizer->str, "var") == 0) {
          tokenizer->token = PLX_TOKEN_VAR;
          return;
        }
        if (strcmp(tokenizer->str, "for") == 0) {
          tokenizer->token = PLX_TOKEN_FOR;
          return;
        }
        if (strcmp(tokenizer->str, "and") == 0) {
          tokenizer->token = PLX_TOKEN_AND;
          return;
        }
        if (strcmp(tokenizer->str, "xor") == 0) {
          tokenizer->token = PLX_TOKEN_XOR;
          return;
        }
        if (strcmp(tokenizer->str, "s16") == 0) {
          tokenizer->token = PLX_TOKEN_S16;
          return;
        }
        if (strcmp(tokenizer->str, "s32") == 0) {
          tokenizer->token = PLX_TOKEN_S32;
          return;
        }
        if (strcmp(tokenizer->str, "s64") == 0) {
          tokenizer->token = PLX_TOKEN_S64;
          return;
        }
        if (strcmp(tokenizer->str, "u16") == 0) {
          tokenizer->token = PLX_TOKEN_U16;
          return;
        }
        if (strcmp(tokenizer->str, "u32") == 0) {
          tokenizer->token = PLX_TOKEN_U32;
          return;
        }
        if (strcmp(tokenizer->str, "u64") == 0) {
          tokenizer->token = PLX_TOKEN_U64;
          return;
        }
        if (strcmp(tokenizer->str, "f16") == 0) {
          tokenizer->token = PLX_TOKEN_F16;
          return;
        }
        if (strcmp(tokenizer->str, "f32") == 0) {
          tokenizer->token = PLX_TOKEN_F32;
          return;
        }
        if (strcmp(tokenizer->str, "f64") == 0) {
          tokenizer->token = PLX_TOKEN_F64;
          return;
        }
        break;
      case 4:
        if (strcmp(tokenizer->str, "func") == 0) {
          tokenizer->token = PLX_TOKEN_FUNC;
          return;
        }
        if (strcmp(tokenizer->str, "else") == 0) {
          tokenizer->token = PLX_TOKEN_ELSE;
          return;
        }
        if (strcmp(tokenizer->str, "loop") == 0) {
          tokenizer->token = PLX_TOKEN_LOOP;
          return;
        }
        if (strcmp(tokenizer->str, "bool") == 0) {
          tokenizer->token = PLX_TOKEN_BOOL;
          return;
        }
        if (strcmp(tokenizer->str, "true") == 0) {
          tokenizer->token = PLX_TOKEN_TRUE;
          return;
        }
        break;
      case 5:
        if (strcmp(tokenizer->str, "const") == 0) {
          tokenizer->token = PLX_TOKEN_CONST;
          return;
        }
        if (strcmp(tokenizer->str, "defer") == 0) {
          tokenizer->token = PLX_TOKEN_DEFER;
          return;
        }
        if (strcmp(tokenizer->str, "while") == 0) {
          tokenizer->token = PLX_TOKEN_WHILE;
          return;
        }
        if (strcmp(tokenizer->str, "break") == 0) {
          tokenizer->token = PLX_TOKEN_BREAK;
          return;
        }
        if (strcmp(tokenizer->str, "false") == 0) {
          tokenizer->token = PLX_TOKEN_FALSE;
          return;
        }
        break;
      case 6:
        if (strcmp(tokenizer->str, "struct") == 0) {
          tokenizer->token = PLX_TOKEN_STRUCT;
          return;
        }
        if (strcmp(tokenizer->str, "return") == 0) {
          tokenizer->token = PLX_TOKEN_RETURN;
          return;
        }
        break;
      case 8:
        if (strcmp(tokenizer->str, "continue") == 0) {
          tokenizer->token = PLX_TOKEN_CONTINUE;
          return;
        }
        break;
    }
    tokenizer->token = PLX_TOKEN_IDENTIFIER;
    return;
  }

  // Integer and float literals
  if (isdigit(reader->c)) {
    unsigned long long uint = 0;
    if (plx_accept_char(reader, '0')) {
      // Hex literals
      if (plx_accept_char(reader, 'x')) {
        if (!isxdigit(reader->c)) {
          tokenizer->token = PLX_TOKEN_ERROR;
          return;
        }
        do {
          uint *= 16;
          uint += isdigit(reader->c) ? reader->c - '0' : reader->c - 'A' + 10;
          plx_next_char(reader);
        } while (isxdigit(reader->c));

        if (isalnum(reader->c)) {
          tokenizer->token = PLX_TOKEN_ERROR;
          return;
        }

        tokenizer->token = PLX_TOKEN_INT;
        tokenizer->uint = uint;
        return;
      }

      // Binary literals
      if (plx_accept_char(reader, 'b')) {
        if (reader->c == EOF || (reader->c != '0' && reader->c != '1')) {
          tokenizer->token = PLX_TOKEN_ERROR;
          return;
        }

        do {
          uint *= 2;
          uint += plx_read_char(reader) - '0';
        } while (reader->c == '0' || reader->c == '1');

        if (isalnum(reader->c)) {
          tokenizer->token = PLX_TOKEN_ERROR;
          return;
        }

        tokenizer->token = PLX_TOKEN_INT;
        tokenizer->uint = uint;
        return;
      }
    }

    // Decimal literals
    while (isdigit(reader->c)) {
      uint *= 10;
      uint += plx_read_char(reader) - '0';
    }

    // Float literals
    if (plx_accept_char(reader, '.')) {
      unsigned long long fractional = 0;
      unsigned long long divisor = 1;
      if (!isalnum(reader->c)) {
        tokenizer->token = PLX_TOKEN_ERROR;
        return;
      }

      while (isdigit(reader->c)) {
        fractional *= 10;
        fractional += plx_read_char(reader) - '0';
        divisor *= 10;
      }

      if (isalpha(reader->c)) {
        tokenizer->token = PLX_TOKEN_ERROR;
        return;
      }

      tokenizer->token = PLX_TOKEN_FLOAT;
      tokenizer->f = (double)uint + (double)fractional / (double)divisor;
      return;
    }

    if (isalpha(reader->c)) {
      tokenizer->token = PLX_TOKEN_ERROR;
      return;
    }

    tokenizer->token = PLX_TOKEN_INT;
    tokenizer->uint = uint;
    return;
  }

  // Strings literals
  if (plx_accept_char(reader, '"')) {
    tokenizer->len = 0;
    while (!plx_accept_char(reader, '"')) {
      if (reader->c == EOF) {
        tokenizer->token = PLX_TOKEN_ERROR;
        return;
      }

      // Character escapes
      if (plx_accept_char(reader, '\\')) {
        if (reader->c == EOF) {
          tokenizer->token = PLX_TOKEN_ERROR;
          return;
        }

        switch (reader->c) {
          // Double quote escapes
          case '"':
          // Backslash escapes
          case '\\':
            plx_append_char(tokenizer, plx_read_char(reader));
            break;
          // Newline escapes
          case 'n':
            plx_next_char(reader);
            plx_append_char(tokenizer, '\n');
            break;
          // Carriage return escapes
          case 'r':
            plx_next_char(reader);
            plx_append_char(tokenizer, '\r');
            break;
          // Tab escapes
          case 't':
            plx_next_char(reader);
            plx_append_char(tokenizer, '\t');
            break;
          // Null escapes
          case '0':
            plx_next_char(reader);
            plx_append_char(tokenizer, '\0');
            break;
          // Whitespace escapes
          case ' ':
          case '\t':
          case '\n':
          case '\v':
          case '\f':
          case '\r':
            do {
              plx_next_char(reader);
            } while (isspace(reader->c));
            break;
          default:
            tokenizer->token = PLX_TOKEN_ERROR;
            return;
        }
      } else {
        plx_append_char(tokenizer, plx_read_char(reader));
      }
    }

    tokenizer->token = PLX_TOKEN_STRING;
    return;
  }

  // Characters and operators
  switch (reader->c) {
    case '.':
      plx_next_char(reader);
      tokenizer->token = PLX_TOKEN_PERIOD;
      return;
    case ',':
      plx_next_char(reader);
      tokenizer->token = PLX_TOKEN_COMMA;
      return;
    case ':':
      plx_next_char(reader);
      tokenizer->token = PLX_TOKEN_COLON;
      return;
    case ';':
      plx_next_char(reader);
      tokenizer->token = PLX_TOKEN_SEMICOLON;
      return;
    case '(':
      plx_next_char(reader);
      tokenizer->token = PLX_TOKEN_OPEN_PAREN;
      return;
    case ')':
      plx_next_char(reader);
      tokenizer->token = PLX_TOKEN_CLOSE_PAREN;
      return;
    case '[':
      plx_next_char(reader);
      tokenizer->token = PLX_TOKEN_OPEN_SQUARE_BRACKET;
      return;
    case ']':
      plx_next_char(reader);
      tokenizer->token = PLX_TOKEN_CLOSE_SQUARE_BRACKET;
      return;
    case '{':
      plx_next_char(reader);
      tokenizer->token = PLX_TOKEN_OPEN_CURLY_BRACE;
      return;
    case '}':
      plx_next_char(reader);
      tokenizer->token = PLX_TOKEN_CLOSE_CURLY_BRACE;
      return;
    case '=':
      plx_next_char(reader);
      tokenizer->token =
          plx_accept_char(reader, '=') ? PLX_TOKEN_EQ : PLX_TOKEN_ASSIGN;
      return;
    case '!':
      plx_next_char(reader);
      tokenizer->token =
          plx_accept_char(reader, '=') ? PLX_TOKEN_NEQ : PLX_TOKEN_NOT;
      return;
    case '<':
      plx_next_char(reader);
      if (plx_accept_char(reader, '>')) {
        tokenizer->token = plx_accept_char(reader, '=')
                               ? PLX_TOKEN_LSHIFT_ASSIGN
                               : PLX_TOKEN_LSHIFT;
      } else {
        tokenizer->token =
            plx_accept_char(reader, '=') ? PLX_TOKEN_LTE : PLX_TOKEN_LT;
      }
      return;
    case '>':
      plx_next_char(reader);
      if (plx_accept_char(reader, '>')) {
        tokenizer->token = plx_accept_char(reader, '=')
                               ? PLX_TOKEN_RSHIFT_ASSIGN
                               : PLX_TOKEN_RSHIFT;
      } else {
        tokenizer->token =
            plx_accept_char(reader, '=') ? PLX_TOKEN_GTE : PLX_TOKEN_GT;
      }
      return;
    case '+':
      plx_next_char(reader);
      tokenizer->token =
          plx_accept_char(reader, '=') ? PLX_TOKEN_ADD_ASSIGN : PLX_TOKEN_ADD;
      return;
    case '-':
      plx_next_char(reader);
      if (plx_accept_char(reader, '>')) {
        tokenizer->token = PLX_TOKEN_ARROW;
      } else if (plx_accept_char(reader, '=')) {
        tokenizer->token = PLX_TOKEN_SUB_ASSIGN;
      } else {
        tokenizer->token = PLX_TOKEN_SUB;
      }
      return;
    case '*':
      plx_next_char(reader);
      tokenizer->token =
          plx_accept_char(reader, '=') ? PLX_TOKEN_MUL_ASSIGN : PLX_TOKEN_MUL;
      return;
    case '/':
      plx_next_char(reader);
      tokenizer->token =
          plx_accept_char(reader, '=') ? PLX_TOKEN_DIV_ASSIGN : PLX_TOKEN_DIV;
      return;
    case '%':
      plx_next_char(reader);
      tokenizer->token =
          plx_accept_char(reader, '=') ? PLX_TOKEN_REM_ASSIGN : PLX_TOKEN_REM;
      return;
    case '&':
      plx_next_char(reader);
      tokenizer->token = PLX_TOKEN_REF;
      return;
  }
  tokenizer->token = PLX_TOKEN_ERROR;
}

enum plx_token plx_read_token(struct plx_tokenizer* const tokenizer) {
  const enum plx_token token = tokenizer->token;
  plx_next_token(tokenizer);
  return token;
}

char* plx_read_identifier_or_string(struct plx_tokenizer* const tokenizer) {
  assert(tokenizer->token == PLX_TOKEN_IDENTIFIER ||
         tokenizer->token == PLX_TOKEN_STRING);
  char* const str = malloc(tokenizer->len + 1);
  if (plx_unlikely(str == NULL)) plx_oom();
  memcpy(str, tokenizer->str, tokenizer->len + 1);
  plx_next_token(tokenizer);
  return str;
}

bool plx_accept_token(struct plx_tokenizer* const tokenizer,
                      const enum plx_token token) {
  if (token == tokenizer->token) {
    plx_next_token(tokenizer);
    return true;
  }
  return false;
}

void plx_unexpected_token(const struct plx_tokenizer* const tokenizer) {
  switch (tokenizer->token) {
    case PLX_TOKEN_EOF:
      plx_error("unexpected end of file");
      break;
    case PLX_TOKEN_ERROR:
      plx_unexpected_character(&tokenizer->reader);
      return;
    case PLX_TOKEN_CONST:
    case PLX_TOKEN_VAR:
    case PLX_TOKEN_STRUCT:
    case PLX_TOKEN_FUNC:
    case PLX_TOKEN_IF:
    case PLX_TOKEN_ELSE:
    case PLX_TOKEN_DEFER:
    case PLX_TOKEN_LOOP:
    case PLX_TOKEN_WHILE:
    case PLX_TOKEN_FOR:
    case PLX_TOKEN_CONTINUE:
    case PLX_TOKEN_BREAK:
    case PLX_TOKEN_RETURN:
    case PLX_TOKEN_AND:
    case PLX_TOKEN_OR:
    case PLX_TOKEN_XOR:
    case PLX_TOKEN_S8:
    case PLX_TOKEN_S16:
    case PLX_TOKEN_S32:
    case PLX_TOKEN_S64:
    case PLX_TOKEN_U8:
    case PLX_TOKEN_U16:
    case PLX_TOKEN_U32:
    case PLX_TOKEN_U64:
    case PLX_TOKEN_F16:
    case PLX_TOKEN_F32:
    case PLX_TOKEN_F64:
    case PLX_TOKEN_BOOL:
    case PLX_TOKEN_TRUE:
    case PLX_TOKEN_FALSE:
    case PLX_TOKEN_IDENTIFIER:
      plx_error("unexpected token `%s`", tokenizer->str);
      break;
    default:
      plx_error("unexpected token");
  }
  plx_print_source_code(&tokenizer->loc,
                        /*annotation=*/"this token is unexpected",
                        PLX_SOURCE_ANNOTATION_ERROR);
}
