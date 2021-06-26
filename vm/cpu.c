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

#include "../myjit/myjit/jitlib.h"

typedef long (*plfl)(long);
struct jit *_jit;

#include "cpu.h"

char *reg_names[] = {
    "R0A", "R1A", "R2A", "R3A", "R4A", "R5A", "R6A", "R7A",
    "R0B", "R1B", "R2B", "R3B", "R4B", "R5B", "R6B", "R7B"
};

i64* ast_stack = NULL;
i64 ast_stack_p = 0;

cpu *new_cpu(i64 *program, i64 heap_size, i64 start, i64 *ast_ref, unsigned short debug_level)
{
    cpu *c = malloc(sizeof(cpu));
    c->program = program;
    c->mems = (i64**)malloc(USHRT_MAX * 256 * sizeof(i64*));
    c->memp = 0;
    c->mem = (i64*)malloc((heap_size + 1) * sizeof(i64));
    c->stack = (i64*)malloc(USHRT_MAX * 256 * sizeof(i64));
    c->sp = USHRT_MAX * 2 - 1;
    c->heap_size = heap_size + 1;
    c->pc = -1 + start;
    c->inst = 0;
    c->heap = heap_size;
    c->debug_level = debug_level;
    c->jmpb = (i64*)malloc(USHRT_MAX * 256 * sizeof(i64));
    c->jmpbp = 0;
    c->brk = (i64*)malloc(USHRT_MAX * 256 * sizeof(i64));
    c->brkp = 0;
    c->cont = (i64*)malloc(USHRT_MAX * 256 * sizeof(i64));
    c->contp = 0;
    c->ast_ref = ast_ref;
    for (unsigned i = 0; i < NUM_REGISTERS; i++) {
        c->r[i] = 0;
    }

    ast_stack = (i64*)malloc(USHRT_MAX * 256 * sizeof(i64));
    return c;
}

void free_cpu(cpu *c)
{
    free(c);
    // free(ast_stack);
    // ast_stack = NULL;
    // ast_stack_p = 0;
}

void run_cpu(cpu *c)
{
    _jit = jit_init();

	plfl foo;

    jit_prolog(_jit, &foo);
    jit_declare_arg(_jit, JIT_SIGNED_NUM, sizeof(long));
    jit_getarg(_jit, R(0), 0);

    do {
        fetch(c);
        execute(c);
    } while (c->inst != HLT);

    jit_retr(_jit, R(0));
    jit_generate_code(_jit);

    printf("%li\n", foo(0));
}

void eat_until_hlt(cpu *c)
{
    do {
        fetch_without_ast_stack(c);
    } while (c->inst != HLT);
}

void fetch(cpu *c)
{
    c->pc++;
    c->inst = c->program[c->pc];
    c->dest = c->program[c->pc + 1];
    c->src = c->program[c->pc + 2];

    ast_stack[ast_stack_p] = c->ast_ref[c->pc];
}

void fetch_without_ast_stack(cpu *c)
{
    c->pc++;
    c->inst = c->program[c->pc];
    c->dest = c->program[c->pc + 1];
    c->src = c->program[c->pc + 2];
}

void execute(cpu *c)
{
    i64 pc_start = c->pc;

    // switch (c->inst) {
    // case LII:
    //     jit_movi(_jit, R(0), c->src);
    //     c->pc += 2;
    //     break;
    // default:
    //     break;
    // }

    if (c->debug_level > 2)
        debug(c, pc_start);
}

void print_registers(cpu *c, i64 pc_start)
{
    printf("[PC: %lld] ", pc_start);
    for (int i = 0; i < NUM_REGISTERS; i++) {
        printf("[%s: %lld] ", getRegName(i), c->r[i]);
    }
    printf("\n");
}

char *getRegName(i64 i)
{
    return reg_names[i];
}

char *build_string(cpu *c, i64 len)
{
    char *s = malloc(len + 1);
    for (size_t i = 0; i < len; i++) {
        s[i] = (int)c->stack[c->sp++] + '0';
    }
    s[len] = '\0';

    return s;
}

char *build_string_from_addr(cpu *c, i64 addr)
{
    i64 len = c->mem[addr];
    char *s = malloc(len + 1);
    for (size_t i = 0; i < len; i++) {
        s[i] = (int)c->mem[--addr] + '0';
    }
    s[len] = '\0';

    return s;
}

void print_bool(cpu *c)
{
    printf("%s", c->r[R1A] ? "true" : "false");
}

void print_int(cpu *c)
{
    printf("%lld", c->r[R1A]);
}

