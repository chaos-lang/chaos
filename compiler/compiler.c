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
    case EchoStmt_kind:
        compileExpr(program, stmt->v.echo_stmt->x);
        push_instr(program, PRNT);
        break;
    case PrintStmt_kind:
        compileExpr(program, stmt->v.print_stmt->x);
        push_instr(program, PRNT);

        push_instr(program, LII);
        push_instr(program, R0A);
        push_instr(program, '\n' - '0');

        push_instr(program, PUSH);
        push_instr(program, R0A);

        push_instr(program, LII);
        push_instr(program, R0A);
        push_instr(program, V_STRING);

        push_instr(program, LII);
        push_instr(program, R1A);
        push_instr(program, 1);

        push_instr(program, PRNT);
        break;
    case DeclStmt_kind:
        compileDecl(program, stmt->v.decl_stmt->decl);
        break;
    case AssignStmt_kind: {
        compileExpr(program, stmt->v.assign_stmt->x);
        shift_registers(program, 8);
        enum ValueType value_type = compileExpr(program, stmt->v.assign_stmt->y) - 1;
        switch (stmt->v.assign_stmt->x->kind) {
        case Ident_kind: {
            Symbol* symbol_x = getSymbol(stmt->v.assign_stmt->x->v.ident->name);
            Symbol* symbol_y = NULL;
            if (stmt->v.assign_stmt->y->kind == Ident_kind)
                symbol_y = getSymbol(stmt->v.assign_stmt->y->v.ident->name);
            i64 addr = symbol_x->addr;
            if (symbol_x->type == K_ANY)
                symbol_x->value_type = value_type;
            else if (symbol_x->type == K_NUMBER && symbol_y != NULL && (
                symbol_y->value_type == V_INT || symbol_y->value_type == V_FLOAT
            ))
                symbol_x->value_type = symbol_y->value_type;
            switch (symbol_x->value_type) {
            case V_BOOL:
                push_instr(program, LII);
                push_instr(program, R0A);
                push_instr(program, V_BOOL);

                push_instr(program, STI);
                push_instr(program, addr++);
                push_instr(program, R0A);

                push_instr(program, STI);
                push_instr(program, addr++);
                push_instr(program, R1A);
                break;
            case V_INT:
                push_instr(program, LII);
                push_instr(program, R0A);
                push_instr(program, V_INT);

                push_instr(program, STI);
                push_instr(program, addr++);
                push_instr(program, R0A);

                push_instr(program, STI);
                push_instr(program, addr++);
                push_instr(program, R1A);
                break;
            case V_FLOAT:
                push_instr(program, LII);
                push_instr(program, R0A);
                push_instr(program, V_FLOAT);

                push_instr(program, STI);
                push_instr(program, addr++);
                push_instr(program, R0A);

                push_instr(program, STI);
                push_instr(program, addr++);
                push_instr(program, R1A);
                break;
            case V_STRING: {
                push_instr(program, LII);
                push_instr(program, R0A);
                push_instr(program, V_STRING);

                push_instr(program, STI);
                push_instr(program, addr++);
                push_instr(program, R0A);

                push_instr(program, STI);
                push_instr(program, addr++);
                push_instr(program, R1A);

                size_t len = symbol_x->len;
                for (size_t i = 0; i < len; i++) {
                    push_instr(program, POP);
                    push_instr(program, R2A);

                    push_instr(program, STI);
                    push_instr(program, addr++);
                    push_instr(program, R2A);
                }
                break;
            }
            default:
                break;
            }
            break;
        }
        case IndexExpr_kind: {
            Symbol* symbol = getSymbol(stmt->v.assign_stmt->x->v.index_expr->x->v.ident->name);
            i64 addr = symbol->addr;
            push_instr(program, LII);
            push_instr(program, R3B);
            push_instr(program, addr);

            push_instr(program, INC);
            push_instr(program, R3B);

            push_instr(program, INC);
            push_instr(program, R3B);

            push_instr(program, ADD);
            push_instr(program, R3B);
            push_instr(program, R4B);

            push_instr(program, POP);
            push_instr(program, R1A);

            push_instr(program, STR);
            push_instr(program, R3B);
            push_instr(program, R1A);
            break;
        }
        default:
            break;
        }
        break;
    }
    case DelStmt_kind: {
        switch (stmt->v.del_stmt->ident->kind) {
        case Ident_kind: {
            Symbol* symbol = getSymbol(stmt->v.assign_stmt->x->v.ident->name);
            removeSymbol(symbol);
            break;
        }
        case IndexExpr_kind: {
            Symbol* symbol = getSymbol(stmt->v.assign_stmt->x->v.index_expr->x->v.ident->name);
            i64 addr = symbol->addr;
            push_instr(program, LII);
            push_instr(program, R3B);
            push_instr(program, addr);

            push_instr(program, INC);
            push_instr(program, R3B);

            push_instr(program, MOV);
            push_instr(program, R5B);
            push_instr(program, R3B);

            push_instr(program, LDR);
            push_instr(program, R2A);
            push_instr(program, R3B);

            push_instr(program, LII);
            push_instr(program, R3A);
            push_instr(program, -1);

            push_instr(program, ADD);
            push_instr(program, R3B);
            push_instr(program, R4B);

            push_instr(program, MOV);
            push_instr(program, R0A);
            push_instr(program, R3B);

            push_instr(program, INC);
            push_instr(program, R3B);

            push_instr(program, LDR);
            push_instr(program, R1A);
            push_instr(program, R3B);

            push_instr(program, STR);
            push_instr(program, R0A);
            push_instr(program, R1A);

            push_instr(program, ADD);
            push_instr(program, R2A);
            push_instr(program, R3A);

            push_instr(program, CMP);
            push_instr(program, R2A);
            push_instr(program, R3A);

            push_instr(program, JNZ);
            push_instr(program, program->size - 19);

            push_instr(program, LDR);
            push_instr(program, R1B);
            push_instr(program, R5B);

            push_instr(program, DEC);
            push_instr(program, R1B);

            push_instr(program, STR);
            push_instr(program, R5B);
            push_instr(program, R1B);
            break;
        }
        default:
            break;
        }
        break;
    }
    default:
        break;
    }
}

