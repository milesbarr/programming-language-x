# Programming Language X

Programming Language X is a compiler written in C.

## Current State

The compiler implements parsing, name resolution, type checking, and code generation for a limited number of programming language features and includes two backends: [LLVM IR](https://llvm.org/docs/LangRef.html) and [WebAssembly](https://webassembly.org/).

## Building

In order to build the compiler from source, [CMake](https://cmake.org/) and a C compiler such as [Clang](https://clang.llvm.org/) or [GCC](https://gcc.gnu.org/) is required. First, ensure these dependencies are installed on your system. Next, clone this repository and open a shell in the directory. Lastly, build the compiler by running the following commands:

```bash
mkdir build
cd build
cmake ..
cmake --build .
```

## Syntax

### Definitions

Define a constant.

```go
const foo = 1;
```

Define a variable.

```go
var foo = 1;
```

Declare a variable.

```go
var foo: s32;
```

Define a function.

```go
func foo(bar: s32) -> s32 {
  return bar;
}
```

### Types

Integer types:

```go
s8
s16
s32
s64
u8
u16
u32
u64
```

Floating point types:

```go
f32
f64
```

Boolean type:

```go
bool
```

Function types:

```go
func (s32) -> s32
```

Array types:

```go
[8]s32
```

Slice types:

```go
[]s32
```

## Examples

Add two integers.

```go
func add(a: s32, b: s32) -> s32 {
  return a + b;
}
```

Fibonacci sequence.

```go
func fib(n: s32) -> s32 {
  if n == 0 {
    return 0;
  } else if n == 1 or n == 2 {
    return 1;
  } else {
    return fib(n - 1) + fib(n - 2);
  }
}
```
