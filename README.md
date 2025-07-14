# 9cc - A Handmade C Compiler

> **📖 Reference Book**: This compiler is implemented based on https://www.sigbus.info/compilerbook.

9cc is a small C compiler created for educational purposes. It compiles a subset of the C language and outputs Linux x86-64 assembly.

## Features

### Supported Language Features

- **Data Types**: `int`, `char`, pointers, arrays
- **Operators**:
  - Arithmetic operators: `+`, `-`, `*`, `/`
  - Comparison operators: `==`, `!=`, `<`, `<=`, `>`, `>=`
  - Assignment operator: `=`
  - Address operators: `&`, `*`
  - `sizeof` operator
- **Control Structures**:
  - `if` statements (with `else`)
  - `while` loops
  - `for` loops
- **Functions**: Function definitions, function calls (up to 6 arguments)
- **Variables**: Local variables, global variables
- **Arrays**: 1-dimensional and multi-dimensional arrays
- **String Literals**: `"hello"`
- **Blocks**: Scope with `{}`
- **Comments**: `//` and `/* */`
- **Statement expression**: `({ ... })`

## Build

```bash
make
```

## Usage

```bash
./9cc <input.c> > output.s
gcc -static -o program output.s
./program
```

### Example

```c
// hello.c
int main() {
    int x = 42;
    return x;
}
```

```bash
./9cc hello.c > hello.s
gcc -static -o hello hello.s
./hello
echo $?  # Outputs 42
```

## Testing

```bash
make test
```

Tests are written in `test/test.c` and test various C language features.

## Architecture

The compiler operates in the following stages:

1. **Lexical Analysis** (`tokenize.c`): Split source code into tokens
2. **Parsing** (`parse.c`): Build Abstract Syntax Tree (AST) from tokens
3. **Type Checking** (`type.c`): Add type information to AST
4. **Code Generation** (`codegen.c`): Generate x86-64 assembly from AST

## File Structure

```
src/
├── 9cc.h      # Header file (structs, function declarations)
├── main.c     # Main entry point
├── tokenize.c # Lexical analysis
├── parse.c    # Parsing
├── type.c     # Type checking
└── codegen.c  # Code generation

test/
├── test.sh    # Test script
└── test.c     # Test file
```

## Limitations

- Only a subset of standard library functions supported
- No preprocessor support
- No struct/union support
- No float/double type support
- Some complex pointer arithmetic limitations

## References
- https://www.sigbus.info/compilerbook - A compiler creation tutorial by Rui Ueyama
