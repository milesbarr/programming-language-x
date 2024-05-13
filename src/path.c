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

#include "path.h"

const char* plx_path_base(const char* const path) {
  const char* base = path;
  const char* s = base;
  while (*s != '\0') {
    if (*s++ == '/') base = s;
  }
#ifdef _WIN32
  s = base;
  while (*s != '\0') {
    if (*s++ == '\\') base = s;
  }
#endif  // _WIN32
  return base;
}

const char* plx_path_ext(const char* const path) {
  const char* const base = plx_path_base(path);
  const char* s = base + (base[0] == '.');
  const char* ext = "";
  while (*s != '\0') {
    if (*s++ == '.') ext = s - 1;
  }
  return ext;
}

bool plx_path_full(const char* const rel, char* const abs) {
#ifdef _WIN32
  return _fullpath(abs, rel, PLX_PATH_MAX) != NULL;
#else
  return realpath(rel, abs) != NULL;
#endif  // _WIN32
}