void print_float(cpu *c)
{
    i64 i = c->r[R1A];
    f64 f;
    memcpy(&f, &i, sizeof f);
    printf("%lg", f);
}

void print_string(cpu *c, bool quoted)
{
    size_t len = c->r[R1A];
    char *s = build_string(c, len);
    if (quoted)
        printf("'%s'", escape_the_sequences_in_string_literal(s));
    else
        printf("%s", escape_the_sequences_in_string_literal(s));
    free(s);
}

void print_list(cpu *c, bool pretty, unsigned long iter)
{
    size_t len = c->r[R1A];
    printf("[");
    if (pretty)
        printf("\n");
    iter++;
    for (size_t i = 0; i < len; i++) {
        if (pretty)
            for (unsigned long j = 0; j < iter; j++) {
                printf(__KAOS_TAB__);
            }
        c->r[R0A] = c->stack[c->sp++];
        c->r[R1A] = c->stack[c->sp++];
        switch (c->r[R0A]) {
        case V_BOOL:
            print_bool(c);
            break;
        case V_INT:
            print_int(c);
            break;
        case V_FLOAT:
            print_float(c);
            break;
        case V_STRING:
            print_string(c, true);
            break;
        case V_LIST:
            print_list(c, pretty, iter);
            break;
        case V_DICT:
            print_dict(c, pretty, iter);
            break;
        default:
            break;
        }
        if (i + 1 != len) {
            if (pretty)
                printf(",\n");
            else
                printf(", ");
        }
    }
    if (pretty)
        printf("\n");
    if (pretty) {
        for (unsigned long j = 0; j < (iter - 1); j++) {
            printf(__KAOS_TAB__);
        }
    }
    printf("]");
}

void print_dict(cpu *c, bool pretty, unsigned long iter)
{
    size_t len = c->r[R1A];
    printf("{");
    if (pretty)
        printf("\n");
    iter++;
    for (size_t i = 0; i < len; i++) {
        if (pretty)
            for (unsigned long j = 0; j < iter; j++) {
                printf(__KAOS_TAB__);
            }
        c->r[R0A] = c->stack[c->sp++];
        c->r[R1A] = c->stack[c->sp++];
        print_string(c, true);
        printf(": ");
        c->r[R0A] = c->stack[c->sp++];
        c->r[R1A] = c->stack[c->sp++];
        switch (c->r[R0A]) {
        case V_BOOL:
            print_bool(c);
            break;
        case V_INT:
            print_int(c);
            break;
        case V_FLOAT:
            print_float(c);
            break;
        case V_STRING:
            print_string(c, true);
            break;
        case V_LIST:
            print_list(c, pretty, iter);
            break;
        case V_DICT:
            print_dict(c, pretty, iter);
            break;
        default:
            break;
        }
        if (i + 1 != len) {
            if (pretty)
                printf(",\n");
            else
                printf(", ");
        }
    }
    if (pretty)
        printf("\n");
    if (pretty) {
        for (unsigned long j = 0; j < (iter - 1); j++) {
            printf(__KAOS_TAB__);
        }
    }
    printf("}");
}

void cpu_store_dynamic(cpu *c)
{
    switch (c->r[R0A]) {
        case V_BOOL:
            cpu_store_common(c);
            break;
        case V_INT:
            cpu_store_common(c);
            break;
        case V_FLOAT:
            cpu_store_common(c);
            break;
        case V_STRING:
            cpu_store_string(c);
            break;
        case V_LIST:
            cpu_store_list(c);
            break;
        case V_DICT:
            cpu_store_dict(c);
            break;
        default:
            break;
    }
}

void cpu_store_common(cpu *c)
{
    cpu_store(c, c->heap++, c->r[R0A]);
    c->r[R1A] = c->stack[c->sp++];
    cpu_store(c, c->heap++, c->r[R1A]);
}

void cpu_store_string(cpu *c)
{
    cpu_store_common(c);
    for (size_t i = c->r[R1A]; i > 0; i--) {
        c->r[R0A] = c->stack[c->sp++];
        cpu_store(c, c->heap++, c->r[R0A]);
    }
}

void cpu_store_list(cpu *c)
{
    cpu_store_common(c);
    c->heap += c->r[R1A];
    i64 _heap = c->heap - 1;
    for (size_t i = c->r[R1A]; i > 0; i--) {
        cpu_store(c, _heap--, c->heap);
        c->r[R0A] = c->stack[c->sp++];
        cpu_store_dynamic(c);
    }
}