unsigned short compileExpr(i64_array* program, Expr* expr)
{
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

            i64 f;
            f64 _f = expr->v.basic_lit->value.f;
            memcpy(&f, &_f, sizeof f);

            push_instr(program, LII);
            push_instr(program, R1A);
            push_instr(program, f);
            break;
        case V_STRING: {
            size_t len = strlen(expr->v.basic_lit->value.s);
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
        }
        default:
            break;
        }
        return expr->v.basic_lit->value_type + 1;
        break;
    case Ident_kind: {
        Symbol* symbol = getSymbol(expr->v.ident->name);
        i64 addr = symbol->addr;
        switch (symbol->value_type) {
        case V_BOOL:
            load_bool(program, symbol);
            break;
        case V_INT:
            load_int(program, symbol);
            break;
        case V_FLOAT:
            load_float(program, symbol);
            break;
        case V_STRING: {
            load_string(program, symbol);
            break;
        }
        case V_LIST: {
            size_t len = symbol->len;
            addr += len + 1;
            for (size_t i = len; i > 0; i--) {
                push_instr(program, LII);
                push_instr(program, R7A);
                push_instr(program, addr--);

                push_instr(program, DLDR);
                push_instr(program, R7A);
            }
            addr--;

            push_instr(program, LDI);
            push_instr(program, R0A);
            push_instr(program, addr++);

            push_instr(program, LDI);
            push_instr(program, R1A);
            push_instr(program, addr++);
            break;
        }
        case V_DICT: {
            size_t len = symbol->len;
            addr += len * 2 - 1 + 2;
            for (size_t i = len; i > 0; i--) {
                push_instr(program, LII);
                push_instr(program, R7A);
                push_instr(program, addr--);

                push_instr(program, DLDR);
                push_instr(program, R7A);

                push_instr(program, LII);
                push_instr(program, R7A);
                push_instr(program, addr--);

                push_instr(program, DLDR);
                push_instr(program, R7A);
            }
            addr--;

            push_instr(program, LDI);
            push_instr(program, R0A);
            push_instr(program, addr++);

            push_instr(program, LDI);
            push_instr(program, R1A);
            push_instr(program, addr++);
            break;
        }
        default:
            break;
        }
        return symbol->value_type + 1;
        break;
    }
    case BinaryExpr_kind: {
        enum ValueType type = compileExpr(program, expr->v.binary_expr->y);
        shift_registers(program, 8);
        if (expr->v.binary_expr->x->kind == ParenExpr_kind || expr->v.binary_expr->x->kind == BinaryExpr_kind) {
            push_instr(program, PUSH);
            push_instr(program, R1B);
        }
        compileExpr(program, expr->v.binary_expr->x);
        if (expr->v.binary_expr->x->kind == ParenExpr_kind || expr->v.binary_expr->x->kind == BinaryExpr_kind) {
            push_instr(program, POP);
            push_instr(program, R1B);
        }
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
            push_instr(program, R0A);
            push_instr(program, V_BOOL);

            push_instr(program, LII);
            push_instr(program, R1A);
            push_instr(program, 1);

            push_instr(program, JEZ);
            push_instr(program, program->size + 3);

            push_instr(program, LII);
            push_instr(program, R1A);
            push_instr(program, 0);
            break;
        case NEQ_tok:
            push_instr(program, CMP);
            push_instr(program, R1A);
            push_instr(program, R1B);

            push_instr(program, LII);
            push_instr(program, R0A);
            push_instr(program, V_BOOL);

            push_instr(program, LII);
            push_instr(program, R1A);
            push_instr(program, 1);

            push_instr(program, JNZ);
            push_instr(program, program->size + 3);

            push_instr(program, LII);
            push_instr(program, R1A);
            push_instr(program, 0);
            break;
        case GTR_tok:
            push_instr(program, CMP);
            push_instr(program, R1A);
            push_instr(program, R1B);

            push_instr(program, LII);
            push_instr(program, R0A);
            push_instr(program, V_BOOL);

            push_instr(program, LII);
            push_instr(program, R1A);
            push_instr(program, 1);

            push_instr(program, JGZ);
            push_instr(program, program->size + 3);

            push_instr(program, LII);
            push_instr(program, R1A);
            push_instr(program, 0);
            break;
        case LSS_tok:
            push_instr(program, CMP);
            push_instr(program, R1A);
            push_instr(program, R1B);

            push_instr(program, LII);
            push_instr(program, R0A);
            push_instr(program, V_BOOL);

            push_instr(program, LII);
            push_instr(program, R1A);
            push_instr(program, 1);

            push_instr(program, JLZ);
            push_instr(program, program->size + 3);

            push_instr(program, LII);
            push_instr(program, R1A);
            push_instr(program, 0);
            break;
        case GEQ_tok:
            push_instr(program, CMP);
            push_instr(program, R1A);
            push_instr(program, R1B);

            push_instr(program, LII);
            push_instr(program, R0A);
            push_instr(program, V_BOOL);

            push_instr(program, LII);
            push_instr(program, R1A);
            push_instr(program, 1);

            push_instr(program, JEZ);
            push_instr(program, program->size + 5);
            push_instr(program, JGZ);
            push_instr(program, program->size + 3);

            push_instr(program, LII);
            push_instr(program, R1A);
            push_instr(program, 0);
            break;
        case LEQ_tok:
            push_instr(program, CMP);
            push_instr(program, R1A);
            push_instr(program, R1B);

            push_instr(program, LII);
            push_instr(program, R0A);
            push_instr(program, V_BOOL);

            push_instr(program, LII);
            push_instr(program, R1A);
            push_instr(program, 1);

            push_instr(program, JEZ);
            push_instr(program, program->size + 5);
            push_instr(program, JLZ);
            push_instr(program, program->size + 3);

            push_instr(program, LII);
            push_instr(program, R1A);
            push_instr(program, 0);
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
    }
    case UnaryExpr_kind: {
        enum ValueType type = compileExpr(program, expr->v.unary_expr->x);
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
    }
    case ParenExpr_kind:
        return compileExpr(program, expr->v.paren_expr->x);
        break;
    case IndexExpr_kind: {
        enum ValueType type = compileExpr(program, expr->v.index_expr->x);
        shift_registers(program, 8);

        push_instr(program, MOV);
        push_instr(program, R7B);
        push_instr(program, R1B);

        compileExpr(program, expr->v.index_expr->index);
        switch (type - 1) {
        case V_STRING: {
            push_instr(program, LII);
            push_instr(program, R0A);
            push_instr(program, V_STRING);

            push_instr(program, MOV);
            push_instr(program, R2A);
            push_instr(program, R1A);

            push_instr(program, MOV);
            push_instr(program, R4A);
            push_instr(program, R1A);

            push_instr(program, LII);
            push_instr(program, R3A);
            push_instr(program, -1);

            push_instr(program, POP);
            push_instr(program, R1A);

            push_instr(program, ADD);
            push_instr(program, R2A);
            push_instr(program, R3A);

            push_instr(program, CMP);
            push_instr(program, R2A);
            push_instr(program, R3A);

            push_instr(program, JNZ);
            push_instr(program, program->size - 10);

            push_instr(program, PUSH);
            push_instr(program, R1A);

            push_instr(program, LII);
            push_instr(program, R1A);
            push_instr(program, 1);
            break;
        }
        case V_LIST:
            push_instr(program, MOV);
            push_instr(program, R2A);
            push_instr(program, R1A);

            push_instr(program, LII);
            push_instr(program, R3A);
            push_instr(program, -1);

            push_instr(program, CMP);
            push_instr(program, R2A);
            push_instr(program, R3A);

            push_instr(program, JGZ);
            push_instr(program, program->size + 9);

            push_instr(program, MOV);
            push_instr(program, R4A);
            push_instr(program, R7B);

            push_instr(program, ADD);
            push_instr(program, R4A);
            push_instr(program, R2A);

            push_instr(program, MOV);
            push_instr(program, R2A);
            push_instr(program, R4A);

            push_instr(program, LII);
            push_instr(program, R3A);
            push_instr(program, -1);

            push_instr(program, POP);
            push_instr(program, R0A);

            push_instr(program, ADD);
            push_instr(program, R2A);
            push_instr(program, R3A);

            push_instr(program, CMP);
            push_instr(program, R2A);
            push_instr(program, R3A);

            push_instr(program, JEZ);
            push_instr(program, program->size + 3);

            push_instr(program, DPOP);

            push_instr(program, JNZ);
            push_instr(program, program->size - 13);

            push_instr(program, POP);
            push_instr(program, R1A);
            break;
        case V_DICT:
            push_instr(program, KSRCH);
            push_instr(program, R7B);
            push_instr(program, R1A);
            break;
        default:
            break;
        }
        return type;
        break;
    }
    case IncDecExpr_kind: {
        enum ValueType type = compileExpr(program, expr->v.incdec_expr->x);
        switch (expr->v.incdec_expr->op) {
        case INC_tok:
            push_instr(program, LII);
            push_instr(program, R1B);
            push_instr(program, 1);
            break;
        case DEC_tok:
            push_instr(program, LII);
            push_instr(program, R1B);
            push_instr(program, -1);
            break;
        default:
            break;
        }
        push_instr(program, ADD);
        push_instr(program, R1A);
        push_instr(program, R1B);
        if (expr->v.incdec_expr->x->kind == Ident_kind) {
            Symbol* symbol = getSymbol(expr->v.incdec_expr->x->v.ident->name);
            i64 addr = symbol->addr;
            push_instr(program, STI);
            push_instr(program, addr++);
            push_instr(program, R0A);

            push_instr(program, STI);
            push_instr(program, addr++);
            push_instr(program, R1A);
        }
        if (!expr->v.incdec_expr->first) {
            switch (expr->v.incdec_expr->op) {
            case INC_tok:
                push_instr(program, LII);
                push_instr(program, R1B);
                push_instr(program, -1);
                break;
            case DEC_tok:
                push_instr(program, LII);
                push_instr(program, R1B);
                push_instr(program, 1);
                break;
            default:
                break;
            }
            push_instr(program, ADD);
            push_instr(program, R1A);
            push_instr(program, R1B);
        }
        return type;
        break;
    }
    case CompositeLit_kind: {
        ExprList* expr_list = expr->v.composite_lit->elts;
        for (size_t i = 0; i < expr_list->expr_count; i++) {
            compileExpr(program, expr_list->exprs[i]);
            if (expr_list->exprs[i]->kind != KeyValueExpr_kind) {
                push_instr(program, PUSH);
                push_instr(program, R1A);

                push_instr(program, PUSH);
                push_instr(program, R0A);
            }
        }
        compileSpec(program, expr->v.composite_lit->type);
        push_instr(program, LII);
        push_instr(program, R1A);
        push_instr(program, expr_list->expr_count);
        break;
    }
    case KeyValueExpr_kind: {
        compileExpr(program, expr->v.key_value_expr->value);
        push_instr(program, PUSH);
        push_instr(program, R1A);

        push_instr(program, PUSH);
        push_instr(program, R0A);
        compileExpr(program, expr->v.key_value_expr->key);
        push_instr(program, PUSH);
        push_instr(program, R1A);

        push_instr(program, PUSH);
        push_instr(program, R0A);
    }
    default:
        break;
    }

    return 0;
}

