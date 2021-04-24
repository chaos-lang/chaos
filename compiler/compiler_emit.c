/*
 * Description: Bytecode emitter module of the Chaos Programming Language's source
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

#include "compiler_emit.h"

void emit(i64_array* program)
{
    cpu *c = new_cpu(program->arr, program->size, program->heap, false);
    print_cpu(c);
    free_cpu(c);
}

void print_cpu(cpu *c)
{
	while (c->inst != HLT) {
		fetch(c);
		emitBytecode(c);
	}
    printf("%s\n", "HLT");
}

void emitBytecode(cpu *c)
{
    printf("[PC: %lld] ", c->pc);

	switch (c->inst) {
    case CLF:
        printf("%s\n", "CLF");
        break;
    case CMP:
        printf("%s %s %s\n", "CMP", getRegName(c->dest), getRegName(c->src));
        c->pc += 2;
        break;
    case CMPI:
        printf("%s %s %lld\n", "CMPI", getRegName(c->dest), c->src);
        c->pc += 2;
        break;
    case MOV:
        printf("%s %s %s\n", "MOV", getRegName(c->dest), getRegName(c->src));
        c->pc += 2;
        break;
    case STI:
        printf("%s %lld %s\n", "STI", c->dest, getRegName(c->src));
        c->pc += 2;
        break;
    case STR:
        printf("%s %s %s\n", "STR", getRegName(c->dest), getRegName(c->src));
        c->pc += 2;
        break;
    case LDI:
        printf("%s %s %lld\n", "LDI", getRegName(c->dest), c->src);
        c->pc += 2;
        break;
    case LDR:
        printf("%s %s %s\n", "LDR", getRegName(c->dest), getRegName(c->src));
        c->pc += 2;
        break;
    case LII:
        printf("%s %s %lld\n", "LII", getRegName(c->dest), c->src);
        c->pc += 2;
        break;
    case PUSH:
        printf("%s %s\n", "PUSH", getRegName(c->mem[++c->pc]));
        break;
    case POP:
        printf("%s %s\n", "POP", getRegName(c->mem[++c->pc]));
        break;
    case INC:
        printf("%s %s\n", "INC", getRegName(c->dest));
        c->pc++;
        break;
    case DEC:
        printf("%s %s\n", "DEC", getRegName(c->dest));
        c->pc++;
        break;
    case ADD:
        printf("%s %s %s\n", "ADD", getRegName(c->dest), getRegName(c->src));
        c->pc += 2;
        break;
    case SUB:
        printf("%s %s %s\n", "SUB", getRegName(c->dest), getRegName(c->src));
        c->pc += 2;
        break;
    case MUL:
        printf("%s %s %s\n", "MUL", getRegName(c->dest), getRegName(c->src));
        c->pc += 2;
        break;
    case DIV:
        printf("%s %s %s\n", "DIV", getRegName(c->dest), getRegName(c->src));
        c->pc += 2;
        break;
    case MOD:
        printf("%s %s %s\n", "MOD", getRegName(c->dest), getRegName(c->src));
        c->pc += 2;
        break;
    case JLZ:
        printf("%s %lld\n", "JLZ", c->mem[++c->pc]);
        break;
    case JGZ:
        printf("%s %lld\n", "JGZ", c->mem[++c->pc]);
        break;
    case JEZ:
        printf("%s %lld\n", "JEZ", c->mem[++c->pc]);
        break;
    case JNZ:
        printf("%s %lld\n", "JNZ", c->mem[++c->pc]);
        break;
    case JMP:
        printf("%s %lld\n", "JMP", c->mem[++c->pc]);
        break;
    case SHL:
        printf("%s %s %s\n", "SHL", getRegName(c->dest), getRegName(c->src));
        c->pc += 2;
        break;
    case SHR:
        printf("%s %s %s\n", "SHR", getRegName(c->dest), getRegName(c->src));
        c->pc += 2;
        break;
    case BAND:
        printf("%s %s %s\n", "BAND", getRegName(c->dest), getRegName(c->src));
        c->pc += 2;
        break;
    case BOR:
        printf("%s %s %s\n", "BOR", getRegName(c->dest), getRegName(c->src));
        c->pc += 2;
        break;
    case BNOT:
        printf("%s %s\n", "BNOT", getRegName(c->dest));
        c->pc++;
        break;
    case BXOR:
        printf("%s %s %s\n", "BXOR", getRegName(c->dest), getRegName(c->src));
        c->pc += 2;
        break;
    case LAND:
        printf("%s %s %s\n", "LAND", getRegName(c->dest), getRegName(c->src));
        c->pc += 2;
        break;
    case LOR:
        printf("%s %s %s\n", "LOR", getRegName(c->dest), getRegName(c->src));
        c->pc += 2;
        break;
    case LNOT:
        printf("%s %s\n", "LNOT", getRegName(c->dest));
        c->pc++;
        break;
    case DLDR:
        printf("%s %s\n", "DLDR", getRegName(c->dest));
        c->pc++;
        break;
    case DSTR:
        printf("%s %s\n", "DSTR", getRegName(c->dest));
        c->pc++;
        break;
    case DPOP:
        printf("%s\n", "DPOP");
        break;
    case DDEL:
        printf("%s %s %s\n", "DDEL", getRegName(c->dest), getRegName(c->src));
        c->pc += 2;
        break;
    case LIND:
        printf("%s %s %s\n", "LIND", getRegName(c->dest), getRegName(c->src));
        c->pc += 2;
        break;
    case KSRCH:
        printf("%s %s %s\n", "KSRCH", getRegName(c->dest), getRegName(c->src));
        c->pc += 2;
        break;
    case DEBUG:
        printf("%s\n", "DEBUG");
        break;
    case PRNT:
        printf("%s\n", "PRNT");
        break;
    case PPRNT:
        printf("%s\n", "PPRNT");
        break;
	}
}
