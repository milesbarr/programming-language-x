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

#include "error.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "ansi_escape_codes.h"

void plx_error(const char* const format, ...) {
  const bool ansi_escape_codes_enabled = plx_enable_ansi_escape_codes_stderr();
  if (ansi_escape_codes_enabled) fputs(PLX_ANSI_FOREGROUND_BRIGHT_RED, stderr);
  fputs("error", stderr);
  if (ansi_escape_codes_enabled) {
    fputs(PLX_ANSI_FOREGROUND_BRIGHT_WHITE, stderr);
  }
  fputs(": ", stderr);
  va_list arg;
  va_start(arg, format);
  vfprintf(stderr, format, arg);
  va_end(arg);
  if (ansi_escape_codes_enabled) fputs(PLX_ANSI_RESET, stderr);
  fputc('\n', stderr);
}

void plx_oom(void) {
  plx_error("out of memory");
  exit(EXIT_FAILURE);
}
