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
    cpu *c = new_cpu(program->arr, program->heap, 0, program->ast_ref->arr, 0);
    print_cpu(c, program->hlt_count);
    free_cpu(c);
}

void print_cpu(cpu *c, i64 hlt_count)
{
    printf(
        "%-40s %-40s %-40s %-80s %-40s\n",
        "Program Counter",
        "Bytecode",
        "Line",
        "File",
        "Line Number"
    );
    for (i64 i = 0; i < hlt_count; i++) {
        do {
            // fetch(c);
            emitBytecode(c);
        } while (c->inst != HLT);
    }
    printf("\n");
}

void emitBytecode(cpu *c)
{
    char str_pc[40];
    char str_inst[40];

    sprintf(str_pc, "%lld", c->pc);

	switch (c->inst) {
    case CLF:
        sprintf(str_inst, "%s", "CLF");
        break;
    case CMP:
        sprintf(str_inst, "%s %s %s", "CMP", getRegName(c->dest), getRegName(c->src));
        c->pc += 2;
        break;
    case CMPI:
        sprintf(str_inst, "%s %s %lld", "CMPI", getRegName(c->dest), c->src);
        c->pc += 2;
        break;
    case MOV:
        sprintf(str_inst, "%s %s %s", "MOV", getRegName(c->dest), getRegName(c->src));
        c->pc += 2;
        break;
    case STI:
        sprintf(str_inst, "%s %lld %s", "STI", c->dest, getRegName(c->src));
        c->pc += 2;
        break;
    case STR:
        sprintf(str_inst, "%s %s %s", "STR", getRegName(c->dest), getRegName(c->src));
        c->pc += 2;
        break;
    case LDI:
        sprintf(str_inst, "%s %s %lld", "LDI", getRegName(c->dest), c->src);
        c->pc += 2;
        break;
    case LDR:
        sprintf(str_inst, "%s %s %s", "LDR", getRegName(c->dest), getRegName(c->src));
        c->pc += 2;
        break;
    case LII:
        sprintf(str_inst, "%s %s %lld", "LII", getRegName(c->dest), c->src);
        c->pc += 2;
        break;
    case PUSH:
        sprintf(str_inst, "%s %s", "PUSH", getRegName(c->program[++c->pc]));
        break;
    case POP:
        sprintf(str_inst, "%s %s", "POP", getRegName(c->program[++c->pc]));
        break;
    case INC:
        sprintf(str_inst, "%s %s", "INC", getRegName(c->dest));
        c->pc++;
        break;
    case DEC:
        sprintf(str_inst, "%s %s", "DEC", getRegName(c->dest));
        c->pc++;
        break;
    case ADD:
        sprintf(str_inst, "%s %s %s", "ADD", getRegName(c->dest), getRegName(c->src));
        c->pc += 2;
        break;
    case SUB:
        sprintf(str_inst, "%s %s %s", "SUB", getRegName(c->dest), getRegName(c->src));
        c->pc += 2;
        break;
    case MUL:
        sprintf(str_inst, "%s %s %s", "MUL", getRegName(c->dest), getRegName(c->src));
        c->pc += 2;
        break;
    case DIV:
        sprintf(str_inst, "%s %s %s", "DIV", getRegName(c->dest), getRegName(c->src));
        c->pc += 2;
        break;
    case MOD:
        sprintf(str_inst, "%s %s %s", "MOD", getRegName(c->dest), getRegName(c->src));
        c->pc += 2;
        break;
    case JLZ:
        sprintf(str_inst, "%s %lld", "JLZ", c->program[++c->pc]);
        break;
    case JGZ:
        sprintf(str_inst, "%s %lld", "JGZ", c->program[++c->pc]);
        break;
    case JEZ:
        sprintf(str_inst, "%s %lld", "JEZ", c->program[++c->pc]);
        break;
    case JNZ:
        sprintf(str_inst, "%s %lld", "JNZ", c->program[++c->pc]);
        break;
    case JMP:
        sprintf(str_inst, "%s %lld", "JMP", c->program[++c->pc]);
        break;
    case SHL:
        sprintf(str_inst, "%s %s %s", "SHL", getRegName(c->dest), getRegName(c->src));
        c->pc += 2;
        break;
    case SHR:
        sprintf(str_inst, "%s %s %s", "SHR", getRegName(c->dest), getRegName(c->src));
        c->pc += 2;
        break;
    case BAND:
        sprintf(str_inst, "%s %s %s", "BAND", getRegName(c->dest), getRegName(c->src));
        c->pc += 2;
        break;
    case BOR:
        sprintf(str_inst, "%s %s %s", "BOR", getRegName(c->dest), getRegName(c->src));
        c->pc += 2;
        break;
    case BNOT:
        sprintf(str_inst, "%s %s", "BNOT", getRegName(c->dest));
        c->pc++;
        break;
    case BXOR:
        sprintf(str_inst, "%s %s %s", "BXOR", getRegName(c->dest), getRegName(c->src));
        c->pc += 2;
        break;
    case LAND:
        sprintf(str_inst, "%s %s %s", "LAND", getRegName(c->dest), getRegName(c->src));
        c->pc += 2;
        break;
    case LOR:
        sprintf(str_inst, "%s %s %s", "LOR", getRegName(c->dest), getRegName(c->src));
        c->pc += 2;
        break;
    case LNOT:
        sprintf(str_inst, "%s %s", "LNOT", getRegName(c->dest));
        c->pc++;
        break;
    case DLDR:
        sprintf(str_inst, "%s %s", "DLDR", getRegName(c->dest));
        c->pc++;
        break;
    case DSTR:
        sprintf(str_inst, "%s %s", "DSTR", getRegName(c->dest));
        c->pc++;
        break;
    case DPOP:
        sprintf(str_inst, "%s", "DPOP");
        break;
    case DDEL:
        sprintf(str_inst, "%s %s %s", "DDEL", getRegName(c->dest), getRegName(c->src));
        c->pc += 2;
        break;
    case LIND:
        sprintf(str_inst, "%s %s %s", "LIND", getRegName(c->dest), getRegName(c->src));
        c->pc += 2;
        break;
    case KSRCH:
        sprintf(str_inst, "%s %s %s", "KSRCH", getRegName(c->dest), getRegName(c->src));
        c->pc += 2;
        break;
    case JMPB:
        sprintf(str_inst, "%s", "JMPB");
        break;
    case SJMPB:
        sprintf(str_inst, "%s %lld", "SJMPB", c->dest);
        c->pc++;
        break;
    case BRK:
        sprintf(str_inst, "%s", "BRK");
        break;
    case SBRK:
        sprintf(str_inst, "%s %lld", "SBRK", c->dest);
        c->pc++;
        break;
    case CONT:
        sprintf(str_inst, "%s", "CONT");
        break;
    case SCONT:
        sprintf(str_inst, "%s %lld", "SCONT", c->dest);
        c->pc++;
        break;
    case CALL:
        sprintf(str_inst, "%s", "CALL");
        break;
    case CALLX:
        sprintf(str_inst, "%s", "CALLX");
        break;
    case CALLEXT:
        sprintf(str_inst, "%s %lld", "CALLEXT", c->dest);
        c->pc++;
        break;
    case DEBUG:
        sprintf(str_inst, "%s", "DEBUG");
        break;
    case PRNT:
        sprintf(str_inst, "%s", "PRNT");
        break;
    case PPRNT:
        sprintf(str_inst, "%s", "PPRNT");
        break;
    case EXIT:
        sprintf(str_inst, "%s", "EXIT");
        break;
    case THRW:
        sprintf(str_inst, "%s %lld %lld", "THRW", c->dest, c->src);
        c->pc++;
        break;
    case HLT:
        sprintf(str_inst, "%s", "HLT");
	}

    FILE* fp_module = NULL;
    AST* current_ast = (void *)ast_stack[ast_stack_p];
    if (current_ast->file->is_interactive) {
        fseek(tmp_stdin, 0, SEEK_SET);
        fp_module = tmp_stdin;
    } else {
        fp_module = fopen(current_ast->file->module_path, "r");
    }
    char* line = get_nth_line(fp_module, current_ast->lineno);
    line[strlen(line) - 1] = '\0';
    printf(
        "%-40s %-40s %-40s %-80s %-40d\n",
        str_pc,
        str_inst,
        line,
        current_ast->file->module_path,
        current_ast->lineno
    );
    if (fp_module != tmp_stdin)
        fclose(fp_module);
}
