# bbc

Bad B Compiler is a work-in-progress compiler for B programming language and my attempt to understand the concepts of working on a programming language.

For now, this compiler has can only:
- generate tokens for whole input file (without support for assignments - e.g. =+),
- generate AST (numbers, variables, operators, assignments only),
- interpret generated AST using a simple tree-walk interpreter (NOT WORKING),
- compile parsed code into x86_64 code using fasm.

The built-in interpreter will be removed (or moved to another repo) in the future as the project becomes more complex.

Example of currently working b code is available in [this file](examples/compilable.b).

## Quick Start
```bash
make
./bbc examples/compilable.b
fasm test.asm
gcc -no-pie test.o -o test
```
