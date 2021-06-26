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

void eat_until_hlt(cpu *c)
{
    do {
        fetch_without_ast_stack(c);
    } while (c->inst != HLT);
}

static inline void fetch(cpu *c)
{
    // c->pc++;
    // c->inst = c->program[c->pc];
    // c->dest = c->program[c->pc + 1];
    // c->src = c->program[c->pc + 2];

    memcpy(&c->inst, &c->program[++c->pc], 3 * sizeof(i64));

    ast_stack[ast_stack_p] = c->ast_ref[c->pc];
}

void fetch_without_ast_stack(cpu *c)
{
    c->pc++;
    c->inst = c->program[c->pc];
    c->dest = c->program[c->pc + 1];
    c->src = c->program[c->pc + 2];
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

static inline void vm_clf(cpu *c)
{
    clear_flags(c);
}

static inline void vm_cmp(cpu *c)
{
    if (c->r[R0A] == V_FLOAT || c->r[R0B] == V_FLOAT) {
        f64 f1;
        f64 f2;
        i64 i1 = c->r[c->dest];
        i64 i2 = c->r[c->src];
        if (c->r[R0A] == V_FLOAT) {
            memcpy(&f1, &i1, sizeof f1);
        } else {
            c->r[R0A] = V_FLOAT;
            f1 = (f64)c->r[c->dest];
        }
        if (c->r[R0B] == V_FLOAT) {
            memcpy(&f2, &i2, sizeof f2);
        } else {
            f2 = (f64)c->r[c->src];
        }
        fset_flags(c, f1, f2);
    } else {
        set_flags(c, c->r[c->dest], c->r[c->src]);
    }
    c->pc += 2;
}

static inline void vm_cmpi(cpu* c)
{
    set_flags(c, c->r[c->dest], c->src);
    c->pc += 2;
}

static inline void vm_mov(cpu* c)
{
    c->r[c->dest] = c->r[c->src];
    c->pc += 2;
}

static inline void vm_sti(cpu* c)
{
    cpu_store(c, c->dest, c->r[c->src]);
    c->pc += 2;
}

static inline void vm_str(cpu* c)
{
    cpu_store(c, c->dest, c->r[c->src]);
    c->pc += 2;
}

static inline void vm_ldi(cpu* c)
{
    c->r[c->dest] = c->mem[c->src];
    c->pc += 2;
}

static inline void vm_ldr(cpu* c)
{
    c->r[c->dest] = c->mem[c->r[c->src]];
    c->pc += 2;
}

static inline void vm_lii(cpu* c)
{
    c->r[c->dest] = c->src;
    c->pc += 2;
}

static inline void vm_push(cpu* c)
{
    c->stack[--c->sp] = c->r[c->program[++c->pc]];
}

static inline void vm_pop(cpu* c)
{
    if (c->sp + 1 > USHRT_MAX * 2 - 1) {
        eat_until_hlt(c);
        throw_error(E_STACK_OVERFLOW);
    }
    c->r[c->program[++c->pc]] = c->stack[c->sp++];
}

static inline void vm_inc(cpu* c)
{
    c->r[c->dest]++;
    c->pc++;
}

static inline void vm_dec(cpu* c)
{
    c->r[c->dest]--;
    c->pc++;
}

static inline void vm_add(cpu* c)
{
    if (c->r[R0A] == V_FLOAT || c->r[R0B] == V_FLOAT) {
        f64 f1;
        f64 f2;
        i64 i1 = c->r[c->dest];
        i64 i2 = c->r[c->src];
        if (c->r[R0A] == V_FLOAT) {
            memcpy(&f1, &i1, sizeof f1);
        } else {
            c->r[R0A] = V_FLOAT;
            f1 = (f64)c->r[c->dest];
        }
        if (c->r[R0B] == V_FLOAT) {
            memcpy(&f2, &i2, sizeof f2);
        } else {
            f2 = (f64)c->r[c->src];
        }
        f1 = f1 + f2;
        i64 i;
        memcpy(&i, &f1, sizeof f1);
        c->r[c->dest] = i;
    } else {
        c->r[c->dest] += c->r[c->src];
    }
    c->pc += 2;
}

static inline void vm_sub(cpu* c)
{
    if (c->r[R0A] == V_FLOAT || c->r[R0B] == V_FLOAT) {
        f64 f1;
        f64 f2;
        i64 i1 = c->r[c->dest];
        i64 i2 = c->r[c->src];
        if (c->r[R0A] == V_FLOAT) {
            memcpy(&f1, &i1, sizeof f1);
        } else {
            c->r[R0A] = V_FLOAT;
            f1 = (f64)c->r[c->dest];
        }
        if (c->r[R0B] == V_FLOAT) {
            memcpy(&f2, &i2, sizeof f2);
        } else {
            f2 = (f64)c->r[c->src];
        }
        f1 = f1 - f2;
        i64 i;
        memcpy(&i, &f1, sizeof f1);
        c->r[c->dest] = i;
    } else {
        c->r[c->dest] -= c->r[c->src];
    }
    c->pc += 2;
}

static inline void vm_mul(cpu* c)
{
    if (c->r[R0A] == V_FLOAT || c->r[R0B] == V_FLOAT) {
        f64 f1;
        f64 f2;
        i64 i1 = c->r[c->dest];
        i64 i2 = c->r[c->src];
        if (c->r[R0A] == V_FLOAT) {
            memcpy(&f1, &i1, sizeof f1);
        } else {
            c->r[R0A] = V_FLOAT;
            f1 = (f64)c->r[c->dest];
        }
        if (c->r[R0B] == V_FLOAT) {
            memcpy(&f2, &i2, sizeof f2);
        } else {
            f2 = (f64)c->r[c->src];
        }
        f1 = f1 * f2;
        i64 i;
        memcpy(&i, &f1, sizeof f1);
        c->r[c->dest] = i;
    } else {
        c->r[c->dest] *= c->r[c->src];
    }
    c->pc += 2;
}

static inline void vm_div(cpu* c)
{
    if (c->r[R0A] == V_FLOAT || c->r[R0B] == V_FLOAT) {
        f64 f1;
        f64 f2;
        i64 i1 = c->r[c->dest];
        i64 i2 = c->r[c->src];
        if (c->r[R0A] == V_FLOAT) {
            memcpy(&f1, &i1, sizeof f1);
        } else {
            c->r[R0A] = V_FLOAT;
            f1 = (f64)c->r[c->dest];
        }
        if (c->r[R0B] == V_FLOAT) {
            memcpy(&f2, &i2, sizeof f2);
        } else {
            f2 = (f64)c->r[c->src];
        }
        f1 = f1 / f2;
        i64 i;
        memcpy(&i, &f1, sizeof f1);
        c->r[c->dest] = i;
    } else {
        c->r[c->dest] /= c->r[c->src];
    }
    c->pc += 2;
}

static inline void vm_mod(cpu* c)
{
    if (c->r[R0A] == V_FLOAT || c->r[R0B] == V_FLOAT) {
        f64 f1;
        f64 f2;
        i64 i1 = c->r[c->dest];
        i64 i2 = c->r[c->src];
        if (c->r[R0A] == V_FLOAT) {
            memcpy(&f1, &i1, sizeof f1);
        } else {
            c->r[R0A] = V_FLOAT;
            f1 = (f64)c->r[c->dest];
        }
        if (c->r[R0B] == V_FLOAT) {
            memcpy(&f2, &i2, sizeof f2);
        } else {
            f2 = (f64)c->r[c->src];
        }
        f1 = fmodl(f1, f2);
        i64 i;
        memcpy(&i, &f1, sizeof f1);
        c->r[c->dest] = i;
    } else {
        c->r[c->dest] %= c->r[c->src];
    }
    c->pc += 2;
}

static inline void vm_jlz(cpu* c)
{
    if (c->ltz) {
        ++(c->pc);
        c->pc = c->program[c->pc];
    }
    else c->pc++;
}

static inline void vm_jgz(cpu* c)
{
    if (c->gtz) {
        ++(c->pc);
        c->pc = c->program[c->pc];
    }
    else c->pc++;
}

static inline void vm_jez(cpu* c)
{
    if (c->zero) {
        ++(c->pc);
        c->pc = c->program[c->pc];
    }
    else c->pc++;
}

static inline void vm_jnz(cpu* c)
{
    if (!c->zero) {
        ++(c->pc);
        c->pc = c->program[c->pc];
    }
    else c->pc++;
}

static inline void vm_jmp(cpu* c)
{
    ++(c->pc);
    c->pc = c->program[c->pc];
}

static inline void vm_jmpb(cpu* c)
{
    c->pc = c->jmpb[--c->jmpbp];
}

static inline void vm_sjmpb(cpu* c)
{
    c->jmpb[c->jmpbp++] = c->dest;
    c->pc++;
}

static inline void vm_brk(cpu* c)
{
    c->pc = c->brk[--c->brkp];
}

static inline void vm_sbrk(cpu* c)
{
    c->brk[c->brkp++] = c->dest;
    c->pc++;
}

static inline void vm_cont(cpu* c)
{
    c->pc = c->cont[--c->contp];
}

static inline void vm_scont(cpu* c)
{
    c->cont[c->contp++] = c->dest;
    c->pc++;
}

static inline void vm_call(cpu* c)
{
    i64 *new_mem = (i64*)malloc(c->heap_size * sizeof(i64));
    c->mems[c->memp++] = c->mem;
    memcpy(new_mem, c->mem, c->heap_size * sizeof(i64));
    c->mem = new_mem;
    c->mems[c->memp++] = c->mem;

    ast_stack_p++;
}

static inline void vm_callx(cpu* c)
{
    --c->memp;
    c->mem = c->mems[--c->memp];
    free(c->mems[c->memp + 1]);

    ast_stack_p--;
}

static inline void vm_callext(cpu* c)
{
    _Function* function = (void *)c->dest;
    callFunctionFromDynamicLibrary(function, c);
    c->pc++;
}

static inline void vm_shl(cpu* c)
{
    c->r[c->dest] <<= c->r[c->src];
    c->pc += 2;
}

static inline void vm_shr(cpu* c)
{
    c->r[c->dest] >>= c->r[c->src];
    c->pc += 2;
}

static inline void vm_band(cpu* c)
{
    c->r[c->dest] &= c->r[c->src];
    c->pc += 2;
}

static inline void vm_bor(cpu* c)
{
    c->r[c->dest] |= c->r[c->src];
    c->pc += 2;
}

static inline void vm_bnot(cpu* c)
{
    c->r[c->dest] = ~c->r[c->dest];
    c->pc++;
}

static inline void vm_bxor(cpu* c)
{
    c->r[c->dest] ^= c->r[c->src];
    c->pc += 2;
}

static inline void vm_land(cpu* c)
{
    c->r[c->dest] = c->r[c->dest] && c->r[c->src];
    c->pc += 2;
}

static inline void vm_lor(cpu* c)
{
    c->r[c->dest] = c->r[c->dest] || c->r[c->src];
    c->pc += 2;
}

static inline void vm_lnot(cpu* c)
{
    c->r[c->dest] = !c->r[c->dest];
    c->pc++;
}

static inline void vm_dldr(cpu* c)
{
    i64 addr = c->mem[c->r[c->dest]];
    c->r[R0A] = c->mem[addr++];
    cpu_load_dynamic(c, addr);
    c->pc++;
}

static inline void vm_dstr(cpu* c)
{
    c->r[c->dest] = c->heap;
    cpu_store_dynamic(c);
    c->pc++;
}

static inline void vm_dpop(cpu* c)
{
    cpu_pop_dynamic(c);
}

static inline void vm_ddel(cpu* c)
{
    i64 addr = c->r[c->dest];
    enum ValueType value_type = c->mem[addr];
    addr++;
    i64 index = c->r[c->src];
    i64 len = c->mem[addr++];
    cpu_store(c, addr - 1, len - 1);
    if (index < 0)
        index = len + index;

    if (value_type == V_STRING || value_type == V_LIST) {
        for (i64 i = 0; i < len; i++) {
            if (i < index) {
                addr++;
                continue;
            }
            cpu_store(c, addr, c->mem[addr + 1]);
            addr++;
        }
    } else if (value_type == V_DICT) {
        char *key = build_string(c, c->r[R1A]);
        i64 shift_index = 0;
        i64 _addr = addr;
        for (size_t i = 0; i < len; i++) {
            addr = c->mem[_addr + 2 * i + 1];
            char *dict_key = build_string_from_addr(c, addr);

            if (strcmp(dict_key, key) == 0) {
                shift_index = i;
                break;
            }
            free(dict_key);
        }
        free(key);

        for (i64 i = shift_index; i < (len - 1); i++) {
            cpu_store(c, _addr + 2 * i + 1, c->mem[_addr + 2 * (i + 1) + 1]);
            cpu_store(c, _addr + 2 * i, c->mem[_addr + 2 * (i + 1)]);
        }
    }
    c->pc += 2;
}

static inline void vm_print(cpu* c)
{
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
        print_string(c, false);
        break;
    case V_LIST:
        print_list(c, false, 0);
        break;
    case V_DICT:
        print_dict(c, false, 0);
        break;
    default:
        break;
    }
}

