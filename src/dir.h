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

#ifndef PLX_DIR_H
#define PLX_DIR_H

#include <stdbool.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

struct plx_dir {
  HANDLE handle;
  WIN32_FIND_DATAA data;
  bool first;
};
#else
#include <dirent.h>

struct plx_dir {
  DIR* dir;
};
#endif  // _WIN32

bool plx_dir_open(struct plx_dir* dir, const char* path);
const char* plx_dir_read(struct plx_dir* dir, bool* is_dir);
void plx_dir_close(struct plx_dir* dir);

#endif  // PLX_DIR_H
