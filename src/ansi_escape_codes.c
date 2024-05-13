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

#include "ansi_escape_codes.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#else
#include <stdio.h>
#include <unistd.h>
#endif  // _WIN32

bool plx_enable_ansi_escape_codes_stdout(void) {
#ifdef _WIN32
  const HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
  if (handle == INVALID_HANDLE_VALUE) return false;

  DWORD mode = 0;
  if (!GetConsoleMode(handle, &mode)) return false;
  mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
  return SetConsoleMode(handle, mode);
#else
  return isatty(STDOUT_FILENO);
#endif  // _WIN32
}

bool plx_enable_ansi_escape_codes_stderr(void) {
#ifdef _WIN32
  const HANDLE handle = GetStdHandle(STD_ERROR_HANDLE);
  if (handle == INVALID_HANDLE_VALUE) return false;

  DWORD mode = 0;
  if (!GetConsoleMode(handle, &mode)) return false;
  mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
  return SetConsoleMode(handle, mode);
#else
  return isatty(STDERR_FILENO);
#endif  // _WIN32
}
