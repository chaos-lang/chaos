/*
 * Description: Preemptive Errors module of the Chaos Programming Language's source
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

#ifndef KAOS_PREEMPTIVE_ERRORS_H
#define KAOS_PREEMPTIVE_ERRORS_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <setjmp.h>
#include <string.h>

#include "../utilities/language.h"
#include "../interpreter/errors.h"

void print_preemptive_error_base(
    unsigned short code,
    _Function* function,
    char *str1,
    char *str2,
    long long lld1,
    unsigned long long llu1
);
void throw_preemptive_errors();

typedef struct {
    unsigned short code;
    _Function* function;
    char *str1;
    char *str2;
    long long lld1;
    unsigned long long llu1;
} throw_preemptive_error_args;

typedef struct throw_preemptive_error_args_array {
    throw_preemptive_error_args* arr;
    unsigned capacity, size;
} throw_preemptive_error_args_array;

typedef struct preemptive_error_lineno_array {
    int* value;
    unsigned capacity, size;
} preemptive_error_lineno_array;

throw_preemptive_error_args_array preemptive_errors;
preemptive_error_lineno_array preemptive_errors_lineno;

void add_preemptive_error_var(throw_preemptive_error_args in);
void print_preemptive_error(throw_preemptive_error_args in);
void free_preemptive_errors();

#define add_preemptive_error(...) add_preemptive_error_var((throw_preemptive_error_args){__VA_ARGS__});

#endif
