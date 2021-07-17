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
plfv _f;
jit_op *skip_data;

jit_label_array* label_array = NULL;
jit_op_array* op_array = NULL;

char *reg_names[] = {
    "R0", "R1", "R2",  "R3",  "R4",  "R5",  "R6",  "R7",
    "R8", "R9", "R10", "R11", "R12", "R13", "R14", "R15"
};

i64* ast_stack = NULL;
i64 ast_stack_p = 0;

bool temp_disable_debug = false;

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
    op_array = init_op_array();
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

    if (c->debug_level == 3 || c->debug_level == 5) {
        printf("\n>>>>>>>>>> JIT_DEBUG_OPS <<<<<<<<<");
        jit_dump_ops(_jit, JIT_DEBUG_OPS);
        printf(">>>>>>>>>> JIT_DEBUG_OPS <<<<<<<<<\n");
        jit_dump_ops(_jit, JIT_DEBUG_CODE);
    }

    if (c->debug_level == 4) {
        jit_dump_ops(_jit, JIT_DEBUG_COMBINED);
    }

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
    // >>> Function Declaration <<<
    // declare_label
    case DECLARE_LABEL: {
        jit_label* __f = jit_get_label(_jit);
        push_label(label_array, __f);
        break;
    }
    // prolog
    case PROLOG: {
        jit_label* __f = jit_get_label(_jit);
        label_array->arr[c->inst->op1->value.i] = __f;
        jit_prolog(_jit, &_f);
        break;
    }
    case MAIN_PROLOG:
        jit_prolog(_jit, &_main);
        if (c->debug_level > 4) {
            // Fixes the "uninitialized register" warning that caused by the usage in the `debug` function
            // Initialize the integers registers
            jit_movi(_jit, R(0), 0);
            jit_movi(_jit, R(1), 0);
            jit_movi(_jit, R(2), 0);
            jit_movi(_jit, R(3), 0);
            jit_movi(_jit, R(4), 0);
            jit_movi(_jit, R(5), 0);
            jit_movi(_jit, R(6), 0);
            jit_movi(_jit, R(7), 0);
            // Initialize the floating-point registers
            jit_fmovi(_jit, FR(0), 0.0);
            jit_fmovi(_jit, FR(1), 0.0);
            jit_fmovi(_jit, FR(2), 0.0);
            jit_fmovi(_jit, FR(3), 0.0);
        }
        break;
    // declare_arg
    case DECLARE_ARG:
        jit_declare_arg(_jit, c->inst->op1->value.i, c->inst->op2->value.i);
        break;
    // getarg
    case GETARG:
        jit_getarg(_jit, R(c->inst->op1->reg), c->inst->op2->value.i);
        break;
    // ret
    case RETR:
        jit_retr(_jit, R(c->inst->op1->reg));
        break;
    case RETI:
        jit_reti(_jit, c->inst->op1->value.i);
        break;
    // >>> Function Calls <<<
    // prepare
    case PREPARE:
        temp_disable_debug = true;
        jit_prepare(_jit);
        break;
    // putarg
    case PUTARGR:
        jit_putargr(_jit, R(c->inst->op1->reg));
        break;
    case PUTARGI:
        jit_putargi(_jit, c->inst->op1->value.i);
        break;
    // retval
    case RETVAL:
        jit_retval(_jit, R(c->inst->op1->reg));
        break;
    // call
    case CALLR:
        jit_callr(_jit, R(c->inst->op1->reg));
        break;
    case CALL: {
        jit_op* __op = jit_call(_jit, label_array->arr[c->inst->op1->value.i]);
        push_op(op_array, __op);
        temp_disable_debug = false;
        break;
    }
    // >>> Transfer Operations <<<
    // mov
    case MOVR:
        jit_movr(_jit, R(c->inst->op1->reg), R(c->inst->op2->reg));
        break;
    case MOVI:
        jit_movi(_jit, R(c->inst->op1->reg), c->inst->op2->value.i);
        break;
    // fmov
    case FMOV:
        jit_fmovi(_jit, FR(c->inst->op1->reg), c->inst->op2->value.f);
        break;
    case FMOVR:
        jit_fmovr(_jit, FR(c->inst->op1->reg), FR(c->inst->op2->reg));
        break;
    // alloc
    case ALLOCAI: {
        int i = jit_allocai(_jit, c->inst->op2->value.i);
        c->stack[c->inst->op1->value.i] = i;
        break;
    }
    case REF_ALLOCAI:
        jit_addi(_jit, R(c->inst->op1->reg), R_FP, c->stack[c->inst->op2->value.i]);
        break;
    // >>> Load Operations <<<
    // ldr
    case LDR:
        jit_ldr(_jit, R(c->inst->op1->reg), R(c->inst->op2->reg), c->inst->op3->value.i);
        break;
    case LDXR:
        jit_ldxr(_jit, R(c->inst->op1->reg), R(c->inst->op2->reg), R(c->inst->op3->reg), c->inst->op4->value.i);
        break;
    // fldr
    case FLDR:
        jit_fldr(_jit, FR(c->inst->op1->reg), R(c->inst->op2->reg), c->inst->op3->value.i);
        break;
    case FLDXR:
        jit_fldxr(_jit, FR(c->inst->op1->reg), R(c->inst->op2->reg), R(c->inst->op3->reg), c->inst->op4->value.i);
        break;
    // >>> Store Operations <<<
    // str
    case STR:
        jit_str(_jit, R(c->inst->op1->reg), R(c->inst->op2->reg), c->inst->op3->value.i);
        break;
    case STXR:
        jit_stxr(_jit, R(c->inst->op1->reg), R(c->inst->op2->reg), R(c->inst->op3->reg), c->inst->op4->value.i);
        break;
    // fstr
    case FSTR:
        jit_fstr(_jit, R(c->inst->op1->reg), FR(c->inst->op2->reg), c->inst->op3->value.i);
        break;
    case FSTXR:
        jit_fstxr(_jit, R(c->inst->op1->reg), R(c->inst->op2->reg), FR(c->inst->op3->reg), c->inst->op4->value.i);
        break;
    // >>> Binary Arithmetic Operations <<<
    // add
    case ADDR:
        jit_addr(_jit, R(c->inst->op1->reg), R(c->inst->op2->reg), R(c->inst->op3->reg));
        break;
    case ADDI:
        jit_addi(_jit, R(c->inst->op1->reg), R(c->inst->op2->reg), c->inst->op3->value.i);
        break;
    // sub
    case SUBR:
        jit_subr(_jit, R(c->inst->op1->reg), R(c->inst->op2->reg), R(c->inst->op3->reg));
        break;
    case SUBI:
        jit_subi(_jit, R(c->inst->op1->reg), R(c->inst->op2->reg), c->inst->op3->value.i);
        break;
    // mul
    case MULR:
        jit_mulr(_jit, R(c->inst->op1->reg), R(c->inst->op2->reg), R(c->inst->op3->reg));
        break;
    case MULI:
        jit_muli(_jit, R(c->inst->op1->reg), R(c->inst->op2->reg), c->inst->op3->value.i);
        break;
    // div
    case DIVR:
        jit_divr(_jit, R(c->inst->op1->reg), R(c->inst->op2->reg), R(c->inst->op3->reg));
        break;
    case DIVI:
        jit_divi(_jit, R(c->inst->op1->reg), R(c->inst->op2->reg), c->inst->op3->value.i);
        break;
    // mod
    case MODR:
        jit_modr(_jit, R(c->inst->op1->reg), R(c->inst->op2->reg), R(c->inst->op3->reg));
        break;
    case MODI:
        jit_modi(_jit, R(c->inst->op1->reg), R(c->inst->op2->reg), c->inst->op3->value.i);
        break;
    // Binary Logic
    // and
    case ANDR:
        jit_andr(_jit, R(c->inst->op1->reg), R(c->inst->op2->reg), R(c->inst->op3->reg));
        break;
    case ANDI:
        jit_andi(_jit, R(c->inst->op1->reg), R(c->inst->op2->reg), c->inst->op3->value.i);
        break;
    // or
    case ORR:
        jit_orr(_jit, R(c->inst->op1->reg), R(c->inst->op2->reg), R(c->inst->op3->reg));
        break;
    case ORI:
        jit_ori(_jit, R(c->inst->op1->reg), R(c->inst->op2->reg), c->inst->op3->value.i);
        break;
    // xor
    case XORR:
        jit_xorr(_jit, R(c->inst->op1->reg), R(c->inst->op2->reg), R(c->inst->op3->reg));
        break;
    case XORI:
        jit_xori(_jit, R(c->inst->op1->reg), R(c->inst->op2->reg), c->inst->op3->value.i);
        break;
    // Binary Shift
    // lsh
    case LSHR:
        jit_lshr(_jit, R(c->inst->op1->reg), R(c->inst->op2->reg), R(c->inst->op3->reg));
        break;
    case LSHI:
        jit_lshi(_jit, R(c->inst->op1->reg), R(c->inst->op2->reg), c->inst->op3->value.i);
        break;
    // rsh
    case RSHR:
        jit_rshr(_jit, R(c->inst->op1->reg), R(c->inst->op2->reg), R(c->inst->op3->reg));
        break;
    case RSHI:
        jit_rshi(_jit, R(c->inst->op1->reg), R(c->inst->op2->reg), c->inst->op3->value.i);
        break;
    // >>> Unary Arithmetic Operations <<<
    // negr
    case NEGR:
        jit_negr(_jit, R(c->inst->op1->reg), R(c->inst->op2->reg));
        break;
    // fnegr
    case FNEGR:
        jit_fnegr(_jit, R(c->inst->op1->reg), R(c->inst->op2->reg));
        break;
    // notr
    case NOTR:
        jit_notr(_jit, R(c->inst->op1->reg), R(c->inst->op2->reg));
        break;
    // >>> Compare Instructions <<<
    // eqr
    case EQR:
        jit_eqr(_jit, R(c->inst->op1->reg), R(c->inst->op2->reg), R(c->inst->op3->reg));
        break;
    // ner
    case NER:
        jit_ner(_jit, R(c->inst->op1->reg), R(c->inst->op2->reg), R(c->inst->op3->reg));
        break;
    // gtr
    case GTR:
        jit_gtr(_jit, R(c->inst->op1->reg), R(c->inst->op2->reg), R(c->inst->op3->reg));
        break;
    // ltr
    case LTR:
        jit_ltr(_jit, R(c->inst->op1->reg), R(c->inst->op2->reg), R(c->inst->op3->reg));
        break;
    // ger
    case GER:
        jit_ger(_jit, R(c->inst->op1->reg), R(c->inst->op2->reg), R(c->inst->op3->reg));
        break;
    // ler
    case LER:
        jit_ler(_jit, R(c->inst->op1->reg), R(c->inst->op2->reg), R(c->inst->op3->reg));
        break;
    // >>> Branch Operations <<<
    // beq
    case BEQR: {
        jit_op* __op = jit_beqr(_jit, JIT_FORWARD, R(c->inst->op1->reg), R(c->inst->op2->reg));
        push_op(op_array, __op);
        break;
    }
    case BEQI: {
        jit_op* __op = jit_beqi(_jit, JIT_FORWARD, R(c->inst->op1->reg), c->inst->op2->value.i);
        push_op(op_array, __op);
        break;
    }
    // patch
    case PATCH:
        jit_patch(_jit, op_array->arr[c->inst->op1->value.i]);
        break;
    // >>> Non-Atomic Instructions <<<
    // Dynamic Instructions (prefixed with `DYN_`)
    // Dynamic Arithmetic
    case DYN_ADD: {
        jit_op* num_op_label_1 = jit_bnei(_jit, JIT_FORWARD, R(0), V_STRING);
        jit_op* num_op_label_2 = jit_bnei(_jit, JIT_FORWARD, R(4), V_STRING);

        jit_movi(_jit, R(2), cpu_string_concat);
        jit_prepare(_jit);
        jit_putargr(_jit, R(1));
        jit_putargr(_jit, R(5));
        jit_callr(_jit, R(2));
        jit_retval(_jit, R(1));
        jit_op* string_concat_op_label = jit_jmpi(_jit, JIT_FORWARD);

        jit_patch(_jit, num_op_label_1);
        jit_patch(_jit, num_op_label_2);
        DYN_BINARY_ARITH(jit_addr, jit_faddr);
        jit_patch(_jit, string_concat_op_label);
        break;
    }
    case DYN_SUB: {
        DYN_BINARY_ARITH(jit_subr, jit_fsubr);
        break;
    }
    case DYN_MUL: {
        DYN_BINARY_ARITH(jit_mulr, jit_fmulr);
        break;
    }
    case DYN_DIV: {
        DYN_BINARY_ARITH(jit_divr, jit_fdivr);
        break;
    }
    case DYN_NEG: {
        DYN_UNARY_ARITH(jit_negr, jit_fnegr);
        break;
    }
    case DYN_EQR: {
        DYN_BINARY_COMPARISON(jit_beqr, jit_fbeqr);
        break;
    }
    case DYN_NER: {
        DYN_BINARY_COMPARISON(jit_bner, jit_fbner);
        break;
    }
    case DYN_GTR: {
        DYN_BINARY_COMPARISON(jit_bgtr, jit_fbgtr);
        break;
    }
    case DYN_LTR: {
        DYN_BINARY_COMPARISON(jit_bltr, jit_fbltr);
        break;
    }
    case DYN_GER: {
        DYN_BINARY_COMPARISON(jit_bger, jit_fbger);
        break;
    }
    case DYN_LER: {
        DYN_BINARY_COMPARISON(jit_bler, jit_fbler);
        break;
    }
    // Dynamic Logic
    case DYN_LAND: {
        jit_gti(_jit, R(1), R(1), 0);
        jit_op* land_false_label = jit_beqi(_jit, JIT_FORWARD, R(1), 0);
        jit_gti(_jit, R(1), R(5), 0);
        jit_patch(_jit, land_false_label);
        break;
    }
    case DYN_LOR: {
        jit_gti(_jit, R(1), R(1), 0);
        jit_op* lor_true_label = jit_bnei(_jit, JIT_FORWARD, R(1), 0);
        jit_gti(_jit, R(1), R(5), 0);
        jit_patch(_jit, lor_true_label);
        break;
    }
    case DYN_LNOT: {
        jit_op* float_op_label = jit_bnei(_jit, JIT_FORWARD, R(0), V_FLOAT);
        jit_truncr(_jit, R(1), FR(1));
        jit_patch(_jit, float_op_label);
        jit_gti(_jit, R(1), R(1), 0);
        jit_xori(_jit, R(1), R(1), 0x00000001);
        break;
    }
    // Dynamic Printing
    case DYN_PRNT: {
        jit_movi(_jit, R(3), cpu_print);
        jit_prepare(_jit);
        jit_putargr(_jit, R(0));
        jit_putargr(_jit, R(1));
        jit_fputargr(_jit, FR(1), sizeof(double));
        jit_putargi(_jit, 1);
        jit_callr(_jit, R(3));
        break;
    }
    case DYN_ECHO: {
        jit_movi(_jit, R(3), cpu_print);
        jit_prepare(_jit);
        jit_putargr(_jit, R(0));
        jit_putargr(_jit, R(1));
        jit_fputargr(_jit, FR(1), sizeof(double));
        jit_putargi(_jit, 0);
        jit_callr(_jit, R(3));
        break;
    }
    // Dynamic Exit
    case DYN_EXIT: {
        jit_movi(_jit, R(2), exit);
        jit_prepare(_jit);
        jit_putargr(_jit, R(1));
        jit_callr(_jit, R(2));
        break;
    }
    // Dynamic Delete
    case DYN_STR_INDEX_DELETE: {
        jit_movi(_jit, R(2), cpu_delete_string_index);
        jit_prepare(_jit);
        jit_putargr(_jit, R(1));
        jit_putargr(_jit, R(5));
        jit_callr(_jit, R(2));
        break;
    }
    // Dynamic Index
    case DYN_STR_INDEX: {
        // R(1) holds the index value and the offset result should be in R(4)
        // Jump if the index is positive
        jit_op* positive_label = jit_bgti(_jit, JIT_FORWARD, R(1), -1);
        // Load the string length to R(2)
        jit_ldr(_jit, R(2), R(5), sizeof(size_t));
        // Turn negative index into positive index
        jit_addr(_jit, R(1), R(2), R(1));
        // The index is positive
        jit_patch(_jit, positive_label);
        // offset = index * sizeof(char)
        jit_muli(_jit, R(4), R(1), sizeof(char));
        // offset += sizeof(size_t)
        jit_addi(_jit, R(4), R(4), sizeof(size_t));
        // Index offset is available in R(4)
        break;
    }
    // Debug
    case DEBUG:
        debug(_jit);
        break;
    default:
        break;
    }

    if (c->debug_level > 4 && !temp_disable_debug)
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

