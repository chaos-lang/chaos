/*
 * Description: Compiler module of the Chaos Programming Language's source
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

#include "compiler.h"

i64_array* compile(ASTRoot* ast_root)
{
    i64_array* program = initProgram();

    for (unsigned long i = 0; i < ast_root->file_count; i++) {
        compileStmtList(program, ast_root->files[i]->stmt_list);
    }

    push_instr(program, HLT);
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
    switch (stmt->kind) {
    case PrintStmt_kind:
        compileExpr(program, stmt->v.print_stmt->x);
        push_instr(program, PRNT);
        break;
    case DeclStmt_kind:
        compileDecl(program, stmt->v.decl_stmt->decl);
        break;
    default:
        break;
    }
}

void compileExpr(i64_array* program, Expr* expr)
{
    size_t len;
    Symbol* symbol;
    i64 addr;
    switch (expr->kind) {
    case BasicLit_kind:
        switch (expr->v.basic_lit->value_type) {
        case V_BOOL:
            push_instr(program, LII);
            push_instr(program, R0);
            push_instr(program, V_BOOL);

            push_instr(program, LII);
            push_instr(program, R1);
            push_instr(program, expr->v.basic_lit->value.b ? 1 : 0);
            break;
        case V_INT:
            push_instr(program, LII);
            push_instr(program, R0);
            push_instr(program, V_INT);

            push_instr(program, LII);
            push_instr(program, R1);
            push_instr(program, expr->v.basic_lit->value.i);
            break;
        case V_FLOAT:
            push_instr(program, LII);
            push_instr(program, R0);
            push_instr(program, V_FLOAT);

            i64 ipart;
            i64 frac;
            char *buf = NULL;
            buf = snprintf_concat_float(buf, "%Lf", expr->v.basic_lit->value.f);
            sscanf(buf, "%lld.%lld", &ipart, &frac);

            push_instr(program, LII);
            push_instr(program, R1);
            push_instr(program, ipart);

            push_instr(program, LII);
            push_instr(program, R2);
            push_instr(program, frac);
            break;
        case V_STRING:
            len = strlen(expr->v.basic_lit->value.s);
            for (size_t i = len; i > 0; i--) {
                push_instr(program, LII);
                push_instr(program, R0);
                push_instr(program, expr->v.basic_lit->value.s[i - 1] - '0');

                push_instr(program, PUSH);
                push_instr(program, R0);
            }

            push_instr(program, LII);
            push_instr(program, R0);
            push_instr(program, V_STRING);

            push_instr(program, LII);
            push_instr(program, R1);
            push_instr(program, len);
            break;
        default:
            break;
        }
        break;
    case Ident_kind:
        symbol = getSymbol(expr->v.ident->name);
        addr = symbol->addr;
        switch (symbol->value_type) {
        case V_BOOL:
            push_instr(program, LDI);
            push_instr(program, R0);
            push_instr(program, addr++);

            push_instr(program, LDI);
            push_instr(program, R1);
            push_instr(program, addr++);
            break;
        case V_INT:
            push_instr(program, LDI);
            push_instr(program, R0);
            push_instr(program, addr++);

            push_instr(program, LDI);
            push_instr(program, R1);
            push_instr(program, addr++);
            break;
        case V_FLOAT:
            push_instr(program, LDI);
            push_instr(program, R0);
            push_instr(program, addr++);

            push_instr(program, LDI);
            push_instr(program, R1);
            push_instr(program, addr++);

            push_instr(program, LDI);
            push_instr(program, R2);
            push_instr(program, addr++);
            break;
        case V_STRING:
            push_instr(program, LDI);
            push_instr(program, R0);
            push_instr(program, addr++);

            push_instr(program, LDI);
            push_instr(program, R1);
            push_instr(program, addr++);

            len = strlen(symbol->value.s);
            addr += len - 1;
            for (size_t i = len; i > 0; i--) {
                push_instr(program, LDI);
                push_instr(program, R2);
                push_instr(program, addr--);

                push_instr(program, PUSH);
                push_instr(program, R2);
            }
            addr += len - 1;
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }
}

void compileDecl(i64_array* program, Decl* decl)
{
    size_t len;
    Symbol* symbol;
    switch (decl->kind) {
    case VarDecl_kind:
        compileExpr(program, decl->v.var_decl->expr);

        switch (decl->v.var_decl->expr->v.basic_lit->value_type) {
        case V_BOOL:
            symbol = addSymbolBool(
                decl->v.var_decl->ident->v.ident->name,
                decl->v.var_decl->expr->v.basic_lit->value.b
            );
            symbol->addr = program->heap;

            push_instr(program, STI);
            push_instr(program, program->heap++);
            push_instr(program, R0);

            push_instr(program, STI);
            push_instr(program, program->heap++);
            push_instr(program, R1);
            break;
        case V_INT:
            symbol = addSymbolInt(
                decl->v.var_decl->ident->v.ident->name,
                decl->v.var_decl->expr->v.basic_lit->value.i
            );
            symbol->addr = program->heap;

            push_instr(program, STI);
            push_instr(program, program->heap++);
            push_instr(program, R0);

            push_instr(program, STI);
            push_instr(program, program->heap++);
            push_instr(program, R1);
            break;
        case V_FLOAT:
            symbol = addSymbolFloat(
                decl->v.var_decl->ident->v.ident->name,
                decl->v.var_decl->expr->v.basic_lit->value.f
            );
            symbol->addr = program->heap;

            push_instr(program, STI);
            push_instr(program, program->heap++);
            push_instr(program, R0);

            push_instr(program, STI);
            push_instr(program, program->heap++);
            push_instr(program, R1);

            push_instr(program, STI);
            push_instr(program, program->heap++);
            push_instr(program, R2);
            break;
        case V_STRING:
            symbol = addSymbolString(
                decl->v.var_decl->ident->v.ident->name,
                decl->v.var_decl->expr->v.basic_lit->value.s
            );
            symbol->addr = program->heap;

            push_instr(program, STI);
            push_instr(program, program->heap++);
            push_instr(program, R0);

            push_instr(program, STI);
            push_instr(program, program->heap++);
            push_instr(program, R1);

            len = strlen(decl->v.var_decl->expr->v.basic_lit->value.s);
            for (size_t i = len; i > 0; i--) {
                push_instr(program, POP);
                push_instr(program, R0);

                push_instr(program, STI);
                push_instr(program, program->heap++);
                push_instr(program, R0);
            }
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }
}

void push_instr(i64_array* program, i64 el)
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
    program->heap = USHRT_MAX * 32;
    return program;
}