void cpu_store_dict(cpu *c)
{
    cpu_store_common(c);
    c->heap += c->r[R1A] * 2;
    i64 _heap = c->heap - 1;
    for (size_t i = c->r[R1A]; i > 0; i--) {
        cpu_store(c, _heap--, c->heap);
        c->r[R0A] = c->stack[c->sp++];
        cpu_store_string(c);

        cpu_store(c, _heap--, c->heap);
        c->r[R0A] = c->stack[c->sp++];
        cpu_store_dynamic(c);
    }
}

void cpu_load_dynamic(cpu *c, i64 addr)
{
    switch (c->r[R0A]) {
        case V_BOOL:
            addr = cpu_load_common(c, addr);
            break;
        case V_INT:
            addr = cpu_load_common(c, addr);
            break;
        case V_FLOAT:
            addr = cpu_load_common(c, addr);
            break;
        case V_STRING:
            addr = cpu_load_string(c, addr);
            break;
        case V_LIST:
            addr = cpu_load_list(c, addr);
            break;
        case V_DICT:
            addr = cpu_load_dict(c, addr);
            break;
        default:
            break;
    }
}

i64 cpu_load_common(cpu *c, i64 addr)
{
    c->r[R1A] = c->mem[addr++];
    c->stack[--c->sp] = c->r[R1A];
    c->stack[--c->sp] = c->r[R0A];
    return addr;
}

i64 cpu_load_string(cpu *c, i64 addr)
{
    c->r[R1A] = c->mem[addr++];
    i64 _R0A = c->r[R0A];
    i64 _R1A = c->r[R1A];
    addr += c->r[R1A] - 1;
    for (size_t i = c->r[R1A]; i > 0; i--) {
        c->r[R2A] = c->mem[addr--];
        c->stack[--c->sp] = c->r[R2A];
    }
    addr += c->r[R1A] - 1;
    c->stack[--c->sp] = _R1A;
    c->stack[--c->sp] = _R0A;
    return addr;
}

i64 cpu_load_list(cpu *c, i64 addr)
{
    c->r[R1A] = c->mem[addr++];
    i64 _R0A = c->r[R0A];
    i64 _R1A = c->r[R1A];
    i64 _addr = addr;
    size_t len = c->r[R1A];
    for (size_t i = 0; i < len; i++) {
        addr = c->mem[_addr + i];
        c->r[R0A] = c->mem[addr++];
        cpu_load_dynamic(c, addr);
    }
    addr = _addr + len;
    c->stack[--c->sp] = _R1A;
    c->stack[--c->sp] = _R0A;
    return addr;
}

i64 cpu_load_dict(cpu *c, i64 addr)
{
    c->r[R1A] = c->mem[addr++];
    i64 _R0A = c->r[R0A];
    i64 _R1A = c->r[R1A];
    i64 _addr = addr;
    size_t len = c->r[R1A];
    for (size_t i = 0; i < len; i++) {
        addr = c->mem[_addr + 2 * i];
        c->r[R0A] = c->mem[addr++];
        cpu_load_dynamic(c, addr);

        addr = c->mem[_addr + 2 * i + 1];
        c->r[R0A] = c->mem[addr++];
        cpu_load_string(c, addr);
    }
    addr = _addr + 2 * len;
    c->stack[--c->sp] = _R1A;
    c->stack[--c->sp] = _R0A;
    return addr;
}

void cpu_eat_string(cpu *c)
{
    i64 len = c->stack[c->sp++];
    for (size_t i = len; i > 0; i--) {
        c->r[R0A] = c->stack[c->sp++];
    }
}

void cpu_eat_dynamic(cpu *c)
{
    switch (c->r[R0A]) {
        case V_BOOL:
            c->sp++;
            break;
        case V_INT:
            c->sp++;
            break;
        case V_FLOAT:
            c->sp++;
            break;
        case V_STRING: {
            cpu_eat_string(c);
            break;
        }
        case V_LIST: {
            i64 len = c->stack[c->sp++];
            for (size_t i = len; i > 0; i--) {
                c->r[R0A] = c->stack[c->sp++];
                cpu_eat_dynamic(c);
            }
            break;
        }
        case V_DICT: {
            i64 len = c->stack[c->sp++];
            for (size_t i = len; i > 0; i--) {
                c->r[R0A] = c->stack[c->sp++];
                cpu_eat_string(c);

                c->r[R0A] = c->stack[c->sp++];
                cpu_eat_dynamic(c);
            }
            break;
        }
        default:
            break;
    }
}

