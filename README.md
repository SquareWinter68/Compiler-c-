# し++ compiler
This is a work in progress, the thought was to make a compiler which compiles to assembly, and to familiarize myself with the c++ programming language.

## Structure
The project has two forms of intermediate representation, both an AST, and TAC (three adress code), it uses some modern c++ 20 features such as concepts.

The code concerned with intermediate representation can be found in include/intermediate.hpp, and some type information is stored in include/utils.hpp

It ships with an example factorial function.
