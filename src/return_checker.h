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

#ifndef PLX_RETURN_CHECKER_H
#define PLX_RETURN_CHECKER_H

#include <stdbool.h>

#include "ast.h"

// Checks that functions in the abstract syntax tree are not missing any return
// statements.
bool plx_check_returns(const struct plx_node* node);

#endif  // PLX_RETURN_CHECKER_H
