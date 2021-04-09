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

cpu *new_cpu(i64 *memory, i64 mem_size, i64 heap, bool debug)
{
	cpu *c = malloc(sizeof(cpu));
	c->mem = memory;
	c->sp = mem_size - 1;
	c->max_mem = mem_size;
	c->pc = -1;
	c->inst = 0;
    c->heap = heap;
    c->debug = debug;
    for (unsigned i = 0; i < NUM_REGISTERS; i++) {
        c->r[i] = 0;
    }
	return c;
}

void free_cpu(cpu *c)
{
	free(c);
}

void run_cpu(cpu *c)
{
	while (c->inst != HLT) {
		fetch(c);
		execute(c);
	}
}

void fetch(cpu *c)
{
	c->pc++;
	c->inst = c->mem[c->pc];
	c->dest = c->mem[c->pc+1];
	c->src = c->mem[c->pc+2];
}

void execute(cpu *c)
{
    i64 pc_start = c->pc;

	switch (c->inst) {
    case CLF:
        clear_flags(c);
        break;
    case CMP:
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
        break;
    case CMPI:
        set_flags(c, c->r[c->dest], c->src);
        c->pc += 2;
        break;
    case MOV:
        c->r[c->dest] = c->r[c->src];
        c->pc += 2;
        break;
    case STI:
        c->mem[c->dest] = c->r[c->src];
        c->pc += 2;
        break;
    case STR:
        c->mem[c->r[c->dest]] = c->r[c->src];
        c->pc += 2;
        break;
    case LDI:
        c->r[c->dest] = c->mem[c->src];
        c->pc += 2;
        break;
    case LDR:
        c->r[c->dest] = c->mem[c->r[c->src]];
        c->pc += 2;
        break;
    case LII:
        c->r[c->dest] = c->src;
        c->pc += 2;
        break;
    case PUSH:
        c->mem[--c->sp] = c->r[c->mem[++c->pc]];
        break;
    case POP:
        c->r[c->mem[++c->pc]] = c->mem[c->sp++];
        break;
    case INC:
        c->r[c->dest]++;
        c->pc++;
        break;
    case DEC:
        c->r[c->dest]--;
        c->pc++;
        break;
    case ADD:
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
        break;
    case SUB:
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
        break;
    case MUL:
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
        break;
    case DIV:
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
        break;
    case MOD:
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
        break;
    case JLZ:
        if (c->ltz) {
            ++(c->pc);
            c->pc = c->mem[c->pc];
        }
        else c->pc++;
        break;
    case JGZ:
        if (c->gtz) {
            ++(c->pc);
            c->pc = c->mem[c->pc];
        }
        else c->pc++;
        break;
    case JEZ:
        if (c->zero) {
            ++(c->pc);
            c->pc = c->mem[c->pc];
        }
        else c->pc++;
        break;
    case JNZ:
        if (!c->zero) {
            ++(c->pc);
            c->pc = c->mem[c->pc];
        }
        else c->pc++;
        break;
    case JMP:
        ++(c->pc);
        c->pc = c->mem[c->pc];
        break;
    case SHL:
        c->r[c->dest] <<= c->r[c->src];
        c->pc += 2;
        break;
    case SHR:
        c->r[c->dest] >>= c->r[c->src];
        c->pc += 2;
        break;
    case BAND:
        c->r[c->dest] &= c->r[c->src];
        c->pc += 2;
        break;
    case BOR:
        c->r[c->dest] |= c->r[c->src];
        c->pc += 2;
        break;
    case BNOT:
        c->r[c->dest] = ~c->r[c->dest];
        c->pc++;
        break;
    case BXOR:
        c->r[c->dest] ^= c->r[c->src];
        c->pc += 2;
        break;
    case LAND:
        c->r[c->dest] = c->r[c->dest] && c->r[c->src];
        c->pc += 2;
        break;
    case LOR:
        c->r[c->dest] = c->r[c->dest] || c->r[c->src];
        c->pc += 2;
        break;
    case LNOT:
        c->r[c->dest] = !c->r[c->dest];
        c->pc++;
        break;
    case DLDR: {
        i64 addr = c->mem[c->r[c->dest]];
        c->r[R0A] = c->mem[addr++];
        cpu_load_dynamic(c, addr);
        c->pc++;
        break;
    }
    case DSTR:
        c->r[c->dest] = c->heap;
        cpu_store_dynamic(c);
        c->pc++;
        break;
    case DPOP:
        cpu_pop_dynamic(c);
        break;
    case DDEL: {
        i64 addr = c->r[c->dest];
        enum ValueType value_type = c->mem[addr];
        addr++;
        i64 index = c->r[c->src];
        i64 len = c->mem[addr++];
        c->mem[addr - 1] = len - 1;
        if (index < 0)
            index = len + index;

        if (value_type == V_STRING || value_type == V_LIST) {
            for (i64 i = 0; i < len; i++) {
                if (i < index) {
                    addr++;
                    continue;
                }
                c->mem[addr] = c->mem[addr + 1];
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
                c->mem[_addr + 2 * i + 1] = c->mem[_addr + 2 * (i + 1) + 1];
                c->mem[_addr + 2 * i] = c->mem[_addr + 2 * (i + 1)];
            }
        }
        c->pc += 2;
        break;
    }
    case PRNT:
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
            print_list(c);
            break;
        case V_DICT:
            print_dict(c);
            break;
        default:
            break;
        }
        break;
    case KSRCH:
        cpu_dict_key_search(c, c->r[c->dest], c->r[c->src]);
        c->pc += 2;
        break;
    default:
        break;
	}

    if (c->debug)
        print_registers(c, pc_start);
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
        s[i] = (int)c->mem[c->sp++] + '0';
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