static inline void vm_pprint(cpu* c)
{
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
        print_string(c, false);
        break;
    case V_LIST:
        print_list(c, true, 0);
        break;
    case V_DICT:
        print_dict(c, true, 0);
        break;
    default:
        break;
    }
}

static inline void vm_lind(cpu* c)
{
    cpu_list_index_access(c, c->r[c->dest], c->r[c->src]);
    c->pc += 2;
}

static inline void vm_ksrch(cpu* c)
{
    cpu_dict_key_search(c, c->r[c->dest], c->r[c->src]);
    c->pc += 2;
}

static inline void vm_exit(cpu* c)
{
    if (is_interactive)
        print_bye_bye();
    exit(c->r[R1A]);
}

static inline void vm_thrw(cpu* c)
{
    c->pc += 2;
    if (c->gtz) {
        i64 err_code = c->dest;
        i64 i = c->r[c->src];
        if (is_interactive) {
            eat_until_hlt(c);
        }
        throw_error(err_code, NULL, NULL, i);
    }
}

static inline void vm_debug(cpu* c)
{}

static inline void vm_hlt(cpu* c)
{}

static inline void execute(cpu *c)
{
    void (*f[53])() = {
        vm_clf,
        vm_cmp, vm_cmpi,
        vm_mov,
        vm_sti, vm_ldi, vm_str, vm_ldr,
        vm_lii,
        vm_push, vm_pop,
        vm_inc, vm_dec,
        vm_add, vm_sub, vm_mul, vm_div, vm_mod,
        vm_jlz, vm_jgz, vm_jez, vm_jnz, vm_jmp,
        vm_shl, vm_shr,
        vm_band, vm_bor, vm_bnot, vm_bxor,
        vm_land, vm_lor, vm_lnot,
        vm_dldr, vm_dstr, vm_dpop, vm_ddel,
        vm_print, vm_pprint,
        vm_lind, vm_ksrch,
        vm_jmpb, vm_sjmpb,
        vm_brk, vm_sbrk, vm_cont, vm_scont,
        vm_call, vm_callx, vm_callext,
        vm_debug,
        vm_hlt, vm_exit, vm_thrw
    };

    f[c->inst](c);
}

void run_cpu(cpu *c)
{
    do {
        fetch(c);
        execute(c);
    } while (c->inst != HLT);
}