void cpu_pop_dynamic(cpu *c)
{
    switch (c->r[R0A]) {
        case V_BOOL:
            cpu_pop_common(c);
            break;
        case V_INT:
            cpu_pop_common(c);
            break;
        case V_FLOAT:
            cpu_pop_common(c);
            break;
        case V_STRING:
            cpu_pop_string(c);
            break;
        case V_LIST:
            cpu_pop_list(c);
            break;
        case V_DICT:
            cpu_pop_dict(c);
            break;
        default:
            break;
    }
}

void cpu_pop_common(cpu *c)
{
    c->r[R1A] = c->stack[c->sp++];
}

void cpu_pop_string(cpu *c)
{
    cpu_pop_common(c);
    for (size_t i = c->r[R1A]; i > 0; i--) {
        c->r[R0A] = c->stack[c->sp++];
    }
}

void cpu_pop_list(cpu *c)
{
    cpu_pop_common(c);
    for (size_t i = c->r[R1A]; i > 0; i--) {
        c->r[R0A] = c->stack[c->sp++];
        cpu_pop_dynamic(c);
    }
}

void cpu_pop_dict(cpu *c)
{
    cpu_pop_common(c);
    for (size_t i = c->r[R1A]; i > 0; i--) {
        c->r[R0A] = c->stack[c->sp++];
        cpu_pop_string(c);

        c->r[R0A] = c->stack[c->sp++];
        cpu_pop_dynamic(c);
    }
}

void cpu_list_index_access(cpu *c, i64 list_len, i64 index)
{
    if (index < 0)
        index = list_len + index;

    if (index < 0 || index >= list_len) {
        if (is_interactive) {
            c->pc += 2;
            eat_until_hlt(c);
        }
        throw_error(E_INDEX_OUT_OF_RANGE, NULL, NULL, index);
    }

    for (i64 i = 0; i < list_len; i++) {
        if (i == index) {
            c->r[R4A] = i;

            i64 addr = c->heap;
            c->r[R0A] = c->stack[c->sp++];
            cpu_store_dynamic(c);

            while (i < list_len - 1) {
                c->r[R0A] = c->stack[c->sp++];
                cpu_eat_dynamic(c);
                i++;
            }

            c->r[R0A] = c->mem[addr++];
            cpu_load_dynamic(c, addr);

            c->r[R0A] = c->stack[c->sp++];
            cpu_pop_common(c);
            return;
        } else {
            c->r[R0A] = c->stack[c->sp++];
            cpu_pop_dynamic(c);
        }
    }
}

void cpu_dict_key_search(cpu *c, i64 dict_len, i64 key_len)
{
    char *key = build_string(c, key_len);
    for (i64 i = dict_len + 1; i > 0; i--) {
        c->r[R0A] = c->stack[c->sp++];
        cpu_pop_common(c);
        char *dict_key = build_string(c, c->r[R1A]);
        c->r[R0A] = c->stack[c->sp++];

        if (strcmp(dict_key, key) == 0) {
            c->r[R4A] = dict_len - i + 1;
            free(key);
            free(dict_key);

            i64 addr = c->heap;
            cpu_store_dynamic(c);

            while (i > 2) {
                cpu_eat_string(c);
                cpu_eat_dynamic(c);
                i--;
            }

            c->r[R0A] = c->mem[addr++];
            cpu_load_dynamic(c, addr);

            c->r[R0A] = c->stack[c->sp++];
            cpu_pop_common(c);
            return;
        }
        free(dict_key);

        cpu_pop_dynamic(c);
    }
    free(key);
}

void debug(cpu *c, i64 pc_start)
{
    printf(" ----------------------------------------------------------\n");
    print_registers(c, pc_start);
    print_stack(c);
    print_heap(c);
}

void print_stack(cpu *c)
{
    for (i64 i = c->sp; i < USHRT_MAX * 2 - 1; i++) {
        printf("[SP: %lld] %lld\n", i, c->stack[i]);
    }
}

void print_heap(cpu *c)
{
    i64 heap_start = 0;
    if (is_interactive)
        heap_start = USHRT_MAX * 32;
    for (i64 i = heap_start; i < c->heap_size; i++) {
        printf("[HP: %lld: %lld] ", i, c->mem[i]);
    }
    printf("\n");
}

void cpu_store(cpu *c, i64 heap, i64 value)
{
    if (heap > (c->heap_size - 1)) {
        c->heap_size = heap + 1;
        c->mem = (i64*)realloc(c->mem, c->heap_size * sizeof(i64));
        c->mems[c->memp - 1] = c->mem;
    }
    c->mem[heap] = value;
}
