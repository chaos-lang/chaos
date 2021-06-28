/*
 * Description: CPU module of the Chaos Programming Language's source
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

#ifndef CPU_H
#define CPU_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

#include "ir.h"

#include "../enums.h"
#include "../utilities/helpers.h"

#include "../myjit/myjit/jitlib.h"

i64* ast_stack;
i64 ast_stack_p;

typedef struct jit_label_array {
    jit_label** arr;
    i64 capacity;
    i64 size;
    i64 hlt_count;
} jit_label_array;

cpu *new_cpu(KaosIR* program, unsigned short debug_level);
void free_cpu(cpu *c);
void run_cpu(cpu *c);
void eat_until_hlt(cpu *c);
void fetch(cpu *c);
void execute(cpu *c);

jit_label_array* init_label_array();
void push_label(jit_label_array* label_array, jit_label* label);
jit_label* get_label(jit_label_array* label_array, i64 i);

#endif
