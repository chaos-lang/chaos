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

typedef struct jit_op_array {
    jit_op** arr;
    i64 capacity;
    i64 size;
    i64 hlt_count;
} jit_op_array;

cpu *new_cpu(KaosIR* program, unsigned short debug_level);
void free_cpu(cpu *c);
void run_cpu(cpu *c);
void eat_until_hlt(cpu *c);
void fetch(cpu *c);
void execute(cpu *c);

jit_label_array* init_label_array();
void push_label(jit_label_array* label_array, jit_label* label);
jit_label* get_label(jit_label_array* label_array, i64 i);

jit_op_array* init_op_array();
void push_op(jit_op_array* op_array, jit_op* op);
jit_op* get_op(jit_op_array* op_array, i64 i);

void cpu_print(i64 r0, i64 r1, f64 fr1, i64 nl);
void cpu_print_bool(i64 i);
void cpu_print_int(i64 i);
void cpu_print_float(f64 f);
void cpu_print_string(i64 addr, bool quoted);
void cpu_print_list(i64 addr);

void cpu_delete_string_index(i64 index, i64 addr);
i64 cpu_string_concat(i64 addr1, i64 addr2);
i64 cpu_boolean_to_string(i64 val);
i64 cpu_string_to_boolean(i64 addr);
i64 cpu_list_index_access(i64 addr, i64 i);
void cpu_list_index_update(i64 addr, i64 i, i64 r0, i64 r1);

void debug(struct jit *jit);

#define DYN_BINARY_ARITH(_fn, _ffn) \
    /* Check if any of the operands are float */ \
    jit_op* float_op_label_1 = jit_beqi(_jit, JIT_FORWARD, R(0), V_FLOAT); \
    jit_op* float_op_label_2 = jit_beqi(_jit, JIT_FORWARD, R(4), V_FLOAT); \
\
    /* It's an integer operation, do the operation */ \
    _fn(_jit, R(1), R(1), R(5)); \
    jit_op* float_op_label_5 = jit_jmpi(_jit, JIT_FORWARD); \
\
    /* It's a float operation. Set the jump point to dodge the integer operation TODO: reduce to one */ \
    jit_patch(_jit, float_op_label_1); \
    jit_patch(_jit, float_op_label_2); \
\
    /* Check if left-hand operand is a float and cast it to float if it's not a float */ \
    jit_op* float_op_label_3 = jit_beqi(_jit, JIT_FORWARD, R(0), V_FLOAT); \
    jit_movi(_jit, R(0), V_FLOAT); \
    jit_extr(_jit, FR(1), R(1)); \
    jit_patch(_jit, float_op_label_3); \
\
    /* Check if right-hand operand is a float and cast it to float if it's not a float */ \
    jit_op* float_op_label_4 = jit_beqi(_jit, JIT_FORWARD, R(4), V_FLOAT); \
    jit_fmovr(_jit, FR(0), FR(1)); \
    jit_extr(_jit, FR(2), R(5)); \
    jit_patch(_jit, float_op_label_4); \
\
    /* Do the float operation */ \
    _ffn(_jit, FR(1), FR(2), FR(1)); \
\
    /* Set the jump point to dodge the float operation */ \
    jit_patch(_jit, float_op_label_5); \

#define DYN_UNARY_ARITH(_fn, _ffn) \
    /* Check if the operand is float */ \
    jit_op* float_op_label_1 = jit_beqi(_jit, JIT_FORWARD, R(0), V_FLOAT); \
\
    /* It's an integer operation, do the operation */ \
    _fn(_jit, R(1), R(1)); \
    jit_op* float_op_label_2 = jit_jmpi(_jit, JIT_FORWARD); \
\
    /* It's a float operation. Set the jump point to dodge the integer operation */ \
    jit_patch(_jit, float_op_label_1); \
\
    /* Do the float operation */ \
    _ffn(_jit, FR(1), FR(1)); \
\
    /* Set the jump point to dodge the float operation */ \
    jit_patch(_jit, float_op_label_2); \

#define DYN_BINARY_COMPARISON(_fn, _ffn) \
    /* Check if any of the operands are float */ \
    jit_op* float_op_label_1 = jit_beqi(_jit, JIT_FORWARD, R(0), V_FLOAT); \
    jit_op* float_op_label_2 = jit_beqi(_jit, JIT_FORWARD, R(4), V_FLOAT); \
\
    /* It's an integer operation, do the operation */ \
    jit_movi(_jit, R(3), 1); \
    jit_op* float_comp_label_true_int = _fn(_jit, JIT_FORWARD, R(1), R(5)); \
    jit_movi(_jit, R(3), 0); \
    jit_patch(_jit, float_comp_label_true_int); \
    jit_op* float_op_label_5 = jit_jmpi(_jit, JIT_FORWARD); \
\
    /* It's a float operation. Set the jump point to dodge the integer operation TODO: reduce to one */ \
    jit_patch(_jit, float_op_label_1); \
    jit_patch(_jit, float_op_label_2); \
\
    /* Check if left-hand operand is a float and cast it to float if it's not a float */ \
    jit_op* float_op_label_3 = jit_beqi(_jit, JIT_FORWARD, R(0), V_FLOAT); \
    jit_movi(_jit, R(0), V_FLOAT); \
    jit_extr(_jit, FR(1), R(1)); \
    jit_patch(_jit, float_op_label_3); \
\
    /* Check if right-hand operand is a float and cast it to float if it's not a float */ \
    jit_op* float_op_label_4 = jit_beqi(_jit, JIT_FORWARD, R(4), V_FLOAT); \
    jit_fmovr(_jit, FR(0), FR(1)); \
    jit_extr(_jit, FR(2), R(5)); \
    jit_patch(_jit, float_op_label_4); \
\
    /* Do the float operation */ \
    jit_movi(_jit, R(3), 1); \
    jit_op* float_comp_label_true_float = _ffn(_jit, JIT_FORWARD, FR(1), FR(2)); \
    jit_movi(_jit, R(3), 0); \
    jit_patch(_jit, float_comp_label_true_float); \
\
    /* Set the jump point to dodge the float operation */ \
    jit_patch(_jit, float_op_label_5); \
    jit_movr(_jit, R(1), R(3)); \

#endif