jit_op_array* init_op_array()
{
    jit_op_array* op_array = malloc(sizeof *op_array);
    op_array->capacity = 0;
    op_array->arr = NULL;
    op_array->size = 0;
    return op_array;
}

void push_op(jit_op_array* op_array, jit_op* op)
{
    if (op_array->capacity == 0) {
        op_array->arr = (jit_op**)malloc((++op_array->capacity) * sizeof(jit_op*));
    } else {
        op_array->arr = (jit_op**)realloc(op_array->arr, (++op_array->capacity) * sizeof(jit_op*));
    }
    op_array->arr[op_array->size++] = op;
}

jit_op* get_op(jit_op_array* op_array, i64 i)
{
    return op_array->arr[i];
}

void cpu_print(i64 r0, i64 r1, f64 fr1, i64 nl)
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
        cpu_print_string(r1);
        break;
    case V_LIST:
        break;
    case V_DICT:
        break;
    default:
        break;
    }

    if (nl != 0)
        printf("\n");
}

void cpu_print_bool(i64 i)
{
    printf("%s", i ? "true" : "false");
}

void cpu_print_int(i64 i)
{
    printf("%lld", i);
}

void cpu_print_float(f64 f)
{
    printf("%lg", f);
}

void cpu_print_string(i64 addr)
{
    addr += sizeof(size_t);
    char *s = (char*)addr;
    printf("%s", escape_the_sequences_in_string_literal(s));
}

