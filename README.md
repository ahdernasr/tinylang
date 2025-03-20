# TinyLang

A minimal interpreted language implemented in C++17. TinyLang focuses on the essentials: lexing, parsing, and evaluating a small expression language with variables, basic control flow, and print statements.

## What is included

- Tokenizer and recursive-descent parser
- Expression evaluation with numbers, strings, booleans, and `nil`
- Statements: `let` declarations, assignments, blocks, `if` / `else`, `while`, and expression statements
- Built-in `print` statement
- Simple REPL (`tl`) for interactive exploration

## What was removed

To keep the codebase approachable, the following advanced features from the original project were removed:

- Bytecode compiler, virtual machine opcodes, and optimizer passes
- Closures, upvalues, and function objects
- Garbage collector and string interning
- Disassembler, compiler CLI utilities, and benchmark suite
- Extra REPL commands and shell scripts

## Building

```bash
mkdir build
cd build
cmake ..
cmake --build .
```

## Running the REPL

```bash
./build/tl
```

Example session:

```
tl> let name = "TinyLang";
tl> print name;
TinyLang
tl> let x = 3;
tl> print x * 4 + 2;
14
```

The REPL accepts multiple statements separated by semicolons. Use `:quit` (or `:exit`) to leave the session.

## Running a file

```bash
./build/tl < examples/quickstart.tl
```

## Language overview

### Values

- Numbers (floating point)
- Strings (`"hello"`)
- Booleans (`true`, `false`)
- `nil`

### Expressions

- Arithmetic: `+`, `-`, `*`, `/`
- Comparison: `<`, `<=`, `>`, `>=`
- Equality: `==`, `!=`
- Logical: `and`, `or`, `!`
- Grouping: `( expression )`

### Statements

- Variable declaration: `let answer = 42;`
- Assignment: `answer = answer + 1;`
- Print: `print answer;`
- Blocks: `{ ... }`
- If/Else: `if (condition) { ... } else { ... }`
- While loop: `while (condition) { ... }`

## Example

See `examples/quickstart.tl` for a complete sample program demonstrating the current feature set.

## License

This repository is intended for educational purposes and experimentation with language implementation concepts.
