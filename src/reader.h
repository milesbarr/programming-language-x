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

#ifndef PLX_READER_H
#define PLX_READER_H

#include <stdbool.h>
#include <stdio.h>

#include "source_code_location.h"

struct plx_reader {
  FILE* stream;
  struct plx_source_code_location loc;
  int c;
};

void plx_reader_init(struct plx_reader* reader, const char* filename,
                     FILE* stream);
void plx_next_char(struct plx_reader* reader);
int plx_peek_char(const struct plx_reader* reader);
int plx_read_char(struct plx_reader* reader);
bool plx_accept_char(struct plx_reader* reader, const char c);
void plx_unexpected_character(const struct plx_reader* reader);

#endif  // PLX_READER_H
