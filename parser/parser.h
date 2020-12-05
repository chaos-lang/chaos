/*
 * Description: Parser of the Chaos Programming Language's source
 *
 * Copyright (c) 2019-2020 Chaos Language Development Authority <info@chaos-lang.org>
 *
 * License: GNU General Public License v3.0
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>
 *
 * Authors: M. Mert Yildiran <me@mertyildiran.com>
 */

#ifndef KAOS_PARSER_H
#define KAOS_PARSER_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#if defined(__APPLE__) && defined(__MACH__)
#   include <sys/syslimits.h>
#elif !defined(_WIN32) && !defined(_WIN64) && !defined(__CYGWIN__)
#   include <linux/limits.h>
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
void throwMissingExtraFlags();

#endif
