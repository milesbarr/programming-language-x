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

#include "reader.h"

#include "error.h"
#include "source_code_printer.h"

void plx_reader_init(struct plx_reader* const reader,
                     const char* const filename, FILE* const stream) {
  reader->stream = stream;
  reader->loc = (struct plx_source_code_location){filename, 0, 0, 0};
  reader->c = '\n';
  plx_next_char(reader);
}

void plx_next_char(struct plx_reader* const reader) {
  if (reader->c == '\n') {
    reader->loc.line_pos = ftell(reader->stream);
    ++reader->loc.line;
    reader->loc.col = 1;
  } else {
    ++reader->loc.col;
  }
  reader->c = fgetc(reader->stream);
}

int plx_peek_char(const struct plx_reader* const reader) { return reader->c; }

int plx_read_char(struct plx_reader* const reader) {
  const int c = reader->c;
  plx_next_char(reader);
  return c;
}

bool plx_accept_char(struct plx_reader* const reader, const char c) {
  if (c == reader->c) {
    plx_next_char(reader);
    return true;
  }
  return false;
}

void plx_unexpected_character(const struct plx_reader* const reader) {
  if (reader->c == EOF) {
    plx_error("unexpected end of file");
  } else {
    plx_error("unexpected character `%c`", reader->c);
  }
  plx_print_source_code(&reader->loc,
                        /*annotation=*/"this character is unexpected",
                        PLX_SOURCE_ANNOTATION_ERROR);
}
