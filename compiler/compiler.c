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

unsigned short compileExpr(i64_array* program, Expr* expr)
{
    size_t len;
    Symbol* symbol;
    unsigned short type;
    i64 addr;
    switch (expr->kind) {
    case BasicLit_kind:
        switch (expr->v.basic_lit->value_type) {
        case V_BOOL:
            push_instr(program, LII);
            push_instr(program, R0A);
            push_instr(program, V_BOOL);

            push_instr(program, LII);
            push_instr(program, R1A);
            push_instr(program, expr->v.basic_lit->value.b ? 1 : 0);
            break;
        case V_INT:
            push_instr(program, LII);
            push_instr(program, R0A);
            push_instr(program, V_INT);

            push_instr(program, LII);
            push_instr(program, R1A);
            push_instr(program, expr->v.basic_lit->value.i);
            break;
        case V_FLOAT:
            push_instr(program, LII);
            push_instr(program, R0A);
            push_instr(program, V_FLOAT);

            i64 ipart;
            i64 frac;
            i64 leading_zeros = parse_f64(expr->v.basic_lit->value.f, &ipart, &frac);

            push_instr(program, LII);
            push_instr(program, R1A);
            push_instr(program, ipart);

            push_instr(program, LII);
            push_instr(program, R2A);
            push_instr(program, frac);

            push_instr(program, LII);
            push_instr(program, R3A);
            push_instr(program, leading_zeros);
            break;
        case V_STRING:
            len = strlen(expr->v.basic_lit->value.s);
            for (size_t i = len; i > 0; i--) {
                push_instr(program, LII);
                push_instr(program, R0A);
                push_instr(program, expr->v.basic_lit->value.s[i - 1] - '0');

                push_instr(program, PUSH);
                push_instr(program, R0A);
            }

            push_instr(program, LII);
            push_instr(program, R0A);
            push_instr(program, V_STRING);

            push_instr(program, LII);
            push_instr(program, R1A);
            push_instr(program, len);
            break;
        default:
            break;
        }
        return expr->v.basic_lit->value_type + 1;
        break;
    case Ident_kind:
        symbol = getSymbol(expr->v.ident->name);
        addr = symbol->addr;
        switch (symbol->value_type) {
        case V_BOOL:
            push_instr(program, LDI);
            push_instr(program, R0A);
            push_instr(program, addr++);

            push_instr(program, LDI);
            push_instr(program, R1A);
            push_instr(program, addr++);
            break;
        case V_INT:
            push_instr(program, LDI);
            push_instr(program, R0A);
            push_instr(program, addr++);

            push_instr(program, LDI);
            push_instr(program, R1A);
            push_instr(program, addr++);
            break;
        case V_FLOAT:
            push_instr(program, LDI);
            push_instr(program, R0A);
            push_instr(program, addr++);

            push_instr(program, LDI);
            push_instr(program, R1A);
            push_instr(program, addr++);

            push_instr(program, LDI);
            push_instr(program, R2A);
            push_instr(program, addr++);
            break;
        case V_STRING:
            push_instr(program, LDI);
            push_instr(program, R0A);
            push_instr(program, addr++);

            push_instr(program, LDI);
            push_instr(program, R1A);
            push_instr(program, addr++);

            len = strlen(symbol->value.s);
            addr += len - 1;
            for (size_t i = len; i > 0; i--) {
                push_instr(program, LDI);
                push_instr(program, R2A);
                push_instr(program, addr--);

                push_instr(program, PUSH);
                push_instr(program, R2A);
            }
            addr += len - 1;
            break;
        default:
            break;
        }
        return symbol->value_type + 1;
        break;
    case BinaryExpr_kind:
        type = compileExpr(program, expr->v.binary_expr->y);
        shift_registers(program, 4);
        compileExpr(program, expr->v.binary_expr->x);
        switch (expr->v.binary_expr->op) {
        case ADD_tok:
            push_instr(program, ADD);
            push_instr(program, R1A);
            push_instr(program, R1B);
            break;
        case SUB_tok:
            push_instr(program, SUB);
            push_instr(program, R1A);
            push_instr(program, R1B);
            break;
        case MUL_tok:
            push_instr(program, MUL);
            push_instr(program, R1A);
            push_instr(program, R1B);
            break;
        case QUO_tok:
            push_instr(program, DIV);
            push_instr(program, R1A);
            push_instr(program, R1B);
            break;
        case REM_tok:
            push_instr(program, MOD);
            push_instr(program, R1A);
            push_instr(program, R1B);
            break;
        case AND_tok:
            push_instr(program, BAND);
            push_instr(program, R1A);
            push_instr(program, R1B);
            break;
        case OR_tok:
            push_instr(program, BOR);
            push_instr(program, R1A);
            push_instr(program, R1B);
            break;
        case XOR_tok:
            push_instr(program, BXOR);
            push_instr(program, R1A);
            push_instr(program, R1B);
            break;
        case SHL_tok:
            push_instr(program, SHL);
            push_instr(program, R1A);
            push_instr(program, R1B);
            break;
        case SHR_tok:
            push_instr(program, SHR);
            push_instr(program, R1A);
            push_instr(program, R1B);
            break;
        case EQL_tok:
            push_instr(program, CMP);
            push_instr(program, R1A);
            push_instr(program, R1B);

            push_instr(program, LII);
            push_instr(program, R1A);
            push_instr(program, 1);

            push_instr(program, JEZ);
            push_instr(program, program->size + 7);

            push_instr(program, LII);
            push_instr(program, R1A);
            push_instr(program, 0);

            push_instr(program, LII);
            push_instr(program, R0A);
            push_instr(program, V_BOOL);
            break;
        case NEQ_tok:
            push_instr(program, CMP);
            push_instr(program, R1A);
            push_instr(program, R1B);

            push_instr(program, LII);
            push_instr(program, R1A);
            push_instr(program, 1);

            push_instr(program, JNZ);
            push_instr(program, program->size + 7);

            push_instr(program, LII);
            push_instr(program, R1A);
            push_instr(program, 0);

            push_instr(program, LII);
            push_instr(program, R0A);
            push_instr(program, V_BOOL);
            break;
        case GTR_tok:
            push_instr(program, CMP);
            push_instr(program, R1A);
            push_instr(program, R1B);

            push_instr(program, LII);
            push_instr(program, R1A);
            push_instr(program, 1);

            push_instr(program, JGZ);
            push_instr(program, program->size + 7);

            push_instr(program, LII);
            push_instr(program, R1A);
            push_instr(program, 0);

            push_instr(program, LII);
            push_instr(program, R0A);
            push_instr(program, V_BOOL);
            break;
        case LSS_tok:
            push_instr(program, CMP);
            push_instr(program, R1A);
            push_instr(program, R1B);

            push_instr(program, LII);
            push_instr(program, R1A);
            push_instr(program, 1);

            push_instr(program, JLZ);
            push_instr(program, program->size + 7);

            push_instr(program, LII);
            push_instr(program, R1A);
            push_instr(program, 0);

            push_instr(program, LII);
            push_instr(program, R0A);
            push_instr(program, V_BOOL);
            break;
        case GEQ_tok:
            push_instr(program, CMP);
            push_instr(program, R1A);
            push_instr(program, R1B);

            push_instr(program, LII);
            push_instr(program, R1A);
            push_instr(program, 1);

            push_instr(program, JEZ);
            push_instr(program, program->size + 9);
            push_instr(program, JGZ);
            push_instr(program, program->size + 7);

            push_instr(program, LII);
            push_instr(program, R1A);
            push_instr(program, 0);

            push_instr(program, LII);
            push_instr(program, R0A);
            push_instr(program, V_BOOL);
            break;
        case LEQ_tok:
            push_instr(program, CMP);
            push_instr(program, R1A);
            push_instr(program, R1B);

            push_instr(program, LII);
            push_instr(program, R1A);
            push_instr(program, 1);

            push_instr(program, JEZ);
            push_instr(program, program->size + 9);
            push_instr(program, JLZ);
            push_instr(program, program->size + 7);

            push_instr(program, LII);
            push_instr(program, R1A);
            push_instr(program, 0);

            push_instr(program, LII);
            push_instr(program, R0A);
            push_instr(program, V_BOOL);
            break;
        case LAND_tok:
            push_instr(program, LAND);
            push_instr(program, R1A);
            push_instr(program, R1B);
            break;
        case LOR_tok:
            push_instr(program, LOR);
            push_instr(program, R1A);
            push_instr(program, R1B);
            break;
        default:
            break;
        }
        return type;
        break;
    case UnaryExpr_kind:
        type = compileExpr(program, expr->v.unary_expr->x);
        switch (expr->v.unary_expr->op) {
        case ADD_tok:
            push_instr(program, LII);
            push_instr(program, R0B);
            push_instr(program, V_INT);

            push_instr(program, LII);
            push_instr(program, R1B);
            push_instr(program, 1);

            push_instr(program, MUL);
            push_instr(program, R1A);
            push_instr(program, R1B);
            break;
        case SUB_tok:
            push_instr(program, LII);
            push_instr(program, R0B);
            push_instr(program, V_INT);

            push_instr(program, LII);
            push_instr(program, R1B);
            push_instr(program, -1);

            push_instr(program, MUL);
            push_instr(program, R1A);
            push_instr(program, R1B);
            break;
        case NOT_tok:
            push_instr(program, LNOT);
            push_instr(program, R1A);
            break;
        case TILDE_tok:
            push_instr(program, BNOT);
            push_instr(program, R1A);
            break;
        default:
            break;
        }
        return type;
        break;
    default:
        break;
    }

    return 0;
}

