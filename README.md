# expr-eval

Simple expression evaluator that can output 3-address assembly code.

## Building expr-eval

### Prerequisites

- C compiler (GCC/Clang)
- GNU Bison
- flex

### Building

Build binaries

```
# This will generate scanner, parser, and compiler binaries.
# Binaries are stored in "build" folder.
$ make
```

Clean project

```
$ make clean
```

## Run

### Usage

```
USAGE: ./build/program [-o output] file
```

### Example

```
$ ./build/compiler -o out.asm input.txt
```
