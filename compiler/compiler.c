/*
 * Description: Compiler module of the Chaos Programming Language's source
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

#include "compiler.h"

i64_array* compile(ASTRoot* ast_root)
{
    i64_array* program = initProgram();

    for (unsigned long i = 0; i < ast_root->file_count; i++) {
        compileStmtList(program, ast_root->files[i]->stmt_list);
    }

    pushProgram(program, HLT);

    return program;
}

void compileStmtList(i64_array* program, StmtList* stmt_list)
{
    for (unsigned long i = stmt_list->stmt_count; 0 < i; i--) {
        compileStmt(program, stmt_list->stmts[i - 1]);
    }
}

void compileStmt(i64_array* program, Stmt* stmt)
{
    switch (stmt->kind)
    {
        case PrintStmt_kind:
            compileExpr(program, stmt->v.print_stmt->x);
            pushProgram(program, PRNT);
            break;
        default:
            break;
    }
}

void compileExpr(i64_array* program, Expr* expr)
{
    size_t len;

    switch (expr->kind)
    {
        case BasicLit_kind:
            switch (expr->v.basic_lit->value_type)
            {
                case V_BOOL:
                    pushProgram(program, LII);
                    pushProgram(program, R0);
                    pushProgram(program, V_BOOL);
                    pushProgram(program, LII);
                    pushProgram(program, R1);
                    pushProgram(program, expr->v.basic_lit->value.b ? 1 : 0);
                    break;
                case V_INT:
                    pushProgram(program, LII);
                    pushProgram(program, R0);
                    pushProgram(program, V_INT);
                    pushProgram(program, LII);
                    pushProgram(program, R1);
                    pushProgram(program, expr->v.basic_lit->value.i);
                    break;
                case V_FLOAT:
                    pushProgram(program, LII);
                    pushProgram(program, R0);
                    pushProgram(program, V_FLOAT);
                    i64 ipart;
                    i64 frac;
                    char *buf = NULL;
                    buf = snprintf_concat_float(buf, "%Lf", expr->v.basic_lit->value.f);
                    sscanf(buf, "%lld.%lld", &ipart, &frac);
                    pushProgram(program, LII);
                    pushProgram(program, R1);
                    pushProgram(program, ipart);
                    pushProgram(program, LII);
                    pushProgram(program, R2);
                    pushProgram(program, frac);
                    break;
                case V_STRING:
                    len = strlen(expr->v.basic_lit->value.s);
                    for (size_t i = len; i > 0; i--) {
                        pushProgram(program, LII);
                        pushProgram(program, R0);
                        pushProgram(program, expr->v.basic_lit->value.s[i - 1] - '0');
                        pushProgram(program, PUSH);
                        pushProgram(program, R0);
                    }
                    pushProgram(program, LII);
                    pushProgram(program, R0);
                    pushProgram(program, V_STRING);
                    pushProgram(program, LII);
                    pushProgram(program, R1);
                    pushProgram(program, len);
                    break;
                default:
                    break;
            }
            break;
        default:
            break;
    }
}

void pushProgram(i64_array* program, i64 el)
{
    if (program->capacity == 0)
        program->arr = (i64*)malloc(++(program->capacity) * sizeof(i64*));
    else
        program->arr = (i64*)realloc(program->arr, ++(program->capacity) * sizeof(i64*));

    program->arr[program->size] = el;
    program->size++;
}

i64 popProgram(i64_array* program)
{
    return program->arr[program->size--];
}

void expandStack(i64_array* program, i64 stack)
{
    if (program->capacity == 0)
        program->arr = (i64*)malloc((program->capacity += stack) * sizeof(i64*));
    else
        program->arr = (i64*)realloc(program->arr, (program->capacity += stack) * sizeof(i64*));
}

void freeProgram(i64_array* program)
{
    free(program->arr);
    initProgram(program);
}

i64_array* initProgram()
{
    i64_array* program = (i64_array*)malloc(1 * sizeof(i64_array*));
    program->capacity = 0;
    program->size = 0;
    return program;
}
