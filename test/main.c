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

#include <stdlib.h>

void plx_test_leb128(void);
void plx_test_symbol_table(void);
void plx_test_tokenizer(void);

int main() {
  plx_test_leb128();
  plx_test_symbol_table();
  plx_test_tokenizer();
  return EXIT_SUCCESS;
}