void print_list(cpu *c)
{
    size_t len = c->r[R1A];
    printf("[");
    for (size_t i = 0; i < len; i++) {
        c->r[R0A] = c->mem[c->sp++];
        c->r[R1A] = c->mem[c->sp++];
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
            print_list(c);
            break;
        case V_DICT:
            print_dict(c);
            break;
        default:
            break;
        }
        if (i + 1 != len)
            printf(", ");
    }
    printf("]");
}

void print_dict(cpu *c)
{
    size_t len = c->r[R1A];
    printf("{");
    for (size_t i = 0; i < len; i++) {
        c->r[R0A] = c->mem[c->sp++];
        c->r[R1A] = c->mem[c->sp++];
        print_string(c, true);
        printf(": ");
        c->r[R0A] = c->mem[c->sp++];
        c->r[R1A] = c->mem[c->sp++];
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
            print_list(c);
            break;
        case V_DICT:
            print_dict(c);
            break;
        default:
            break;
        }
        if (i + 1 != len)
            printf(", ");
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
    c->mem[c->heap++] = c->r[R0A];
    c->r[R1A] = c->mem[c->sp++];
    c->mem[c->heap++] = c->r[R1A];
}

void cpu_store_string(cpu *c)
{
    cpu_store_common(c);
    for (size_t i = c->r[R1A]; i > 0; i--) {
        c->r[R0A] = c->mem[c->sp++];
        c->mem[c->heap++] = c->r[R0A];
    }
}

void cpu_store_list(cpu *c)
{
    cpu_store_common(c);
    c->heap += c->r[R1A];
    i64 _heap = c->heap - 1;
    for (size_t i = c->r[R1A]; i > 0; i--) {
        c->mem[_heap--] = c->heap;
        c->r[R0A] = c->mem[c->sp++];
        cpu_store_dynamic(c);
    }
}

void cpu_store_dict(cpu *c)
{
    cpu_store_common(c);
    c->heap += c->r[R1A] * 2;
    i64 _heap = c->heap - 1;
    for (size_t i = c->r[R1A]; i > 0; i--) {
        c->mem[_heap--] = c->heap;
        c->r[R0A] = c->mem[c->sp++];
        cpu_store_string(c);

        c->mem[_heap--] = c->heap;
        c->r[R0A] = c->mem[c->sp++];
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
    c->mem[--c->sp] = c->r[R1A];
    c->mem[--c->sp] = c->r[R0A];
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
        c->mem[--c->sp] = c->r[R2A];
    }
    addr += c->r[R1A] - 1;
    c->mem[--c->sp] = _R1A;
    c->mem[--c->sp] = _R0A;
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
    c->mem[--c->sp] = _R1A;
    c->mem[--c->sp] = _R0A;
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
    c->mem[--c->sp] = _R1A;
    c->mem[--c->sp] = _R0A;
    return addr;
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
    c->r[R1A] = c->mem[c->sp++];
}

void cpu_pop_string(cpu *c)
{
    cpu_pop_common(c);
    for (size_t i = c->r[R1A]; i > 0; i--) {
        c->r[R0A] = c->mem[c->sp++];
    }
}

void cpu_pop_list(cpu *c)
{
    cpu_pop_common(c);
    for (size_t i = c->r[R1A]; i > 0; i--) {
        c->r[R0A] = c->mem[c->sp++];
        cpu_pop_dynamic(c);
    }
}

void cpu_pop_dict(cpu *c)
{
    cpu_pop_common(c);
    for (size_t i = c->r[R1A]; i > 0; i--) {
        c->r[R0A] = c->mem[c->sp++];
        cpu_pop_string(c);

        c->r[R0A] = c->mem[c->sp++];
        cpu_pop_dynamic(c);
    }
}

void cpu_dict_key_search(cpu *c, i64 dict_len, i64 key_len)
{
    char *key = build_string(c, key_len);
    for (size_t i = dict_len + 1; i > 0; i--) {
        c->r[R0A] = c->mem[c->sp++];
        cpu_pop_common(c);
        char *dict_key = build_string(c, c->r[R1A]);
        c->r[R0A] = c->mem[c->sp++];

        if (strcmp(dict_key, key) == 0) {
            free(key);
            free(dict_key);
            cpu_pop_common(c);
            return;
        }
        free(dict_key);

        cpu_pop_dynamic(c);
    }
    free(key);
}
