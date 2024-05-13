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

#include "dir.h"

#ifdef _WIN32

#include <stdio.h>

bool plx_dir_open(struct plx_dir* dir, const char* path) {
  char filename[MAX_PATH];
  if (snprintf(filename, sizeof(filename), "%s\\*", path) < 0) return false;
  dir->handle = FindFirstFileA(filename, &dir->data);
  dir->first = true;
  return dir->handle != INVALID_HANDLE_VALUE;
}

const char* plx_dir_read(struct plx_dir* const dir, bool* const is_dir) {
  if (!dir->first && !FindNextFileA(dir->handle, &dir->data)) return NULL;
  dir->first = false;
  *is_dir = dir->data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;
  return dir->data.cFileName;
}

void plx_dir_close(struct plx_dir* const dir) { FindClose(dir->handle); }

#else

bool plx_dir_open(struct plx_dir* const dir, const char* const path) {
  dir->dir = opendir(path);
  return dir->dir != NULL;
}

const char* plx_dir_read(struct plx_dir* const dir, bool* const is_dir) {
  const struct dirent* const entry = readdir(dir->dir);
  *is_dir = S_ISDIR(entry->d_type & DT_DIR);
  return entry->d_name;
}

void plx_dir_close(struct plx_dir* const dir) { closedir(dir->dir); }

#endif  // _WIN32
