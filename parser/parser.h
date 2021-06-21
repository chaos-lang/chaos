/*
 * Description: Parser of the Chaos Programming Language's source
 *
 * Copyright (c) 2019-2021 Chaos Language Development Authority <info@chaos-lang.org>
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

#ifndef CHAOS_COMPILER
#include "../utilities/messages.h"
#endif

#ifndef CHAOS_COMPILER
#include "../compiler/compiler.h"
#include "../compiler/compiler_emit.h"
#endif

#include "../ast/ast_print.h"
#include "../vm/cpu.h"

extern int yyparse();
extern int yylex_destroy();
extern FILE* yyin;
extern char *yytext;
FILE *fp;
bool fp_opened;

#ifndef CHAOS_COMPILER
bool is_interactive;
FILE* tmp_stdin;
#endif

char *program_file_path;
char *program_file_dir;
char *program_code;
char *main_interpreted_module;
jmp_buf InteractiveShellErrorAbsorber;

i64 prev_stmt_count;
i64 prev_import_count;
i64_array* interactive_program;
bool interactively_importing;
bool compiling_a_function;

int initParser(int argc, char** argv);
void compile_interactive();
void freeEverything();
void yyerror(const char* s);

#ifndef CHAOS_COMPILER
void absorbError();
void throwCompilerInteractiveError();
void throwMissingOutputName();
void throwMissingCompileOption();
void throwMissingExtraFlags();
#endif

#endif
