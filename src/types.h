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

#ifndef PLX_TYPES_H
#define PLX_TYPES_H

#include <stdbool.h>

#include "ast.h"

// Returns whether a type is a signed integer type.
bool plx_is_sint_type(const struct plx_node* type);

// Returns whether a type is an unsigned integer type.
bool plx_is_uint_type(const struct plx_node* type);

// Returns whether a type is an integer type.
bool plx_is_int_type(const struct plx_node* type);

// Returns whether a type is a floating point type.
bool plx_is_float_type(const struct plx_node* type);

// Returns whether a type is numeric.
bool plx_is_numeric_type(const struct plx_node* type);

// Returns whether a type can be used in logical operations.
bool plx_is_logical_type(const struct plx_node* type);

// Returns whether a type can be used in equality operations.
bool plx_is_equality_type(const struct plx_node* type);

// Returns whether two types are equal.
bool plx_type_eq(const struct plx_node* type_a, const struct plx_node* type_b);

#endif  // PLX_TYPES_H
