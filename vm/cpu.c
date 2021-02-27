/*
 * Description: CPU module of the Chaos Programming Language's source
 *
 * Copyright (c) 2019-2020 Chaos Language Development Authority <info@chaos-lang.org>
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

cpu *new_cpu(i64 *memory, i64 mem_size)
{
	cpu *c = malloc(sizeof(cpu));
	c->mem = memory;
	c->sp = mem_size - 1;
	c->max_mem = mem_size;
	c->pc = -1;
	c->inst = 0;
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
    size_t len;

	switch (c->inst) {
		case CLF:
			clear_flags(c);
			break;
		case CMP:
			set_flags(c, c->r[c->dest], c->r[c->src]);
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
		case LDI:
			c->r[c->dest] = c->mem[c->src];
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
			c->r[c->dest] += c->r[c->src];
			c->pc += 2;
			break;
		case SUB:
			c->r[c->dest] -= c->r[c->src];
			c->pc += 2;
			break;
		case MUL:
			c->r[c->dest] *= c->r[c->src];
			c->pc += 2;
			break;
		case DIV:
			c->r[c->dest] /= c->r[c->src];
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
        case PRNT:
            switch (c->r[R0])
            {
                case V_BOOL:
                    printf("%s\n", c->r[R1] ? "true" : "false");
                    break;
                case V_INT:
                    printf("%lld\n", c->r[R1]);
                    break;
                case V_FLOAT:
                    printf("%lld.%lld\n", c->r[R1], c->r[R2]);
                    break;
                case V_STRING:
                    len = c->r[R1];
                    for (size_t i = 0; i < len; i++) {
                        c->r[R1] = c->mem[c->sp++];
                        printf("%c", (int)c->r[R1] + '0');
                    }
                    printf("\n");
                    break;
            }
			break;
	}
}