void compileDecl(i64_array* program, Decl* decl)
{
    size_t len;
    Symbol* symbol;
    unsigned short type;
    switch (decl->kind) {
    case VarDecl_kind:
        type = compileExpr(program, decl->v.var_decl->expr);

        switch (type - 1) {
        case V_BOOL:
            symbol = addSymbolBool(
                decl->v.var_decl->ident->v.ident->name,
                decl->v.var_decl->expr->v.basic_lit->value.b
            );
            symbol->addr = program->heap;

            push_instr(program, STI);
            push_instr(program, program->heap++);
            push_instr(program, R0A);

            push_instr(program, STI);
            push_instr(program, program->heap++);
            push_instr(program, R1A);
            break;
        case V_INT:
            symbol = addSymbolInt(
                decl->v.var_decl->ident->v.ident->name,
                decl->v.var_decl->expr->v.basic_lit->value.i
            );
            symbol->addr = program->heap;

            push_instr(program, STI);
            push_instr(program, program->heap++);
            push_instr(program, R0A);

            push_instr(program, STI);
            push_instr(program, program->heap++);
            push_instr(program, R1A);
            break;
        case V_FLOAT:
            symbol = addSymbolFloat(
                decl->v.var_decl->ident->v.ident->name,
                decl->v.var_decl->expr->v.basic_lit->value.f
            );
            symbol->addr = program->heap;

            push_instr(program, STI);
            push_instr(program, program->heap++);
            push_instr(program, R0A);

            push_instr(program, STI);
            push_instr(program, program->heap++);
            push_instr(program, R1A);

            push_instr(program, STI);
            push_instr(program, program->heap++);
            push_instr(program, R2A);
            break;
        case V_STRING:
            symbol = addSymbolString(
                decl->v.var_decl->ident->v.ident->name,
                decl->v.var_decl->expr->v.basic_lit->value.s
            );
            symbol->addr = program->heap;

            push_instr(program, STI);
            push_instr(program, program->heap++);
            push_instr(program, R0A);

            push_instr(program, STI);
            push_instr(program, program->heap++);
            push_instr(program, R1A);

            len = strlen(decl->v.var_decl->expr->v.basic_lit->value.s);
            for (size_t i = len; i > 0; i--) {
                push_instr(program, POP);
                push_instr(program, R0A);

                push_instr(program, STI);
                push_instr(program, program->heap++);
                push_instr(program, R0A);
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
    program->arr[program->size++] = el;
}

i64 popProgram(i64_array* program)
{
    return program->arr[program->size--];
}

void freeProgram(i64_array* program)
{
    free(program->arr);
    initProgram(program);
}

i64_array* initProgram()
{
    i64_array* program = malloc(sizeof *program);
    program->capacity = USHRT_MAX * 32;
    program->arr = (i64*)malloc(program->capacity * sizeof(i64));
    program->size = 0;
    program->heap = USHRT_MAX * 2;
    return program;
}

void shift_registers(i64_array* program, size_t shift)
{
    size_t len = NUM_REGISTERS / 2;
    for (size_t i = 0; i < shift; i++) {
        push_instr(program, MOV);
        push_instr(program, i + len);
        push_instr(program, i);
    }
}
