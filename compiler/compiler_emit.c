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
        "Program Counter",
        "Bytecode",
        "Line",
        "File",
        "Line Number"
    );
    for (i64 i = 0; i < hlt_count; i++) {
        do {
            fetch(c);
            emitBytecode(c);
        } while (c->inst != HLT);
    }
    printf("\n");
}

void emitBytecode(cpu *c)
{
    char str_pc[40];
    char str_inst[40];

    sprintf(str_pc, "%lld", c->ic);

	switch (c->inst->op_code) {
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
