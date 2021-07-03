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

void emit(KaosIR* program)
{
    cpu *c = new_cpu(program, 0);
    print_cpu(c, program->hlt_count);
    free_cpu(c);
}

void print_cpu(cpu *c, i64 hlt_count)
{
    printf(
        "%-40s %-40s %-40s %-80s %-40s\n",
        "Counter",
        "Instructions",
        "Line",
        "File",
        "Line Number"
    );
    for (i64 i = 0; i < hlt_count; i++) {
        do {
            fetch(c);
            emitBytecode(c);
        } while (c->inst->op_code != HLT);
    }
    printf("\n");
}

void emitBytecode(cpu *c)
{
    char str_pc[40];
    char str_inst[50];

    sprintf(str_pc, "%lld", c->ic);

	switch (c->inst->op_code) {
    case PROLOG:
        sprintf(str_inst, "%s", "PROLOG");
        break;
    // >>> Transfer Operations <<<
    // mov
    case MOVR:
        sprintf(str_inst, "%s R(%d) R(%d)", "MOVR", c->inst->op1->reg, c->inst->op2->reg);
        break;
    case MOVI:
        sprintf(str_inst, "%s R(%d) %lld", "MOVI", c->inst->op1->reg, c->inst->op2->value.i);
        break;
    // fmov
    case FMOV:
        sprintf(str_inst, "%s FR(%d) %lf", "FMOV", c->inst->op1->reg, c->inst->op2->value.f);
        break;
    // alloc
    case ALLOCAI:
        sprintf(str_inst, "%s addr: %lld space: %lld", "ALLOCAI", c->inst->op1->value.i, c->inst->op2->value.i);
        break;
    case REF_ALLOCAI:
        sprintf(str_inst, "%s R(%d) addr: %lld", "REF_ALLOCAI", c->inst->op1->reg, c->inst->op2->value.i);
        break;
    // >>> Load Operations <<<
    // ldr
    case LDR:
        sprintf(str_inst, "%s R(%d) R(%d) size: %lld", "LDR", c->inst->op1->reg, c->inst->op2->reg, c->inst->op3->value.i);
        break;
    case LDXR:
        sprintf(str_inst, "%s R(%d) R(%d) R(%d) size: %lld", "LDXR", c->inst->op1->reg, c->inst->op2->reg, c->inst->op3->reg, c->inst->op4->value.i);
        break;
    // >>> Store Operations <<<
    // str
    case STR:
        sprintf(str_inst, "%s R(%d) R(%d) size: %lld", "STR", c->inst->op1->reg, c->inst->op2->reg, c->inst->op3->value.i);
        break;
    case STXR:
        sprintf(str_inst, "%s R(%d) R(%d) R(%d) size: %lld", "STXR", c->inst->op1->reg, c->inst->op2->reg, c->inst->op3->reg, c->inst->op4->value.i);
        break;
    // fstr
    case FSTR:
        sprintf(str_inst, "%s R(%d) FR(%d) size: %lld", "FSTR", c->inst->op1->reg, c->inst->op2->reg, c->inst->op3->value.i);
        break;
    case FSTXR:
        sprintf(str_inst, "%s R(%d) R(%d) FR(%d) size: %lld", "FSTXR", c->inst->op1->reg, c->inst->op2->reg, c->inst->op3->reg, c->inst->op4->value.i);
        break;
    // fldr
    case FLDR:
        sprintf(str_inst, "%s FR(%d) R(%d) size: %lld", "FLDR", c->inst->op1->reg, c->inst->op2->reg, c->inst->op3->value.i);
        break;
    case FLDXR:
        sprintf(str_inst, "%s FR(%d) R(%d) R(%d) size: %lld", "FLDXR", c->inst->op1->reg, c->inst->op2->reg, c->inst->op3->reg, c->inst->op4->value.i);
        break;
    // >>> Binary Arithmetic Operations <<<
    // add
    case ADDR:
        sprintf(str_inst, "%s R(%d) R(%d) R(%d)", "ADDR", c->inst->op1->reg, c->inst->op2->reg, c->inst->op3->reg);
        break;
    case ADDI:
        sprintf(str_inst, "%s R(%d) R(%d) %lld", "ADDI", c->inst->op1->reg, c->inst->op2->reg, c->inst->op3->value.i);
        break;
    // sub
    case SUBR:
        sprintf(str_inst, "%s R(%d) R(%d) R(%d)", "SUBR", c->inst->op1->reg, c->inst->op2->reg, c->inst->op3->reg);
        break;
    case SUBI:
        sprintf(str_inst, "%s R(%d) R(%d) %lld", "SUBI", c->inst->op1->reg, c->inst->op2->reg, c->inst->op3->value.i);
        break;
    // mul
    case MULR:
        sprintf(str_inst, "%s R(%d) R(%d) R(%d)", "MULR", c->inst->op1->reg, c->inst->op2->reg, c->inst->op3->reg);
        break;
    case MULI:
        sprintf(str_inst, "%s R(%d) R(%d) imm: %lld", "MULI", c->inst->op1->reg, c->inst->op2->reg, c->inst->op3->value.i);
        break;
    // div
    case DIVR:
        sprintf(str_inst, "%s R(%d) R(%d) R(%d)", "DIVR", c->inst->op1->reg, c->inst->op2->reg, c->inst->op3->reg);
        break;
    case DIVI:
        sprintf(str_inst, "%s R(%d) R(%d) %lld", "DIVI", c->inst->op1->reg, c->inst->op2->reg, c->inst->op3->value.i);
        break;
    // mod
    case MODR:
        sprintf(str_inst, "%s R(%d) R(%d) R(%d)", "MODR", c->inst->op1->reg, c->inst->op2->reg, c->inst->op3->reg);
        break;
    case MODI:
        sprintf(str_inst, "%s R(%d) R(%d) %lld", "MODI", c->inst->op1->reg, c->inst->op2->reg, c->inst->op3->value.i);
        break;
	// Binary Logic
	// and
    case ANDR:
        sprintf(str_inst, "%s R(%d) R(%d) R(%d)", "ANDR", c->inst->op1->reg, c->inst->op2->reg, c->inst->op3->reg);
        break;
    case ANDI:
        sprintf(str_inst, "%s R(%d) R(%d) %lld", "ANDI", c->inst->op1->reg, c->inst->op2->reg, c->inst->op3->value.i);
        break;
	// or
    case ORR:
        sprintf(str_inst, "%s R(%d) R(%d) R(%d)", "ORR", c->inst->op1->reg, c->inst->op2->reg, c->inst->op3->reg);
        break;
    case ORI:
        sprintf(str_inst, "%s R(%d) R(%d) %lld", "ORI", c->inst->op1->reg, c->inst->op2->reg, c->inst->op3->value.i);
        break;
	// xor
    case XORR:
        sprintf(str_inst, "%s R(%d) R(%d) R(%d)", "XORR", c->inst->op1->reg, c->inst->op2->reg, c->inst->op3->reg);
        break;
    case XORI:
        sprintf(str_inst, "%s R(%d) R(%d) %lld", "XORI", c->inst->op1->reg, c->inst->op2->reg, c->inst->op3->value.i);
        break;
	// Binary Shift
	// lsh
    case LSHR:
        sprintf(str_inst, "%s R(%d) R(%d) R(%d)", "LSHR", c->inst->op1->reg, c->inst->op2->reg, c->inst->op3->reg);
        break;
    case LSHI:
        sprintf(str_inst, "%s R(%d) R(%d) %lld", "LSHI", c->inst->op1->reg, c->inst->op2->reg, c->inst->op3->value.i);
        break;
	// rsh
    case RSHR:
        sprintf(str_inst, "%s R(%d) R(%d) R(%d)", "RSHR", c->inst->op1->reg, c->inst->op2->reg, c->inst->op3->reg);
        break;
    case RSHI:
        sprintf(str_inst, "%s R(%d) R(%d) %lld", "RSHI", c->inst->op1->reg, c->inst->op2->reg, c->inst->op3->value.i);
        break;
    // >>> Non-Atomic Instructions <<<
	case DYN_ADD:
        sprintf(str_inst, "%s", "DYN_ADD");
        break;
	case DYN_SUB:
        sprintf(str_inst, "%s", "DYN_SUB");
        break;
	case DYN_MUL:
        sprintf(str_inst, "%s", "DYN_MUL");
        break;
	case DYN_DIV:
        sprintf(str_inst, "%s", "DYN_DIV");
        break;
	case DYN_MOD:
        sprintf(str_inst, "%s", "DYN_MOD");
        break;
    case PRNT:
        sprintf(str_inst, "%s", "PRNT");
        break;
    case DEBUG:
        sprintf(str_inst, "%s", "DEBUG");
        break;
    case HLT:
        sprintf(str_inst, "%s", "HLT");
        break;
    default:
        break;
	}

    FILE* fp_module = NULL;
    AST* current_ast = c->inst->ast;
    char* line = NULL;
    char* module_path = NULL;
    int lineno = 0;
    if (current_ast != NULL) {
        if (current_ast->file->is_interactive) {
            fseek(tmp_stdin, 0, SEEK_SET);
            fp_module = tmp_stdin;
        } else {
            fp_module = fopen(current_ast->file->module_path, "r");
        }
        line = get_nth_line(fp_module, current_ast->lineno);
        if (line != NULL) {
            line[strlen(line) - 1] = '\0';
            module_path = current_ast->file->module_path;
            lineno = current_ast->lineno;
        }
    }

    printf(
        "%-40s %-40s %-40s %-80s %-40d\n",
        str_pc,
        str_inst,
        line,
        module_path,
        lineno
    );
    if (fp_module != tmp_stdin)
        fclose(fp_module);
}
