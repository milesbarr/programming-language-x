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

#include "source_code_printer.h"

#include <stdio.h>

#include "ansi_escape_codes.h"

void plx_print_source_code(
    const struct plx_source_code_location* const loc,
    const char* const annotation,
    const enum plx_source_annotation_style annotation_style) {
  const bool ansi_escape_codes_enabled = plx_enable_ansi_escape_codes_stderr();

  // Print the file name.
  if (ansi_escape_codes_enabled) fputs(PLX_ANSI_FOREGROUND_BRIGHT_CYAN, stderr);
  fprintf(stderr, "%s:%d:%d\n", loc->filename, loc->line, loc->col);
  if (ansi_escape_codes_enabled) fputs(PLX_ANSI_RESET, stderr);

  // Open the file.
  FILE* const stream = fopen(loc->filename, "r");
  if (stream == NULL) return;

  // Seek to the line.
  if (fseek(stream, loc->line_pos, SEEK_SET) != 0) return;

  // Print the line number.
  if (ansi_escape_codes_enabled) fputs(PLX_ANSI_FOREGROUND_BRIGHT_CYAN, stderr);
  fprintf(stderr, "%d | ", loc->line);
  if (ansi_escape_codes_enabled) fputs(PLX_ANSI_RESET, stderr);

  // Print the line.
  int c;
  while ((c = fgetc(stream)) != EOF && c != '\n') fputc(c, stderr);
  fputc('\n', stderr);

  // Count the number of digits in the line number.
  unsigned int line = loc->line;
  int line_number_digits = 0;
  do {
    ++line_number_digits;
  } while ((line /= 10) != 0);

  // Print the annotation.
  fprintf(stderr, "%*s", line_number_digits + 2 + loc->col, "");
  if (annotation_style == PLX_SOURCE_ANNOTATION_ERROR &&
      ansi_escape_codes_enabled) {
    fputs(PLX_ANSI_FOREGROUND_BRIGHT_RED, stderr);
  }
  fputc('^', stderr);
  if (annotation != NULL) fprintf(stderr, " %s\n", annotation);
  if (ansi_escape_codes_enabled) fputs(PLX_ANSI_RESET, stderr);
  fputc('\n', stderr);
}
