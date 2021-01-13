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

#include "preemptive_errors.h"

extern int kaos_lineno;

void throw_preemptive_error_var(throw_preemptive_error_args in) {
    unsigned short code = in.code ? in.code : 0;
    _Function* function = in.function ? in.function : NULL;
    char *str1 = in.str1 ? in.str1 : "";
    char *str2 = in.str2 ? in.str2 : "";
    long long lld1 = in.lld1 ? in.lld1 : 0;
    unsigned long long llu = in.llu1 ? in.llu1 : 0;
    throw_preemptive_error_base(code, function, str1, str2, lld1, llu);
}

void throw_preemptive_error_base(
    unsigned short code,
    _Function* function,
    char *str1,
    char *str2,
    long long lld1,
    unsigned long long llu1
) {
    throw_error_base(code, str1, str2, lld1, llu1, true, function);
}
