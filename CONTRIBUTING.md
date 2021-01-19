# Contributing to Chaos Language

## Code of Conduct

This project and everyone participating in it is governed by the [Code of Conduct](CODE_OF_CONDUCT.md).
By participating, you are expected to uphold this code. Please report unacceptable behavior to [info@chaos-lang.org](mailto:info@chaos-lang.org).

## Compiling Chaos Interpreter on Development Mode

```
make clean
make dev
sudo make install
tests/interpreter.sh
```

placing this command into a shell script like `dev.sh` and executing that, is the suitable way for the developers.
`*dev.*` wildcard ignored in `.gitignore` So name your program files like `loops-dev.kaos` for testing while developing.

### Flex & bison

 - https://github.com/westes/flex
 - https://github.com/akimd/bison

### Useful Resources

 - **You must read:**
    - https://github.com/angrave/SystemProgramming/wiki#1-learning-c
    - https://www.geeksforgeeks.org/flex-fast-lexical-analyzer-generator/
    - https://www.geeksforgeeks.org/introduction-to-yacc/
    - http://dinosaur.compilertools.net/
    - https://berthub.eu/lex-yacc/cvs/lex-yacc-howto.html
    - https://github.com/konieshadow/lex-yacc-examples
    - https://prettydiff.com/2/guide/unrelated_parser.xhtml
    - https://www.geeksforgeeks.org/linked-list-set-1-introduction/
    - https://www.geeksforgeeks.org/symbol-table-compiler/

 - **Good to read:**
   - https://www.gnu.org/software/bison/manual/bison.html#Examples
   - http://ashimg.tripod.com/example.html
   - https://gnuu.org/2009/09/18/writing-your-own-toy-compiler/
   - https://github.com/zakirullin/tiny-compiler
   - https://github.com/Parsons-David/A-Simple-Compiler
   - https://github.com/lotabout/write-a-C-interpreter
   - https://github.com/nineties/amber
   - http://catalog.compilertools.net/lexparse.html
   - http://catalog.compilertools.net/kits.html
   - https://www.ibm.com/support/knowledgecenter/en/ssw_aix_71/generalprogramming/ie_prog_4lex_yacc.html

## Frequently Asked Questions

### Could you please explain the directory structure of the project?

 - `lexer/` contains the Lexical Analyzer (Tokenizer or Scanner in other words) definition
which a Lex file named `lexer.l`
 - `parser/` contains the grammar file for the parser which is a Yacc file named `parser.y`
and several other functions related to parsing process.
 - `ast/` contains the functions related to building an Abstract Syntax Tree (AST) from the program file that parsed.
 - `interpreter/` contains the functions to register the Chaos modules and functions then
execute the program using the Abstract Syntax Tree.
 - `utilities/` contains the functions and macros used from everywhere in the project.
 - `tests/` contains the example Chaos programs, outputs of those programs and some Bash scripts to execute those programs
and then validate their output.

### How to debug a segmentation fault?

You can use GNU Debugger to debug a segmentation fault. In order to do that,
you need to simply compile the Chaos interpreter with `make dev` command which contains `-ggdb` option and then:

```
$ gdb
GNU gdb (Ubuntu 8.1-0ubuntu3.2) 8.1.0.20180409-git
Copyright (C) 2018 Free Software Foundation, Inc.
License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>
This is free software: you are free to change and redistribute it.
There is NO WARRANTY, to the extent permitted by law.  Type "show copying"
and "show warranty" for details.
This GDB was configured as "x86_64-linux-gnu".
Type "show configuration" for configuration details.
For bug reporting instructions, please see:
<http://www.gnu.org/software/gdb/bugs/>.
Find the GDB manual and other documentation resources online at:
<http://www.gnu.org/software/gdb/documentation/>.
For help, type "help".
Type "apropos word" to search for commands related to "word".
(gdb) file chaos
Reading symbols from chaos...done.
(gdb) run
Starting program: /home/mertyildiran/Documents/chaos/chaos
    Chaos Language 0.2.0 (Jan 20 2021 02:39:23)
    GCC version: 9.3.0 on linux
    Turn chaos into magic!

kaos> list a = [1, 2, 3]

Program received signal SIGSEGV, Segmentation fault.
__strcmp_ssse3 () at ../sysdeps/x86_64/multiarch/../strcmp.S:173
173	../sysdeps/x86_64/multiarch/../strcmp.S: No such file or directory.
(gdb) bt
#0  __strcmp_ssse3 () at ../sysdeps/x86_64/multiarch/../strcmp.S:173
#1  0x0000555555555489 in isDefined (name=0x0) at symbol.h:145
#2  0x0000555555554fd4 in addSymbol (name=0x0, type=INT, value=...) at symbol.h:33
#3  0x00005555555556b7 in addSymbolInt (name=0x0, i=1) at symbol.h:203
#4  0x00005555555564ae in yyparse () at parser.y:121
#5  0x00005555555569ea in main (argc=1, argv=0x7fffffffd798) at parser.y:145
(gdb)
```

