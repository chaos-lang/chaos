/*
 * Description: Preemptive Errors module of the Chaos Programming Language's source
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

#ifndef KAOS_PREEMPTIVE_ERRORS_H
#define KAOS_PREEMPTIVE_ERRORS_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <setjmp.h>
#include <string.h>

#include "../utilities/language.h"
#include "../interpreter/errors.h"

void throw_preemptive_error_base(
    unsigned short code,
    char *str1,
    char *str2,
    long long lld1,
    unsigned long long llu1
);

typedef struct {
    unsigned short code;
    char *str1;
    char *str2;
    long long lld1;
    unsigned long long llu1;
} throw_preemptive_error_args;

void throw_preemptive_error_var(throw_preemptive_error_args in);

#define throw_preemptive_error(...) throw_preemptive_error_var((throw_preemptive_error_args){__VA_ARGS__});

#endif
