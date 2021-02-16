/*
 * Description: Abstract Syntax Tree printing module of the Chaos Programming Language's source
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

#include "ast_print.h"

unsigned short indent;

void printAST()
{
    indent = __KAOS_INDENT_LENGTH__;
    printf(
        "{\n%*c\"type\": \"Program\"\n%*c\"files\": [\n",
        indent,
        __KAOS_INDENT_CHAR__,
        indent,
        __KAOS_INDENT_CHAR__
    );
    indent = indent + __KAOS_INDENT_LENGTH__;

    for (unsigned long i = 0; i < program->file_count; i++) {
        printf(
            "%*c{\n",
            indent,
            __KAOS_INDENT_CHAR__
        );
        indent = indent + __KAOS_INDENT_LENGTH__;

        printf(
            "%*c\"type\": \"File\"\n%*c\"stmt_list\": [\n",
            indent,
            __KAOS_INDENT_CHAR__,
            indent,
            __KAOS_INDENT_CHAR__
        );
        indent = indent + __KAOS_INDENT_LENGTH__;

        StmtList* stmt_list = program->files[i]->stmt_list;
        for (unsigned long j = stmt_list->stmt_count; 0 < j; j--) {
            printASTStmt(stmt_list->stmts[j - 1], true);
        }

        indent = indent - __KAOS_INDENT_LENGTH__;
        printf(
            "%*c]\n",
            indent,
            __KAOS_INDENT_CHAR__
        );

        indent = indent - __KAOS_INDENT_LENGTH__;
        printf(
            "%*c}\n",
            indent,
            __KAOS_INDENT_CHAR__
        );
    }

    indent = indent - __KAOS_INDENT_LENGTH__;
    printf("%*c]\n}\n", indent, __KAOS_INDENT_CHAR__);
}

void printASTStmt(Stmt* stmt, bool is_list)
{
    if (is_list)
        printf(
            "%*c{\n",
            indent,
            __KAOS_INDENT_CHAR__
        );
    else
        printf("{\n");
    indent = indent + __KAOS_INDENT_LENGTH__;

    switch (stmt->kind)
    {
        case AssignStmt_kind:
            printf(
                "%*c\"type\": \"AssignStmt\"\n%*c\"x\": ",
                indent,
                __KAOS_INDENT_CHAR__,
                indent,
                __KAOS_INDENT_CHAR__
            );
            printASTExpr(stmt->v.assign_stmt->x, false);
            printf(
                "%*c\"op\": \"%s\",\n",
                indent,
                __KAOS_INDENT_CHAR__,
                getToken(stmt->v.assign_stmt->tok)
            );
            printf(
                "%*c\"y\": ",
                indent,
                __KAOS_INDENT_CHAR__
            );
            printASTExpr(stmt->v.assign_stmt->y, false);
            break;
        case ReturnStmt_kind:
            printf(
                "%*c\"type\": \"ReturnStmt\"\n%*c\"x\": ",
                indent,
                __KAOS_INDENT_CHAR__,
                indent,
                __KAOS_INDENT_CHAR__
            );
            printASTExpr(stmt->v.return_stmt->x, false);
            break;
        case PrintStmt_kind:
            printf(
                "%*c\"type\": \"PrintStmt\"\n%*c\"x\": ",
                indent,
                __KAOS_INDENT_CHAR__,
                indent,
                __KAOS_INDENT_CHAR__
            );
            printASTExpr(stmt->v.return_stmt->x, false);
            break;
        default:
            break;
    }

    indent = indent - __KAOS_INDENT_LENGTH__;
    printf("%*c}\n", indent, __KAOS_INDENT_CHAR__);
}

void printASTExpr(Expr* expr, bool is_list)
{
    if (is_list)
        printf(
            "%*c{\n",
            indent,
            __KAOS_INDENT_CHAR__
        );
    else
        printf("{\n");
    indent = indent + __KAOS_INDENT_LENGTH__;

    switch (expr->kind)
    {
        case BasicLit_kind:
            printf(
                "%*c\"type\": \"BasicLit\",\n",
                indent,
                __KAOS_INDENT_CHAR__
            );
            switch (expr->v.basic_lit->value_type)
            {
                case V_BOOL:
                    printf(
                        "%*c\"value_type\": \"bool\",\n"
                        "%*c\"value\": \"%s\",\n",
                        indent,
                        __KAOS_INDENT_CHAR__,
                        indent,
                        __KAOS_INDENT_CHAR__,
                        expr->v.basic_lit->value.b ? "true" : "false"
                    );
                    break;
                case V_INT:
                    printf(
                        "%*c\"value_type\": \"int\",\n"
                        "%*c\"value\": \"%lld\",\n",
                        indent,
                        __KAOS_INDENT_CHAR__,
                        indent,
                        __KAOS_INDENT_CHAR__,
                        expr->v.basic_lit->value.i
                    );
                    break;
                case V_FLOAT:
                    printf(
                        "%*c\"value_type\": \"float\",\n"
                        "%*c\"value\": \"%Lg\",\n",
                        indent,
                        __KAOS_INDENT_CHAR__,
                        indent,
                        __KAOS_INDENT_CHAR__,
                        expr->v.basic_lit->value.f
                    );
                    break;
                case V_STRING:
                    printf(
                        "%*c\"value_type\": \"int\",\n"
                        "%*c\"value\": \"%s\",\n",
                        indent,
                        __KAOS_INDENT_CHAR__,
                        indent,
                        __KAOS_INDENT_CHAR__,
                        expr->v.basic_lit->value.s
                    );
                    break;
                default:
                    break;
            }
            break;
        case Ident_kind:
            printf(
                "%*c\"type\": \"Ident\",\n"
                "%*c\"name\": \"%s\",\n",
                indent,
                __KAOS_INDENT_CHAR__,
                indent,
                __KAOS_INDENT_CHAR__,
                expr->v.ident->name
            );
            break;
        case BinaryExpr_kind:
            printf(
                "%*c\"type\": \"BinaryExpr\"\n%*c\"x\": ",
                indent,
                __KAOS_INDENT_CHAR__,
                indent,
                __KAOS_INDENT_CHAR__
            );
            printASTExpr(expr->v.binary_expr->x, false);
            printf(
                "%*c\"op\": \"%s\",\n",
                indent,
                __KAOS_INDENT_CHAR__,
                getToken(expr->v.binary_expr->op)
            );
            printf(
                "%*c\"y\": ",
                indent,
                __KAOS_INDENT_CHAR__
            );
            printASTExpr(expr->v.binary_expr->y, false);
            break;
        case UnaryExpr_kind:
            printf(
                "%*c\"type\": \"UnaryExpr\"\n%*c\"op\": \"%s\",\n",
                indent,
                __KAOS_INDENT_CHAR__,
                indent,
                __KAOS_INDENT_CHAR__,
                getToken(expr->v.unary_expr->op)
            );
            printf(
                "%*c\"x\": ",
                indent,
                __KAOS_INDENT_CHAR__
            );
            printASTExpr(expr->v.unary_expr->x, false);
            break;
        case ParenExpr_kind:
            printf(
                "%*c\"type\": \"ParenExpr\"\n%*c\"x\": ",
                indent,
                __KAOS_INDENT_CHAR__,
                indent,
                __KAOS_INDENT_CHAR__
            );
            printASTExpr(expr->v.paren_expr->x, false);
            break;
        default:
            break;
    }

    indent = indent - __KAOS_INDENT_LENGTH__;
    printf("%*c}\n", indent, __KAOS_INDENT_CHAR__);
}

char *getToken(enum Token tok)
{
    switch (tok)
    {
        case ADD_token:
            return ADD;
            break;
        case SUB_token:
            return SUB;
            break;
        case MUL_token:
            return MUL;
            break;
        case QUO_token:
            return QUO;
            break;
        case REM_token:
            return REM;
            break;
        case EQL_token:
            return EQL;
            break;
        default:
            break;
    }
    return ILLEGAL;
}
