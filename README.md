# bbc

Bad B Compiler is a work-in-progress compiler for B programming language and my attempt to understand the concepts of working on a programming language.

For now, this compiler has can only:
- generate tokens for whole input file (without support for special assignments - e.g. =+),
- generate AST:
    - numbers
    - variables
    - arithmetic operators
    - assignments
    - if statements
    - while loops
- compile parsed code into x86_64 code using fasm.

Example of currently working b code is available in [this file](examples/compilable.b).

## Quick Start
```bash
make
./bbc examples/compilable.b
fasm test.asm
gcc -no-pie test.o -o test
```
