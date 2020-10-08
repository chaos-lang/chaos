#ifndef PARSER_H
#define PARSER_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#if defined(__APPLE__) && defined(__MACH__)
#include <sys/syslimits.h>
#elif !defined(_WIN32) && !defined(_WIN64) && !defined(__CYGWIN__)
#include <linux/limits.h>
#endif

#include "../interpreter/loop.h"
#include "../compiler/compiler.h"

extern int yyparse();
extern int yylex_destroy();
extern FILE* yyin;
extern char *yytext;
FILE *fp;
bool fp_opened;
bool is_interactive;
char *program_file_path;
char *program_file_dir;
char *program_code;
char *main_interpreted_module;
jmp_buf InteractiveShellErrorAbsorber;

int initParser(int argc, char** argv);
void freeEverything();
void yyerror(const char* s);
void absorbError();
void throwCompilerInteractiveError();
void throwMissingOutputName();
void throwMissingCompileOption();

#endif