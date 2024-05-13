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

#ifndef PLX_CONSTANT_FOLDER_H
#define PLX_CONSTANT_FOLDER_H

#include <stdbool.h>

#include "ast.h"

// Evaluates constant expressions in the abstract syntax tree and returns
// whether any changes were made.
// https://en.wikipedia.org/wiki/Constant_folding
bool plx_fold_constants(struct plx_node* node);

#endif  // PLX_CONSTANT_FOLDER_H
