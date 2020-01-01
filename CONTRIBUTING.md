# Contributing to Chaos Language

## Code of Conduct

This project and everyone participating in it is governed by the [Code of Conduct](CODE_OF_CONDUCT.md).
By participating, you are expected to uphold this code. Please report unacceptable behavior to [info@chaos-lang.org](mailto:info@chaos-lang.org).

## Compiling Chaos Interpreter on Development Mode

```
make clean
make dev
sudo make install
tests/run.sh
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

## Tips & Tricks

### How to debug a segmentation fault?

Add `-ggdb` option to GCC like `gcc -o chaos chaos.tab.c lex.yy.c -ggdb` then:


```
mertyildiran@Corsair:~/Documents/chaos$ gdb
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
Chaos Language 0.0.1-alpha (Dec 26 2019 02:37:16)
GCC version: 7.4.0 on linux
Turn chaos into magic!

### list a = [1, 2, 3]

Program received signal SIGSEGV, Segmentation fault.
__strcmp_ssse3 () at ../sysdeps/x86_64/multiarch/../strcmp.S:173
173	../sysdeps/x86_64/multiarch/../strcmp.S: No such file or directory.
(gdb) bt
#0  __strcmp_ssse3 () at ../sysdeps/x86_64/multiarch/../strcmp.S:173
#1  0x0000555555555489 in isDefined (name=0x0) at symbol.h:145
#2  0x0000555555554fd4 in addSymbol (name=0x0, type=INT, value=...) at symbol.h:33
#3  0x00005555555556b7 in addSymbolInt (name=0x0, i=1) at symbol.h:203
#4  0x00005555555564ae in yyparse () at chaos.y:121
#5  0x00005555555569ea in main (argc=1, argv=0x7fffffffd798) at chaos.y:145
(gdb)
```

**Note:** `make dev` contains `gcc -o chaos chaos.tab.c lex.yy.c -ggdb` so just run `make dev` instead.
