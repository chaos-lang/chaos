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

#include "cpu.h"

typedef long (*plfv)();
struct jit *_jit;
plfv _main;
jit_op *skip_data;

jit_label_array* label_array = NULL;

char *reg_names[] = {
    "R0", "R1", "R2",  "R3",  "R4",  "R5",  "R6",  "R7",
    "R8", "R9", "R10", "R11", "R12", "R13", "R14", "R15"
};

i64* ast_stack = NULL;
i64 ast_stack_p = 0;

cpu *new_cpu(KaosIR* program, unsigned short debug_level)
{
    cpu *c = malloc(sizeof(cpu));
    c->program = program;
    c->ic = 0;
    c->debug_level = debug_level;

    c->stack = (int*)malloc(USHRT_MAX * 256 * sizeof(int));

    // ast_stack = (i64*)malloc(USHRT_MAX * 256 * sizeof(i64));
    return c;
}

void free_cpu(cpu *c)
{
    free(c);
}

void run_cpu(cpu *c)
{
    label_array = init_label_array();
    _jit = jit_init();

    // jit_declare_arg(_jit, JIT_SIGNED_NUM, sizeof(long));
    // jit_getarg(_jit, R(0), 0);

    do {
        fetch(c);
        execute(c);
    } while (c->inst->op_code != HLT);

    jit_reti(_jit, 0);
    if (c->debug_level > 2)
        jit_check_code(_jit, JIT_WARN_ALL);
    jit_generate_code(_jit);
    _main();
}

void eat_until_hlt(cpu *c)
{
    do {
    } while (c->inst->op_code != HLT);
}

void fetch(cpu *c)
{
    c->inst = c->program->arr[c->ic++];
}

void execute(cpu *c)
{
    // i64 ic_start = c->ic;

    switch (c->inst->op_code) {
    case PROLOG:
        jit_prolog(_jit, &_main);
        break;
    case MOVI:
        jit_movi(_jit, R(c->inst->op1->reg), c->inst->op2->value.i);
        break;
    case MOVR:
        jit_movr(_jit, R(c->inst->op1->reg), R(c->inst->op2->reg));
        break;
    case FMOV:
        jit_fmovi(_jit, FR(c->inst->op1->reg), c->inst->op2->value.f);
        break;
    case ALLOCAI: {
        int i = jit_allocai(_jit, c->inst->op2->value.i);
        c->stack[c->inst->op1->value.i] = i;
        break;
    }
    case REF_ALLOCAI:
        jit_addi(_jit, R(c->inst->op1->reg), R_FP, c->stack[c->inst->op2->value.i]);
        break;
    case STR:
        jit_str(_jit, R(c->inst->op1->reg), R(c->inst->op2->reg), c->inst->op3->value.i);
        break;
    case STXR:
        jit_stxr(_jit, R(c->inst->op1->reg), R(c->inst->op2->reg), R(c->inst->op3->reg), c->inst->op4->value.i);
        break;
    case FSTR:
        jit_fstr(_jit, R(c->inst->op1->reg), FR(c->inst->op2->reg), c->inst->op3->value.i);
        break;
    case FSTXR:
        jit_fstxr(_jit, R(c->inst->op1->reg), R(c->inst->op2->reg), FR(c->inst->op3->reg), c->inst->op4->value.i);
        break;
    case LDR:
        jit_ldr(_jit, R(c->inst->op1->reg), R(c->inst->op2->reg), c->inst->op3->value.i);
        break;
    case LDXR:
        jit_ldxr(_jit, R(c->inst->op1->reg), R(c->inst->op2->reg), R(c->inst->op3->reg), c->inst->op4->value.i);
        break;
    case FLDR:
        jit_fldr(_jit, FR(c->inst->op1->reg), R(c->inst->op2->reg), c->inst->op3->value.i);
        break;
    case FLDXR:
        jit_fldxr(_jit, FR(c->inst->op1->reg), R(c->inst->op2->reg), R(c->inst->op3->reg), c->inst->op4->value.i);
        break;
    case SUBR:
        jit_subr(_jit, R(c->inst->op1->reg), R(c->inst->op2->reg), R(c->inst->op3->reg));
        break;
    case MULI:
        jit_muli(_jit, R(c->inst->op1->reg), R(c->inst->op2->reg), c->inst->op3->value.i);
        break;
    case PRNT: {
        jit_movi(_jit, R(2), cpu_print);
        jit_prepare(_jit);
        jit_putargr(_jit, R(0));
        jit_putargr(_jit, R(1));
        jit_fputargr(_jit, FR(1), sizeof(double));
        jit_callr(_jit, R(2));
        break;
    }
    case DEBUG:
        debug(_jit);
        break;
    default:
        break;
    }

    if (c->debug_level > 2)
        debug(_jit);
}

jit_label_array* init_label_array()
{
    jit_label_array* label_array = malloc(sizeof *label_array);
    label_array->capacity = 0;
    label_array->arr = NULL;
    label_array->size = 0;
    return label_array;
}

void push_label(jit_label_array* label_array, jit_label* label)
{
    if (label_array->capacity == 0) {
        label_array->arr = (jit_label**)malloc((++label_array->capacity) * sizeof(jit_label*));
    } else {
        label_array->arr = (jit_label**)realloc(label_array->arr, (++label_array->capacity) * sizeof(jit_label*));
    }
    label_array->arr[label_array->size++] = label;
}

jit_label* get_label(jit_label_array* label_array, i64 i)
{
    return label_array->arr[i];
}

void cpu_print(i64 r0, i64 r1, f64 fr1)
{
    switch (r0) {
    case V_BOOL:
        cpu_print_bool(r1);
        break;
    case V_INT:
        cpu_print_int(r1);
        break;
    case V_FLOAT:
        cpu_print_float(fr1);
        break;
    case V_STRING:
        break;
    case V_LIST:
        break;
    case V_DICT:
        break;
    default:
        break;
    }
}

void cpu_print_bool(i64 i)
{
    printf("%s\n", i ? "true" : "false");
}

void cpu_print_int(i64 i)
{
    printf("%lld\n", i);
}

void cpu_print_float(f64 f)
{
    printf("%lg\n", f);
}

void debug(struct jit *jit)
{
    jit_msg(jit, " ----------------------------------------------------------\n");
    jit_msgr(jit, "[R0: %lld] ", R(0));
    jit_msgr(jit, "[R1: %lld] ", R(1));
    jit_msgr(jit, "[R2: %lld] ", R(2));
    jit_msgr(jit, "[R3: %lld] | ", R(3));
    jit_msgr(jit, "[FR0: %lf] ", FR(0));
    jit_msgr(jit, "[FR1: %lf] ", FR(1));
    jit_msgr(jit, "[FR2: %lf] ", FR(2));
    jit_msgr(jit, "[FR3: %lf]\n", FR(3));
}
