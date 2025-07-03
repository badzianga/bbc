# bbc

Bad B Compiler is a work-in-progress compiler for B programming language and my attempt to understand the concepts of working on a programming language.

For now, this compiler has can only:
- generate tokens for whole input file (without support for assignments - e.g. =+),
- generate AST for single-line expression (numbers and operators only),
- interpret generated AST using a simple tree-walk interpreter.

The built-in interpreter will be removed (or moved to another repo) in the future as the project becomes more complex.

## Quick Start
```bash
make
./bbc examples/expression.b
```
