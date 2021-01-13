/*
 * Description: Errors module of the Chaos Programming Language's source
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

#ifndef KAOS_ERRORS_H
#define KAOS_ERRORS_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <setjmp.h>
#include <string.h>

#include "../utilities/language.h"
#include "function.h"

enum ExitCode {
    E_SUCCESS,
    E_SYNTAX_ERROR,
    E_UNKNOWN_VARIABLE_TYPE,
    E_VARIABLE_ALREADY_DEFINED,
    E_UNDEFINED_VARIABLE,
    E_MEMORY_ALLOCATION_FOR_LIST_FAILED,
    E_ILLEGAL_ELEMENT_TYPE_FOR_TYPED_LIST,
    E_VARIABLE_IS_NOT_AN_LIST,
    E_INDEX_OUT_OF_RANGE,
    E_ILLEGAL_VARIABLE_TYPE_FOR_VARIABLE,
    E_VARIABLE_IS_NOT_A_DICTIONARY,
    E_UNDEFINED_KEY,
    E_UNRECOGNIZED_COMPLEX_DATA_TYPE,
    E_ILLEGAL_VARIABLE_TYPE_FOR_FUNCTION,
    E_UNDEFINED_FUNCTION,
    E_MEMORY_ALLOCATION_FOR_FUNCTION_FAILED,
    E_UNEXPECTED_VALUE_TYPE,
    E_FUNCTION_DID_NOT_RETURN_ANYTHING,
    E_MODULE_IS_EMPTY_OR_NOT_EXISTS_ON_PATH,
    E_NO_VARIABLE_WITH_ID,
    E_INDEX_OUT_OF_RANGE_STRING,
    E_ILLEGAL_CHARACTER_ASSIGNMENT_FOR_STRING,
    E_NOT_A_CHARACTER,
    E_PROGRAM_FILE_DOES_NOT_EXISTS_ON_PATH,
    E_INCORRECT_FUNCTION_ARGUMENT_COUNT,
    E_NOT_A_LIST,
    E_NOT_A_DICT,
    E_RAISED_FROM_AN_EXTENSION,
    E_ILLEGAL_VARIABLE_TYPE_FOR_FUNCTION_PARAMETER,
    E_UNEXPECTED_ACCESSOR_DATA_TYPE,
    E_INVALID_OPTION,
    E_NEGATIVE_ITERATION_COUNT,
    E_BREAK_CALL_OUTSIDE_LOOP,
    E_CONTINUE_CALL_OUTSIDE_LOOP,
    E_BREAK_CALL_MULTILINE_LOOP,
    E_CONTINUE_CALL_MULTILINE_LOOP
};

extern void freeEverything();

#ifndef CHAOS_COMPILER
extern void absorbError();
#endif

#ifndef CHAOS_COMPILER
extern bool is_interactive;
#endif

extern jmp_buf InteractiveShellErrorAbsorber;

void throw_error_base(
    unsigned short code,
    char *str1,
    char *str2,
    long long lld1,
    unsigned long long llu1,
    bool is_preemptive,
    _Function* function
);

typedef struct {
    unsigned short code;
    char *str1;
    char *str2;
    long long lld1;
    unsigned long long llu1;
    bool is_preemptive;
    _Function* function;
} throw_error_args;

void throw_error_var(throw_error_args in);

#define throw_error(...) throw_error_var((throw_error_args){__VA_ARGS__});

int InteractiveShellErrorAbsorber_ws_col;

#endif