void compileDecl(i64_array* program, Decl* decl)
{
    switch (decl->kind) {
    case VarDecl_kind: {
        enum ValueType value_type = compileExpr(program, decl->v.var_decl->expr) - 1;
        enum Type type = compileSpec(program, decl->v.var_decl->type_spec);

        switch (type) {
        case K_BOOL:
            store_bool(
                program,
                decl->v.var_decl->ident->v.ident->name,
                false
            );
            break;
        case K_NUMBER:
            if (value_type == V_FLOAT) {
                store_float(
                    program,
                    decl->v.var_decl->ident->v.ident->name,
                    false
                );
            } else {
                store_int(
                    program,
                    decl->v.var_decl->ident->v.ident->name,
                    false
                );
            }
            break;
        case K_STRING: {
            size_t len = 0;

            switch (decl->v.var_decl->expr->kind) {
            case BasicLit_kind:
                len = strlen(decl->v.var_decl->expr->v.basic_lit->value.s);
                break;
            case BinaryExpr_kind:
                len =
                    strlen(decl->v.var_decl->expr->v.binary_expr->x->v.basic_lit->value.s)
                    +
                    strlen(decl->v.var_decl->expr->v.binary_expr->y->v.basic_lit->value.s);
                break;
            case IndexExpr_kind:
                len = 1;
            default:
                break;
            }

            store_string(
                program,
                decl->v.var_decl->ident->v.ident->name,
                len,
                false
            );
            break;
        }
        case K_ANY: {
            switch (value_type) {
            case V_BOOL:
                store_bool(
                    program,
                    decl->v.var_decl->ident->v.ident->name,
                    true
                );
                break;
            case V_INT: {
                store_int(
                    program,
                    decl->v.var_decl->ident->v.ident->name,
                    true
                );
                break;
            }
            case V_FLOAT: {
                store_float(
                    program,
                    decl->v.var_decl->ident->v.ident->name,
                    true
                );
                break;
            }
            case V_STRING: {
                size_t len = 0;

                switch (decl->v.var_decl->expr->kind) {
                case BasicLit_kind:
                    len = strlen(decl->v.var_decl->expr->v.basic_lit->value.s);
                    break;
                case BinaryExpr_kind:
                    len =
                        strlen(decl->v.var_decl->expr->v.binary_expr->x->v.basic_lit->value.s)
                        +
                        strlen(decl->v.var_decl->expr->v.binary_expr->y->v.basic_lit->value.s);
                    break;
                case IndexExpr_kind:
                    len = 1;
                default:
                    break;
                }

                store_string(
                    program,
                    decl->v.var_decl->ident->v.ident->name,
                    len,
                    true
                );
                break;
            }
            default:
                break;
            }
            break;
        }
        case K_LIST: {
            size_t len = decl->v.var_decl->expr->v.composite_lit->elts->expr_count;

            Symbol* symbol = addSymbolListNew(
                decl->v.var_decl->ident->v.ident->name,
                len
            );
            symbol->addr = program->heap;

            push_instr(program, LII);
            push_instr(program, R0A);
            push_instr(program, V_LIST);

            push_instr(program, STI);
            push_instr(program, program->heap++);
            push_instr(program, R0A);

            push_instr(program, STI);
            push_instr(program, program->heap++);
            push_instr(program, R1A);

            for (size_t i = len; i > 0; i--) {
                push_instr(program, POP);
                push_instr(program, R0A);

                push_instr(program, DSTR);
                push_instr(program, R7A);

                push_instr(program, STI);
                push_instr(program, program->heap++);
                push_instr(program, R7A);
            }
            break;
        }
        case K_DICT: {
            size_t len = decl->v.var_decl->expr->v.composite_lit->elts->expr_count;

            Symbol* symbol = addSymbolDictNew(
                decl->v.var_decl->ident->v.ident->name,
                len
            );
            symbol->addr = program->heap;

            push_instr(program, LII);
            push_instr(program, R0A);
            push_instr(program, V_DICT);

            push_instr(program, STI);
            push_instr(program, program->heap++);
            push_instr(program, R0A);

            push_instr(program, STI);
            push_instr(program, program->heap++);
            push_instr(program, R1A);

            for (size_t i = len; i > 0; i--) {
                push_instr(program, POP);
                push_instr(program, R0A);

                push_instr(program, DSTR);
                push_instr(program, R7A);

                push_instr(program, STI);
                push_instr(program, program->heap++);
                push_instr(program, R7A);

                push_instr(program, POP);
                push_instr(program, R0A);

                push_instr(program, DSTR);
                push_instr(program, R7A);

                push_instr(program, STI);
                push_instr(program, program->heap++);
                push_instr(program, R7A);
            }
            break;
        }
        default:
            break;
        }
        break;
    }
    default:
        break;
    }
}

