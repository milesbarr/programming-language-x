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

#ifndef PLX_PATH_H
#define PLX_PATH_H

#include <stdbool.h>

#ifdef _WIN32
#include <stdlib.h>

#define PLX_PATH_MAX _MAX_PATH
#else
#include <limits.h>

#define PLX_PATH_MAX PATH_MAX
#endif  // _WIN32

// Returns the base of a path.
const char* plx_path_base(const char* path);

// Returns the file extension of a path.
const char* plx_path_ext(const char* path);

// Resolves a relative path to a canonicalized absolute path.
bool plx_path_full(const char* const rel, char* const abs);

#endif  // PLX_PATH_H
