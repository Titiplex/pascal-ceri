---
title: "CERI Compiler"
subtitle: "A simple Pascal compiler"
author: "Johanny Titouan"
institution: "Avignon Université"
date: "05/2025"
description: "A compiler from Pascal-like imperative LL(k) language to 64-bit x86 assembly language (AT&T syntax)"
keywords:

- compiler
- pascal
- assembly
- x86
  tags:
- academic
- programming-language

---

# CERI Compiler

## Overview
<p>
A simple Pascal compiler developed at the University of Avignon during the an assembly class given by Mr. Pierre Jourlin. You can see his reference git repository [here](https://framagit.org/jourlin/cericompiler).

- **Author:** Johanny Titouan
- **Class:** Assembly and Compiler
  - **Teacher:** Pierre Jourlin
  - **University:** Avignon (France)
- **Purpose:** Translates a Pascal-like imperative LL(k) language into 64-bit x86 assembly language (AT&T syntax).

For this assignment, I developed the compiler myself from the start (I didn't use my teacher's framagit). For the last part of the assignment "free coding", I implemented the following features :
- Case Of (Switch)
- Arrays
- Functions and Procedures
- Strings

I tried to follow as much as possible the standard [Pascal Grammar](https://condor.depaul.edu/ichu/csc447/notes/wk2/pascal.html) even though I had to adjust a bit considering time and level constraints.

The display statement was made thanks to the internet, considering that my teacher's reference git repository only considered Ubuntu.
</p>

## Language Grammar

The grammar is defined as follows:

$$
\begin{aligned}
\langle\text{Program}\rangle\; &::=\;  \textbf{[}\; \langle\text{VarDeclarationPart}\rangle\; \textbf{]}\; \textbf{[}\; \langle\text{FuncProcDeclarationPart}\rangle\; \textbf{]}\; \langle\text{StatementPart}\rangle\; \textbf{.} \\[16pt]
\langle\text{VarDeclarationPart}\rangle\; &::=\;  \textbf{VAR}\; \langle\text{VarDeclaration}\rangle\; \text{\{}\; \textbf{;}\; \langle\text{VarDeclaration}\rangle\; \text{\}}\; \textbf{.} \\[4pt]
\langle\text{VarDeclaration}\rangle\; &::=\;  \langle\text{Ident}\rangle\; \text{\{}\;\textbf{,}\; \langle\text{Ident}\rangle\; \text{\}}\; \textbf{:}\; \langle\text{Type}\rangle\; \\[4pt]
\langle\text{Type}\rangle\; &::=\;  \textbf{INTEGER}\;
\text{|}\; \textbf{BOOLEAN}\;
\text{|}\; \textbf{DOUBLE}\;
\text{|}\; \textbf{CHAR}\;
\text{|}\; \textbf{STRING}\;
\text{|}\; \textbf{ARRAY}\; \\[16pt]
\langle\text{FuncProcDeclarationPart}\rangle\; &::=\;  \text{\{}\;\langle\text{FuncProcDeclaration}\rangle\text{\}}\; \textbf{;} \\[4pt]
\langle\text{FuncProcDeclaration}\rangle\; &::=\;  \langle\text{FunctionDeclaration}\rangle\; \text{|}\; \langle\text{ProcedureDeclaration}\rangle\; \\[4pt]
\langle\text{ProcedureDeclaration}\rangle\; &::=\;  \langle\text{ProcedureHeading}\rangle\; \langle\text{BlockStmt}\rangle\; \\[4pt]
\langle\text{ProcedureHeading}\rangle\; &::=\;  \textbf{PROCEDURE}\; \langle\text{Ident}\rangle\; \text{[}\; \textbf{(}\; \langle\text{ParameterGroup}\rangle\; \text{\{}\; \textbf{;}\; \langle\text{ParameterGroup}\rangle\; \text{\}}\; \textbf{)}\; \text{]}\; \textbf{;} \\[4pt]
\langle\text{ParameterGroup}\rangle\; &::=\;  \langle\text{Ident}\rangle\; \text{\{}\; \textbf{,}\; \langle\text{Ident}\rangle\; \text{\}}\; \textbf{:}\; \langle\text{Type}\rangle\; \\[4pt]
\langle\text{FunctionDeclaration}\rangle\; &::=\;  \langle\text{FunctionHeading}\rangle\; \langle\text{BlockStmt}\rangle\; \\[4pt]
\langle\text{FunctionHeading}\rangle\; &::=\;  \textbf{FUNCTION}\; \langle\text{Ident}\rangle\; \text{[}\; \textbf{(}\; \langle\text{ParameterGroup}\rangle\; \text{\{}\; \textbf{;}\; \langle\text{ParameterGroup}\rangle\; \text{\}}\; \textbf{)}\; \text{]}\; \textbf{:}\; \langle\text{Type}\rangle\; \textbf{;} \\[16pt]
\langle\text{StatementPart}\rangle\; &::=\;  \langle\text{Statement}\rangle\; \text{\{}\; \textbf{;}\; \langle\text{Statement}\rangle\; \text{\}}\; \textbf{.} \\[4pt]
\langle\text{Statement}\rangle\; &::=\;  \langle\text{AssignmentStmt}\rangle\
\text{|}\; \langle\text{IfStatement}\rangle\
\text{|}\; \langle\text{WhileStmt}\rangle\
\text{|}\; \langle\text{ForStmt}\rangle\
\text{|}\; \langle\text{CaseStmt}\rangle\
\text{|}\; \langle\text{DisplayStmt}\rangle\
\text{|}\; \langle\text{ReturnStmt}\rangle\
\text{|}\; \langle\text{BlockStmt}\rangle\; \\[4pt]
\langle\text{AssignmentStmt}\rangle\; &::=\;  \langle\text{Ident}\rangle\; \text{[}\; \langle\text{ArrayAccess}\langle\; \text{]}\; \textbf{:=}\; \langle\text{Expression}\rangle\; \\[4pt]
\langle\text{IfStmt}\rangle\; &::=\;  \textbf{IF}\; \langle\text{Expression}\rangle\; \textbf{THEN}\; \langle\text{Statement}\rangle\; \text{[}\; \textbf{ELSE}\; \langle\text{Statement}\rangle\; \text{]}\;\\[4pt]
\langle\text{WhileStmt}\rangle\; &::=\;  \textbf{WHILE}\; \langle\text{Expression}\rangle\; \textbf{DO}\; \langle\text{Statement}\rangle\; \\[4pt]
\langle\text{ForStmt}\rangle\; &::=\;  \textbf{FOR}\; \langle\text{AssignmentStmt}\rangle\; \langle\text{ForEnd}\rangle\; \text{[}\; \textbf{STEP}\; \langle\text{Number}\rangle\; \text{]}\; \textbf{DO}\; \langle\text{Expression}\rangle\; \\[4pt]
\langle\text{ForEnd}\rangle\; &::=\;  \textbf{TO}\; \langle\text{Expression}\rangle\; \text{|}\; \textbf{DOWNTO}\; \langle\text{Expression}\rangle\; \\[4pt]
\langle\text{CaseStmt}\rangle\; &::=\;  \textbf{CASE}\; \langle\text{Expression}\rangle\; \textbf{OF}\; \langle\text{CaseListElement}\rangle\; \text{\{}\; \textbf{;}\; \langle\text{CaseListElement}\rangle\; \text{\}}\; \textbf{END} \\[4pt]
\langle\text{CaseListElement}\rangle\; &::=\;  \langle\text{CaseLabelList}\rangle\; \textbf{:}\; \text{[}\; Statement\; \text{]}\;\\[4pt]
\langle\text{CaseLabelList}\rangle\; &::=\;  \langle\text{Number}\rangle\; \text{\{}\; \textbf{,}\; \langle\text{Number}\rangle\; \text{\}}\; \\[4pt]
\langle\text{BlockStmt}\rangle\; &::=\;  \textbf{BEGIN}\; \langle\text{Statement}\rangle\; \text{\{}\; \textbf{;}\; \langle\text{Statement}\rangle\; \text{\}}\; \textbf{END} \\[4pt]
\langle\text{ReturnStmt}\rangle\; &::=\;  \textbf{RETURN}\; \text{[}\; \langle\text{Expression}\rangle\; \text{]}\;\\[4pt]
\langle\text{DisplayStmt}\rangle\; &::=\;  \textbf{DISPLAY}\; \langle\text{Expression}\rangle\; \\[16pt]
\langle\text{Expression}\rangle\; &::=\;  \langle\text{SimpleExpression}\rangle\; \text{[}\; \langle\text{RelationalOperator}\rangle\; \langle\text{SimpleExpression}\rangle\; \text{]}\;\\[4pt]
\langle\text{SimpleExpression}\rangle\; &::=\;  \langle\text{Term}\rangle\; \text{\{}\; \langle\text{AdditiveOperator}\rangle\; \langle\text{Term}\rangle\; \text{\}}\; \\[4pt]
\langle\text{Term}\rangle\; &::=\;  \langle\text{Factor}\rangle\; \text{\{}\; \langle\text{MultiplicativeOperator}\rangle\; \langle\text{Factor}\rangle\; \text{\}}\; \\[4pt]
\langle\text{Factor}\rangle\; &::=\;  \langle\text{Identifier}\rangle\; \text{|}\; \langle\text{Number}\rangle\; \text{|}\; \langle\text{Double}\rangle\; \text{|}\; \langle\text{Character}\rangle\; \text{|}\; \langle\text{String}\rangle\; \text{|}\; \textbf{(}\; \langle\text{Expression}\rangle\; \textbf{)}\; \text{|}\; \textbf{!} \langle\text{Factor}\rangle\; \\[16pt]
\langle\text{RelationalOperator}\rangle\; &::=\;  \textbf{==}\; \text{|}\; \textbf{!=}\; \text{|}\; \textbf{>}\; \text{|}\; \textbf{<}\; \text{|}\; \textbf{>=}\; \text{|}\; \textbf{<=} \\[4pt]
\langle\text{AdditiveOperator}\rangle\; &::=\;  \textbf{+}\; \text{|}\; \textbf{-}\; \text{|}\; \textbf{||} \\[4pt]
\langle\text{MultiplicativeOperator}\rangle\; &::=\;  \textbf{*}\; \text{|}\; \textbf{/}\; \text{|}\; \textbf{%}\; \text{|}\; \textbf{&&} \\[16pt]
\langle\text{Number}\rangle\; &::=\;  \langle\text{Digit}\rangle\; \text{\{}\; \langle\text{Digit}\rangle\; \text{\}}\; \\[4pt]
\langle\text{Digit}\rangle\; &::=\;  \textbf{0}\; \text{|}\; \textbf{1}\; \text{|}\; \textbf{...}\; \text{|}\; \textbf{9} \\[4pt]
\langle\text{Double}\rangle\; &::=\;  \langle\text{Number}\rangle \textbf{.} \langle\text{Number}\rangle\; \\[4pt]
\langle\text{Character}\rangle\; &::=\;  \textbf{'}\; \text{[}\; \textit{ASCII_Value}\; \text{]}\;\textbf{'} \\[4pt]
\langle\text{String}\rangle\; &::=\;  \textbf{"}\; \text{\{}\; \textit{ASCII_Value}\; \text{\}}\; \textbf{"} \\[16pt]
\langle\text{Identifier}\rangle\; &::=\;  \langle\text{Letter}\rangle\; \text{\{}\; \langle\text{LetterOrDigit}\rangle\; \text{\}}\; \text{[}\; \langle\text{Accessors}\rangle\; \text{]} \\[4pt]
\langle\text{Accessors}\rangle\; &::=\; \langle\text{ArrayAccess}\rangle\; \text{|}\; \langle\text{FunctionAccess}\rangle\; \\[4pt]
\langle\text{ArrayAccess}\rangle\; &::=\; \pmb{[}\; \langle\text{Expression}\rangle\; \pmb{]}\; \\[4pt]
\langle\text{FunctionAccess}\rangle\; &::=\; \textbf{(}\; \langle\text{Expression}\rangle\; \text{\{}\; \textbf{,}\;  \langle\text{Expression}\rangle\; \text{\}}\; \textbf{)}\; \\[4pt]
\langle\text{Letter}\rangle\; &::=\;  \textbf{a}\; \text{|}\; \textbf{...}\; \text{|}\; \textbf{z}\; \text{|}\; \textbf{A}\; \text{|}\; \textbf{...}\; \text{|}\; \textbf{Z} \\[4pt]
\langle\text{LetterOrDigit}\rangle\; &::=\;  \langle\text{Letter}\rangle\; \text{|}\; \langle\text{Digit}\rangle\; \\[4pt]
\end{aligned}
$$

## Requirements

<p>
Please make sure you have the flex/flex++ package installed as it is essential for the compiler to parse lexemes. Flex might not be installed by default on your system.

For windows users, you can download the package from [here](https://github.com/westes/flex/releases), or via msys2 or vcpkg. Then please make sure the flex exe is in your PATH and/or FlexLexer.h is in your include path.
</p>
<p>
Required minimum versions :

- c++ : 23
  - compiler : GCC 13+, Clang 17+, MSVC 19.38+
- cmake : 3.20
- make : 4.4.1
- flex/flex++ : 2.6

Please note that the compiler have been made for windows, even though I tried to develop it with linux, I do not guarantee its functionality on linux, especially with the displays.
</p>

## Project folders

<details>
<summary>Project tree</summary>

```markdown
. # project root
├── bin/          # final compiler executable
├── lib/          # static libs produced by CMake/Make
├── build/        # out-of-source CMake builds
├── compiler/     # CLI front‑end (main.cpp)
├── parser/       # recursive‑descent parser
├── tokeniser/    # Flex lexer (.l -> tokeniser.cpp)
├── utils/        # small helper library
├── examples/     # sample Pascal programs
├── include/      # public headers (generated/copied)
└── CMakeLists.txt + per‑dir CMakeLists.txt
```
</details>

## Installation and Build

<p>
To compile the Pascal compiler with cmake ( while in project root folder) :
</p>

````shell
# 1. Configure an out‑of‑tree build
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release # Or Debug

# 2. Build everything with the default generator (Make)
cmake --build build -j$(nproc)

# 3. (Optional) Install to ./bin & ./lib inside the source tree
cmake --install build
````
<p>
You can also use all.run.xml in the .run folder to compile the project with cmake more easily.

Windows MSVC version if needed :
</p>

````shell
cmake -B build -S . -G "Ninja Multi-Config"
cmake --build build --config Release # Or Debug
````

<p>
With GNU make :
</p>

````shell
make
make clean
````
(Switching between the Makefile and CMake builds is safe—they share the same output directories.)

If the Parser Header file doesn't generate with cmake, please clean your build and bin, then try again. If the problem persists, try generating the Parser Header file manually or with make.
<p>
To compile with the pascal compiler, which only gives an assembly at&t file :
</p>

``./bin/pascal <input.p> [-o output.s]``

#### Example

````shell
cd examples
../bin/pascal factorial.p -o factorial.s
gcc -no-pie -o factorial factorial.s
./factorial 5   # => 120
````

You can also use ``make`` to compile all examples while in the examples directory, associated .s files will be in examples/build and exes in examples/bin.
