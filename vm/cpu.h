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

#include "types.h"
#include "instructions.h"
#include "flags.h"

#include "../enums.h"
#include "../utilities/helpers.h"

cpu *new_cpu(i64 *memory, i64 mem_size, bool debug);
void free_cpu(cpu *c);
void run_cpu(cpu *c);
void fetch(cpu *c);
void execute(cpu *c);
void print_registers(cpu *c);
char *getRegName(i64 i);
f64 build_f64(i64 ipart, i64 frac, i64 leading_zeros);
i64 parse_f64(f64 f, i64* ipart, i64* frac);
void load_f64(cpu *c, f64 f);
void get_f64_operands(cpu *c, f64* f1, f64* f2);

#endif
