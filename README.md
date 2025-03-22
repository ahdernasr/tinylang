# TinyLang

A minimal, bytecode-compiled programming language implementation written in C++17.

## Overview

TinyLang is a complete programming language implementation featuring a lexer, parser, compiler, virtual machine, and garbage collector. It demonstrates the core concepts behind interpreted languages like Lua, Python, and JavaScript.

## Features

- **Full Language Implementation**: Lexer, parser, AST, compiler, and VM
- **Language Features**:
  - Variables and functions
  - Closures and higher-order functions
  - Control flow: `if/else`, `while`, `for`, `break`, `continue`
  - Recursive functions
  - Data types: numbers, strings, booleans, nil
- **Memory Management**: Automatic garbage collection
- **Interactive REPL**: Run code interactively with helpful commands
- **Tools**: Compiler (`tlc`) and disassembler (`tldis`)
- **Optimization**: Bytecode optimization pass
- **Error Reporting**: Clear compile-time and runtime error messages

## Building

### Requirements

- CMake 3.16 or higher
- C++17 compatible compiler (GCC, Clang, or MSVC)

### Build Instructions

```bash
mkdir build
cd build
cmake ..
make
```

Or use the provided build script:

```bash
./scripts/build.sh
```

### Build Options

- `TL_ENABLE_ASAN`: Enable AddressSanitizer (OFF by default)
- `TL_ENABLE_GC_STRESS`: Enable GC stress testing (OFF by default)
- `TL_ENABLE_CLOSURES`: Enable closures support (ON by default)

## Usage

### Interactive REPL

Run the REPL:

```bash
./build/tl
```

Example session:

```
tl> let x = 42;
✓ Executed in 0.5ms

tl> print(x);
42
✓ Executed in 0.5ms

tl> fn fib(n) { \
  >   if (n <= 1) return n; \
  >   return fib(n-1) + fib(n-2); \
  > }
✓ Executed in 0.3ms

tl> print(fib(10));
55
✓ Executed in 2.1ms
```

REPL Commands:
- `:help` - Show help message
- `:quit` - Exit REPL
- `:stack` - Show current stack contents
- `:globals` - Show global variables
- `:gc` - Force garbage collection
- `:stats` - Show execution statistics

### Running Scripts

Compile and run a `.tl` file:

```bash
./build/tlc examples/fib.tl
```

Or use the run script:

```bash
./scripts/run.sh examples/fib.tl
```

### Disassembler

View bytecode disassembly:

```bash
./build/tldis examples/fib.tl
```

## Language Syntax

### Variables

```tinylang
let x = 42;
var y = "hello";
```

### Functions

```tinylang
fn add(a, b) {
    return a + b;
}

print(add(5, 3)); // 8
```

### Control Flow

```tinylang
// If-else
if (x > 10) {
    print("large");
} else {
    print("small");
}

// While loop
let i = 0;
while (i < 10) {
    print(i);
    i = i + 1;
}

// For loop
for (let j = 0; j < 5; j = j + 1) {
    print(j);
}
```

### Closures

```tinylang
fn make_counter() {
    let count = 0;
    return fn() {
        count = count + 1;
        return count;
    };
}

let counter = make_counter();
print(counter()); // 1
print(counter()); // 2
```

### Built-in Functions

- `print(value)` - Print a value
- `clock()` - Get current time in seconds
- `len(str)` - Get string length
- `assert(condition)` - Assert a condition
- `toString(value)` - Convert value to string
- `toNumber(value)` - Convert value to number
- `range(start, end)` - Generate range of numbers

## Example Programs

See the `examples/` directory for sample programs:

- `fib.tl` - Fibonacci sequence (recursive and iterative)
- `fizzbuzz.tl` - FizzBuzz implementation
- `control_flow.tl` - Control flow examples
- `closures.tl` - Closures and higher-order functions

## Project Structure

```
tinylang/
├── include/tl/          # Header files
│   ├── lexer.hpp        # Tokenizer
│   ├── parser.hpp       # Parser
│   ├── ast.hpp          # Abstract syntax tree
│   ├── compiler.hpp     # AST to bytecode compiler
│   ├── vm.hpp           # Virtual machine
│   ├── gc.hpp           # Garbage collector
│   └── ...
├── src/                 # Implementation files
├── tools/               # Command-line tools
│   ├── tlc.cpp         # Compiler tool
│   └── tldis.cpp       # Disassembler
├── examples/            # Example programs
├── tests/               # Test suite
└── benchmarks/          # Benchmark programs
```

## Architecture

TinyLang follows a traditional interpreter pipeline:

1. **Lexer** (`lexer.cpp`) - Tokenizes source code
2. **Parser** (`parser.cpp`) - Builds abstract syntax tree
3. **Compiler** (`compiler.cpp`) - Compiles AST to bytecode
4. **Optimizer** (`optimizer.cpp`) - Optimizes bytecode
5. **VM** (`vm.cpp`) - Executes bytecode on stack-based VM

## Testing

Run the test suite:

```bash
cd build
ctest
```

## License

This is an educational project demonstrating language implementation concepts.

## Contributing

This is a personal learning project. Feel free to explore and learn from the code!