unsigned short compileSpec(i64_array* program, Spec* spec)
{
    switch (spec->kind) {
    case ListType_kind:
        push_instr(program, LII);
        push_instr(program, R0A);
        push_instr(program, V_LIST);
        break;
    case DictType_kind:
        push_instr(program, LII);
        push_instr(program, R0A);
        push_instr(program, V_DICT);
        break;
    case TypeSpec_kind:
        if (spec->v.type_spec->sub_type_spec != NULL)
            return compileSpec(program, spec->v.type_spec->sub_type_spec);
        else
            return spec->v.type_spec->type;
        break;
    default:
        break;
    }
    return 0;
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

Symbol* store_bool(i64_array* program, char *name, bool is_any)
{
    union Value value;
    value.i = 0;
    Symbol* symbol;
    if (is_any)
        symbol = addSymbol(name, K_ANY, value, V_BOOL);
    else {
        symbol = addSymbol(name, K_BOOL, value, V_BOOL);
        push_instr(program, LII);
        push_instr(program, R0A);
        push_instr(program, V_BOOL);
    }
    symbol->addr = program->heap;

    push_instr(program, STI);
    push_instr(program, program->heap++);
    push_instr(program, R0A);

    push_instr(program, STI);
    push_instr(program, program->heap++);
    push_instr(program, R1A);

    return symbol;
}

Symbol* store_int(i64_array* program, char *name, bool is_any)
{
    union Value value;
    value.i = 0;
    Symbol* symbol;
    if (is_any)
        symbol = addSymbol(name, K_ANY, value, V_INT);
    else {
        symbol = addSymbol(name, K_NUMBER, value, V_INT);
        push_instr(program, LII);
        push_instr(program, R0A);
        push_instr(program, V_INT);
    }
    symbol->addr = program->heap;

    push_instr(program, STI);
    push_instr(program, program->heap++);
    push_instr(program, R0A);

    push_instr(program, STI);
    push_instr(program, program->heap++);
    push_instr(program, R1A);

    return symbol;
}

Symbol* store_float(i64_array* program, char *name, bool is_any)
{
    union Value value;
    value.i = 0;
    Symbol* symbol;
    if (is_any)
        symbol = addSymbol(name, K_ANY, value, V_FLOAT);
    else {
        symbol = addSymbol(name, K_NUMBER, value, V_FLOAT);
        push_instr(program, LII);
        push_instr(program, R0A);
        push_instr(program, V_FLOAT);
    }
    symbol->addr = program->heap;

    push_instr(program, STI);
    push_instr(program, program->heap++);
    push_instr(program, R0A);

    push_instr(program, STI);
    push_instr(program, program->heap++);
    push_instr(program, R1A);

    return symbol;
}

Symbol* store_string(i64_array* program, char *name, size_t len, bool is_any)
{
    union Value value;
    value.i = 0;
    Symbol* symbol;
    if (is_any)
        symbol = addSymbol(name, K_ANY, value, V_STRING);
    else {
        symbol = addSymbol(name, K_STRING, value, V_STRING);
        push_instr(program, LII);
        push_instr(program, R0A);
        push_instr(program, V_STRING);
    }
    symbol->addr = program->heap;
    symbol->len = len;

    push_instr(program, STI);
    push_instr(program, program->heap++);
    push_instr(program, R0A);

    push_instr(program, STI);
    push_instr(program, program->heap++);
    push_instr(program, R1A);

    for (size_t i = len; i > 0; i--) {
        push_instr(program, POP);
        push_instr(program, R0A);

        push_instr(program, STI);
        push_instr(program, program->heap++);
        push_instr(program, R0A);
    }

    return symbol;
}

void load_bool(i64_array* program, Symbol* symbol)
{
    i64 addr = symbol->addr;

    push_instr(program, LDI);
    push_instr(program, R0A);
    push_instr(program, addr++);

    push_instr(program, LDI);
    push_instr(program, R1A);
    push_instr(program, addr++);
}

void load_int(i64_array* program, Symbol* symbol)
{
    i64 addr = symbol->addr;

    push_instr(program, LDI);
    push_instr(program, R0A);
    push_instr(program, addr++);

    push_instr(program, LDI);
    push_instr(program, R1A);
    push_instr(program, addr++);
}

void load_float(i64_array* program, Symbol* symbol)
{
    i64 addr = symbol->addr;

    push_instr(program, LDI);
    push_instr(program, R0A);
    push_instr(program, addr++);

    push_instr(program, LDI);
    push_instr(program, R1A);
    push_instr(program, addr++);
}

void load_string(i64_array* program, Symbol* symbol)
{
    i64 addr = symbol->addr;

    push_instr(program, LDI);
    push_instr(program, R0A);
    push_instr(program, addr++);

    push_instr(program, LDI);
    push_instr(program, R1A);
    push_instr(program, addr++);

    size_t len = symbol->len;
    addr += len - 1;
    for (size_t i = len; i > 0; i--) {
        push_instr(program, LDI);
        push_instr(program, R2A);
        push_instr(program, addr--);

        push_instr(program, PUSH);
        push_instr(program, R2A);
    }
    addr += len - 1;
}