void cpu_delete_string_index(i64 index, i64 addr)
{
    size_t* t = (size_t*)addr;
    *t = *t - 1;
    addr += sizeof(size_t);
    char *s = (char*)addr;
    memmove(&s[index], &s[index + 1], strlen(s) - index);
}

i64 cpu_string_concat(i64 addr1, i64 addr2)
{
    size_t* t1 = (size_t*)addr1;
    size_t* t2 = (size_t*)addr2;
    size_t t3 = *t1 + *t2;
    addr1 += sizeof(size_t);
    addr2 += sizeof(size_t);
    char *s1 = (char*)addr1;
    char *s2 = (char*)addr2;

    // Allocate a new space to store the concatenated string
    i64 p = (i64)malloc((t3 + 1) * sizeof(char) + sizeof(size_t));

    // Set the new string size
    size_t* p_t = (size_t*)p;
    *p_t = t3;

    // Copy the first string
    p += sizeof(size_t);
    char *p_s = (char*)p;
    strcpy(p_s, s1);

    // Copy the second string
    p += *t1 * sizeof(char);
    char *p_s2 = (char*)p;
    strcpy(p_s2, s2);

    // Reset the pointer and return
    p -= sizeof(size_t) + *t1 * sizeof(char);
    return p;
}

void debug(struct jit *jit)
{
    jit_msg(jit, " ----------------------------------------------------------\n");
    jit_msgr(jit, "[R0: %lld] ", R(0));
    jit_msgr(jit, "[R1: %lld] ", R(1));
    jit_msgr(jit, "[R2: %lld] ", R(2));
    jit_msgr(jit, "[R3: %lld] ", R(3));
    jit_msgr(jit, "[R4: %lld] ", R(4));
    jit_msgr(jit, "[R5: %lld] ", R(5));
    jit_msgr(jit, "[R6: %lld] ", R(6));
    jit_msgr(jit, "[R7: %lld] | ", R(7));
    jit_fmsgr(jit, "[FR0: %lf] ", FR(0));
    jit_fmsgr(jit, "[FR1: %lf] ", FR(1));
    jit_fmsgr(jit, "[FR2: %lf] ", FR(2));
    jit_fmsgr(jit, "[FR3: %lf]", FR(3));
    jit_msg(jit, "\n");
}
