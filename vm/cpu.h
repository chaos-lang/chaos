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

cpu *new_cpu(i64 *program, i64 heap_size, i64 start, bool debug);
void free_cpu(cpu *c);
void run_cpu(cpu *c);
void fetch(cpu *c);
void execute(cpu *c);

void print_registers(cpu *c, i64 pc_start);
char *getRegName(i64 i);
char *build_string(cpu *c, i64 len);
char *build_string_from_addr(cpu *c, i64 addr);

void print_bool(cpu *c);
void print_int(cpu *c);
void print_float(cpu *c);
void print_string(cpu *c, bool quoted);
void print_list(cpu *c, bool pretty, unsigned long iter);
void print_dict(cpu *c, bool pretty, unsigned long iter);

void cpu_store_dynamic(cpu *c);
void cpu_store_common(cpu *c);
void cpu_store_string(cpu *c);
void cpu_store_list(cpu *c);
void cpu_store_dict(cpu *c);

void cpu_load_dynamic(cpu *c, i64 addr);
i64 cpu_load_common(cpu *c, i64 addr);
i64 cpu_load_string(cpu *c, i64 addr);
i64 cpu_load_list(cpu *c, i64 addr);
i64 cpu_load_dict(cpu *c, i64 addr);

void cpu_eat_string(cpu *c);
void cpu_eat_dynamic(cpu *c);

void cpu_pop_dynamic(cpu *c);
void cpu_pop_common(cpu *c);
void cpu_pop_string(cpu *c);
void cpu_pop_list(cpu *c);
void cpu_pop_dict(cpu *c);

void cpu_list_index_access(cpu *c, i64 list_len, i64 index);
void cpu_dict_key_search(cpu *c, i64 dict_len, i64 key_len);
void print_stack(cpu *c);

void cpu_store(cpu *c, i64 heap, i64 value);

#endif