### Is there a debug mode?

The Chaos interpreter has an option `-d` to enable the debug mode which tells you the details about
the parsing and execution order using the Abstract Syntax Tree (AST):

```
$ chaos -d
    Chaos Language 0.2.0 (Jan 20 2021 02:39:23)
    GCC version: 9.3.0 on linux
    Turn chaos into magic!

kaos> list a = [1, 2, 3]
(Create)	ASTNode: {id: 1, node_type: AST_LIST_START, module: /home/mertyildiran/Documents/chaos/__interactive__.kaos, string_size: 0}
(Create)	ASTNode: {id: 2, node_type: AST_EXPRESSION_VALUE, module: /home/mertyildiran/Documents/chaos/__interactive__.kaos, string_size: 0}
(Create)	ASTNode: {id: 3, node_type: AST_VAR_CREATE_NUMBER, module: /home/mertyildiran/Documents/chaos/__interactive__.kaos, string_size: 1}
(Create)	ASTNode: {id: 4, node_type: AST_EXPRESSION_VALUE, module: /home/mertyildiran/Documents/chaos/__interactive__.kaos, string_size: 0}
(Create)	ASTNode: {id: 5, node_type: AST_VAR_CREATE_NUMBER, module: /home/mertyildiran/Documents/chaos/__interactive__.kaos, string_size: 1}
(Create)	ASTNode: {id: 6, node_type: AST_EXPRESSION_VALUE, module: /home/mertyildiran/Documents/chaos/__interactive__.kaos, string_size: 0}
(Create)	ASTNode: {id: 7, node_type: AST_VAR_CREATE_NUMBER, module: /home/mertyildiran/Documents/chaos/__interactive__.kaos, string_size: 1}
(Create)	ASTNode: {id: 8, node_type: AST_STEP, module: /home/mertyildiran/Documents/chaos/__interactive__.kaos, string_size: 0}
(Create)	ASTNode: {id: 9, node_type: AST_STEP, module: /home/mertyildiran/Documents/chaos/__interactive__.kaos, string_size: 0}
(Create)	ASTNode: {id: 10, node_type: AST_LIST_NESTED_FINISH, module: /home/mertyildiran/Documents/chaos/__interactive__.kaos, string_size: 0}
(Create)	ASTNode: {id: 11, node_type: AST_VAR_CREATE_LIST, module: /home/mertyildiran/Documents/chaos/__interactive__.kaos, string_size: 1}
(Register)	ASTNode: {id: 11, node_type: AST_VAR_CREATE_LIST, module: /home/mertyildiran/Documents/chaos/__interactive__.kaos, string_size: 1}
(Execute)	ASTNode: {id: 1, node_type: AST_LIST_START, module: /home/mertyildiran/Documents/chaos/__interactive__.kaos, string_size: 0}
(Execute)	ASTNode: {id: 6, node_type: AST_EXPRESSION_VALUE, module: /home/mertyildiran/Documents/chaos/__interactive__.kaos, string_size: 0}
(Execute)	ASTNode: {id: 7, node_type: AST_VAR_CREATE_NUMBER, module: /home/mertyildiran/Documents/chaos/__interactive__.kaos, string_size: 1}
(Execute)	ASTNode: {id: 4, node_type: AST_EXPRESSION_VALUE, module: /home/mertyildiran/Documents/chaos/__interactive__.kaos, string_size: 0}
(Execute)	ASTNode: {id: 5, node_type: AST_VAR_CREATE_NUMBER, module: /home/mertyildiran/Documents/chaos/__interactive__.kaos, string_size: 1}
(Execute)	ASTNode: {id: 8, node_type: AST_STEP, module: /home/mertyildiran/Documents/chaos/__interactive__.kaos, string_size: 0}
(Execute)	ASTNode: {id: 2, node_type: AST_EXPRESSION_VALUE, module: /home/mertyildiran/Documents/chaos/__interactive__.kaos, string_size: 0}
(Execute)	ASTNode: {id: 3, node_type: AST_VAR_CREATE_NUMBER, module: /home/mertyildiran/Documents/chaos/__interactive__.kaos, string_size: 1}
(Execute)	ASTNode: {id: 9, node_type: AST_STEP, module: /home/mertyildiran/Documents/chaos/__interactive__.kaos, string_size: 0}
(Execute)	ASTNode: {id: 10, node_type: AST_LIST_NESTED_FINISH, module: /home/mertyildiran/Documents/chaos/__interactive__.kaos, string_size: 0}
(Execute)	ASTNode: {id: 11, node_type: AST_VAR_CREATE_LIST, module: /home/mertyildiran/Documents/chaos/__interactive__.kaos, string_size: 1}
kaos> print "hello world"
(Create)	ASTNode: {id: 12, node_type: AST_PRINT_STRING, module: /home/mertyildiran/Documents/chaos/__interactive__.kaos, string_size: 0}
(Register)	ASTNode: {id: 12, node_type: AST_PRINT_STRING, module: /home/mertyildiran/Documents/chaos/__interactive__.kaos, string_size: 0}
(Execute)	ASTNode: {id: 12, node_type: AST_PRINT_STRING, module: /home/mertyildiran/Documents/chaos/__interactive__.kaos, string_size: 0}
hello world
kaos> exit
(Create)	ASTNode: {id: 13, node_type: AST_EXIT_SUCCESS, module: /home/mertyildiran/Documents/chaos/__interactive__.kaos, string_size: 0}
(Register)	ASTNode: {id: 13, node_type: AST_EXIT_SUCCESS, module: /home/mertyildiran/Documents/chaos/__interactive__.kaos, string_size: 0}
(Execute)	ASTNode: {id: 13, node_type: AST_EXIT_SUCCESS, module: /home/mertyildiran/Documents/chaos/__interactive__.kaos, string_size: 0}
    You have chosen the order!
(Free)	ASTNode: {id: 1, node_type: AST_LIST_START, module: /home/mertyildiran/Documents/chaos/__interactive__.kaos, string_size: 0}
(Free)	ASTNode: {id: 6, node_type: AST_EXPRESSION_VALUE, module: /home/mertyildiran/Documents/chaos/__interactive__.kaos, string_size: 0}
(Free)	ASTNode: {id: 7, node_type: AST_VAR_CREATE_NUMBER, module: /home/mertyildiran/Documents/chaos/__interactive__.kaos, string_size: 1}
(Free)	ASTNode: {id: 4, node_type: AST_EXPRESSION_VALUE, module: /home/mertyildiran/Documents/chaos/__interactive__.kaos, string_size: 0}
(Free)	ASTNode: {id: 5, node_type: AST_VAR_CREATE_NUMBER, module: /home/mertyildiran/Documents/chaos/__interactive__.kaos, string_size: 1}
(Free)	ASTNode: {id: 8, node_type: AST_STEP, module: /home/mertyildiran/Documents/chaos/__interactive__.kaos, string_size: 0}
(Free)	ASTNode: {id: 2, node_type: AST_EXPRESSION_VALUE, module: /home/mertyildiran/Documents/chaos/__interactive__.kaos, string_size: 0}
(Free)	ASTNode: {id: 3, node_type: AST_VAR_CREATE_NUMBER, module: /home/mertyildiran/Documents/chaos/__interactive__.kaos, string_size: 1}
(Free)	ASTNode: {id: 9, node_type: AST_STEP, module: /home/mertyildiran/Documents/chaos/__interactive__.kaos, string_size: 0}
(Free)	ASTNode: {id: 10, node_type: AST_LIST_NESTED_FINISH, module: /home/mertyildiran/Documents/chaos/__interactive__.kaos, string_size: 0}
(Free)	ASTNode: {id: 11, node_type: AST_VAR_CREATE_LIST, module: /home/mertyildiran/Documents/chaos/__interactive__.kaos, string_size: 1}
(Free)	ASTNode: {id: 12, node_type: AST_PRINT_STRING, module: /home/mertyildiran/Documents/chaos/__interactive__.kaos, string_size: 0}
(Free)	ASTNode: {id: 13, node_type: AST_EXIT_SUCCESS, module: /home/mertyildiran/Documents/chaos/__interactive__.kaos, string_size: 0}
```

Then search the AST node type (for example `AST_LIST_START`) in:

 - `parser/parser.y` to determine the parser issues.
 - `ast/ast.c` to analyze the Abstract Syntax Tree building issues.
 - `interpreter/interpreter.c` to detect the issues related to execution of the program.

### How am I suppose to use Clang sanitizers?

Instead of compiling the Chaos interpreter using GCC (`make dev`), you can use Clang: `make clang-dev`

We have also additional Makefile rules to use Clang sanitizers:

 - MemorySanitizer: `make clang-dev-sanitizer-memory`
 - AddressSanitizer: `make clang-dev-sanitizer-address`
 - UndefinedBehaviorSanitizer: `make clang-dev-sanitizer-undefined_behavior`
