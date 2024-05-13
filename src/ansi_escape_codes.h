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

#ifndef PLX_ANSI_ESCAPE_CODES_H
#define PLX_ANSI_ESCAPE_CODES_H

#include <stdbool.h>

#define PLX_ANSI_RESET "\x1b[0m"
#define PLX_ANSI_BOLD "\x1b[1m"
#define PLX_ANSI_FAINT "\x1b[2m"
#define PLX_ANSI_ITALIC "\x1b[3m"
#define PLX_ANSI_UNDERLINE "\x1b[4m"
#define PLX_ANSI_SLOW_BLINK "\x1b[5m"
#define PLX_ANSI_RAPID_BLINK "\x1b[6m"
#define PLX_ANSI_INVERT "\x1b[7m"
#define PLX_ANSI_STRIKETHROUGH "\x1b[9m"
#define PLX_ANSI_NOT_BOLD "\x1b[21m"
#define PLX_ANSI_NORMAL_INTENSITY "\x1b[22m"
#define PLX_ANSI_NOT_ITALIC "\x1b[23m"
#define PLX_ANSI_NOT_UNDERLINED "\x1b[24m"
#define PLX_ANSI_NOT_BLINKING "\x1b[25m"
#define PLX_ANSI_NOT_INVERTED "\x1b[27m"
#define PLX_ANSI_NO_STRIKETHROUGH "\x1b[29m"

// Foreground
#define PLX_ANSI_FOREGROUND_BLACK "\x1b[30m"
#define PLX_ANSI_FOREGROUND_RED "\x1b[31m"
#define PLX_ANSI_FOREGROUND_GREEN "\x1b[32m"
#define PLX_ANSI_FOREGROUND_YELLOW "\x1b[33m"
#define PLX_ANSI_FOREGROUND_BLUE "\x1b[34m"
#define PLX_ANSI_FOREGROUND_MAGENTA "\x1b[35m"
#define PLX_ANSI_FOREGROUND_CYAN "\x1b[36m"
#define PLX_ANSI_FOREGROUND_WHITE "\x1b[37m"
#define PLX_ANSI_FOREGROUND_DEFAULT "\x1b[39m"
#define PLX_ANSI_FOREGROUND_BRIGHT_BLACK "\x1b[30;1m"
#define PLX_ANSI_FOREGROUND_BRIGHT_RED "\x1b[31;1m"
#define PLX_ANSI_FOREGROUND_BRIGHT_GREEN "\x1b[32;1m"
#define PLX_ANSI_FOREGROUND_BRIGHT_YELLOW "\x1b[33;1m"
#define PLX_ANSI_FOREGROUND_BRIGHT_BLUE "\x1b[34;1m"
#define PLX_ANSI_FOREGROUND_BRIGHT_MAGENTA "\x1b[35;1m"
#define PLX_ANSI_FOREGROUND_BRIGHT_CYAN "\x1b[36;1m"
#define PLX_ANSI_FOREGROUND_BRIGHT_WHITE "\x1b[37;1m"

// Background
#define PLX_ANSI_BACKGROUND_BLACK "\x1b[40m"
#define PLX_ANSI_BACKGROUND_RED "\x1b[41m"
#define PLX_ANSI_BACKGROUND_GREEN "\x1b[42m"
#define PLX_ANSI_BACKGROUND_YELLOW "\x1b[43m"
#define PLX_ANSI_BACKGROUND_BLUE "\x1b[44m"
#define PLX_ANSI_BACKGROUND_MAGENTA "\x1b[45m"
#define PLX_ANSI_BACKGROUND_CYAN "\x1b[46m"
#define PLX_ANSI_BACKGROUND_WHITE "\x1b[47m"
#define PLX_ANSI_BACKGROUND_DEFAULT "\x1b[49m"
#define PLX_ANSI_BACKGROUND_BRIGHT_BLACK "\x1b[40;1m"
#define PLX_ANSI_BACKGROUND_BRIGHT_RED "\x1b[41;1m"
#define PLX_ANSI_BACKGROUND_BRIGHT_GREEN "\x1b[42;1m"
#define PLX_ANSI_BACKGROUND_BRIGHT_YELLOW "\x1b[43;1m"
#define PLX_ANSI_BACKGROUND_BRIGHT_BLUE "\x1b[44;1m"
#define PLX_ANSI_BACKGROUND_BRIGHT_MAGENTA "\x1b[45;1m"
#define PLX_ANSI_BACKGROUND_BRIGHT_CYAN "\x1b[46;1m"
#define PLX_ANSI_BACKGROUND_BRIGHT_WHITE "\x1b[47;1m"

// Enables ANSI escape codes on the standard output stream.
// Returns true on success and false on failure.
bool plx_enable_ansi_escape_codes_stdout(void);

// Enables ANSI escape codes on the standard error stream.
// Returns true on success and false on failure.
bool plx_enable_ansi_escape_codes_stderr(void);

#endif  // PLX_ANSI_ESCAPE_CODES_H
