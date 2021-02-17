/*
 * Description: Parser of the Chaos Programming Language's source
 *
 * Copyright (c) 2019-2020 Chaos Language Development Authority
 * <info@chaos-lang.org>
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

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef CHAOS_COMPILER
#include "../utilities/messages.h"
#endif

#include "../interpreter/loop.h"

#ifndef CHAOS_COMPILER
#include "../compiler/compiler.h"
#endif

extern int yyparse();
extern int yylex_destroy();
extern FILE *yyin;
extern char *yytext;
FILE *fp;
bool fp_opened;

#ifndef CHAOS_COMPILER
bool is_interactive;
FILE *tmp_stdin;
#endif

char *program_file_path;
char *program_file_dir;
char *program_code;
char *main_interpreted_module;
jmp_buf InteractiveShellErrorAbsorber;

bool global_unsafe;

int initParser(int argc, char **argv);
void freeEverything();
void yyerror(const char *s);

#ifndef CHAOS_COMPILER
void absorbError();
void throwCompilerInteractiveError();
void throwMissingOutputName();
void throwMissingCompileOption();
void throwMissingExtraFlags();
#endif

#endif
