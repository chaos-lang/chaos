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
        "{\n%*c\"_type\": \"Program\",\n%*c\"files\": [\n",
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
            "%*c\"_type\": \"File\",\n",
            indent,
            __KAOS_INDENT_CHAR__
        );

        // Imports
        printf(
            "%*c\"imports\": [\n",
            indent,
            __KAOS_INDENT_CHAR__
        );
        indent = indent + __KAOS_INDENT_LENGTH__;

        SpecList* imports = program->files[i]->imports;
        for (unsigned long j = imports->spec_count; 0 < j; j--) {
            if (j - 1 == 0)
                printASTSpec(imports->specs[j - 1], true, "\n");
            else
                printASTSpec(imports->specs[j - 1], true, ",\n");
        }

        indent = indent - __KAOS_INDENT_LENGTH__;
        printf(
            "%*c],\n",
            indent,
            __KAOS_INDENT_CHAR__
        );

        // Stmts
        printf(
            "%*c\"stmt_list\": [\n",
            indent,
            __KAOS_INDENT_CHAR__
        );
        indent = indent + __KAOS_INDENT_LENGTH__;

        StmtList* stmt_list = program->files[i]->stmt_list;
        for (unsigned long j = stmt_list->stmt_count; 0 < j; j--) {
            if (j - 1 == 0)
                printASTStmt(stmt_list->stmts[j - 1], true, "\n");
            else
                printASTStmt(stmt_list->stmts[j - 1], true, ",\n");
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

void printASTStmt(Stmt* stmt, bool is_list, char *end)
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
                "%*c\"_type\": \"AssignStmt\",\n%*c\"x\": ",
                indent,
                __KAOS_INDENT_CHAR__,
                indent,
                __KAOS_INDENT_CHAR__
            );
            printASTExpr(stmt->v.assign_stmt->x, false, ",\n");
            printf(
                "%*c\"op\": \"%s\"",
                indent,
                __KAOS_INDENT_CHAR__,
                getToken(stmt->v.assign_stmt->tok)
            );
            printf(
                ",\n%*c\"y\": ",
                indent,
                __KAOS_INDENT_CHAR__
            );
            printASTExpr(stmt->v.assign_stmt->y, false, "\n");
            break;
        case ReturnStmt_kind:
            printf(
                "%*c\"_type\": \"ReturnStmt\",\n%*c\"x\": ",
                indent,
                __KAOS_INDENT_CHAR__,
                indent,
                __KAOS_INDENT_CHAR__
            );
            printASTExpr(stmt->v.return_stmt->x, false, "\n");
            break;
        case PrintStmt_kind:
            printf(
                "%*c\"_type\": \"PrintStmt\",\n%*c\"mod\": ",
                indent,
                __KAOS_INDENT_CHAR__,
                indent,
                __KAOS_INDENT_CHAR__
            );
            if (stmt->v.print_stmt->mod == NULL)
                printf("null,\n");
            else
                printASTSpec(stmt->v.print_stmt->mod, false, ",\n");
            printf(
                "%*c\"x\": ",
                indent,
                __KAOS_INDENT_CHAR__
            );
            printASTExpr(stmt->v.print_stmt->x, false, "\n");
            break;
        case EchoStmt_kind:
            printf(
                "%*c\"_type\": \"EchoStmt\",\n%*c\"mod\": ",
                indent,
                __KAOS_INDENT_CHAR__,
                indent,
                __KAOS_INDENT_CHAR__
            );
            if (stmt->v.echo_stmt->mod == NULL)
                printf("null,\n");
            else
                printASTSpec(stmt->v.echo_stmt->mod, false, ",\n");
            printf(
                "%*c\"x\": ",
                indent,
                __KAOS_INDENT_CHAR__
            );
            printASTExpr(stmt->v.echo_stmt->x, false, "\n");
            break;
        case ExprStmt_kind:
            printf(
                "%*c\"_type\": \"ExprStmt\",\n%*c\"x\": ",
                indent,
                __KAOS_INDENT_CHAR__,
                indent,
                __KAOS_INDENT_CHAR__
            );
            printASTExpr(stmt->v.expr_stmt->x, false, "\n");
            break;
        case DeclStmt_kind:
            printf(
                "%*c\"_type\": \"DeclStmt\",\n%*c\"decl\": ",
                indent,
                __KAOS_INDENT_CHAR__,
                indent,
                __KAOS_INDENT_CHAR__
            );
            printASTDecl(stmt->v.decl_stmt->decl, false, "\n");
            break;
        case DelStmt_kind:
            printf(
                "%*c\"_type\": \"DelStmt\",\n%*c\"ident\": ",
                indent,
                __KAOS_INDENT_CHAR__,
                indent,
                __KAOS_INDENT_CHAR__
            );
            printASTExpr(stmt->v.del_stmt->ident, false, "\n");
            break;
        case SymbolTableStmt_kind:
            printf(
                "%*c\"_type\": \"SymbolTableStmt\"\n",
                indent,
                __KAOS_INDENT_CHAR__
            );
            break;
        case FunctionTableStmt_kind:
            printf(
                "%*c\"_type\": \"FunctionTableStmt\"\n",
                indent,
                __KAOS_INDENT_CHAR__
            );
            break;
        default:
            break;
    }

    indent = indent - __KAOS_INDENT_LENGTH__;
    printf("%*c}%s", indent, __KAOS_INDENT_CHAR__, end);
}

void printASTExpr(Expr* expr, bool is_list, char *end)
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
                "%*c\"_type\": \"BasicLit\",\n",
                indent,
                __KAOS_INDENT_CHAR__
            );
            switch (expr->v.basic_lit->value_type)
            {
                case V_BOOL:
                    printf(
                        "%*c\"value_type\": \"bool\",\n"
                        "%*c\"value\": \"%s\"\n",
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
                        "%*c\"value\": \"%lld\"\n",
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
                        "%*c\"value\": \"%Lg\"\n",
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
                        "%*c\"value\": \"%s\"\n",
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
                "%*c\"_type\": \"Ident\",\n"
                "%*c\"name\": \"%s\"\n",
                indent,
                __KAOS_INDENT_CHAR__,
                indent,
                __KAOS_INDENT_CHAR__,
                expr->v.ident->name
            );
            break;
        case BinaryExpr_kind:
            printf(
                "%*c\"_type\": \"BinaryExpr\",\n%*c\"x\": ",
                indent,
                __KAOS_INDENT_CHAR__,
                indent,
                __KAOS_INDENT_CHAR__
            );
            printASTExpr(expr->v.binary_expr->x, false, ",\n");
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
            printASTExpr(expr->v.binary_expr->y, false, "\n");
            break;
        case UnaryExpr_kind:
            printf(
                "%*c\"_type\": \"UnaryExpr\",\n%*c\"op\": \"%s\",\n",
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
            printASTExpr(expr->v.unary_expr->x, false, "\n");
            break;
        case ParenExpr_kind:
            printf(
                "%*c\"_type\": \"ParenExpr\",\n%*c\"x\": ",
                indent,
                __KAOS_INDENT_CHAR__,
                indent,
                __KAOS_INDENT_CHAR__
            );
            printASTExpr(expr->v.paren_expr->x, false, "\n");
            break;
        case IncDecExpr_kind:
            printf(
                "%*c\"_type\": \"IncDecExpr\",\n%*c\"op\": \"%s\",\n",
                indent,
                __KAOS_INDENT_CHAR__,
                indent,
                __KAOS_INDENT_CHAR__,
                getToken(expr->v.incdec_expr->op)
            );
            printf(
                "%*c\"ident\": ",
                indent,
                __KAOS_INDENT_CHAR__
            );
            printASTExpr(expr->v.incdec_expr->ident, false, ",\n");
            printf(
                "%*c\"first\": %s\n",
                indent,
                __KAOS_INDENT_CHAR__,
                expr->v.incdec_expr->first ? "true" : "false"
            );
            break;
        case ModuleSelector_kind:
            printf(
                "%*c\"_type\": \"ModuleSelector\",\n%*c\"parent_dir_spec\": ",
                indent,
                __KAOS_INDENT_CHAR__,
                indent,
                __KAOS_INDENT_CHAR__
            );
            if (expr->v.module_selector->parent_dir_spec == NULL)
                printf("null,\n");
            else
                printASTSpec(expr->v.module_selector->parent_dir_spec, false, ",\n");
            printf(
                "%*c\"x\": ",
                indent,
                __KAOS_INDENT_CHAR__
            );
            if (expr->v.module_selector->x == NULL)
                printf("null,\n");
            else
                printASTExpr(expr->v.module_selector->x, false, ",\n");
            printf(
                "%*c\"sel\": ",
                indent,
                __KAOS_INDENT_CHAR__
            );
            if (expr->v.module_selector->sel == NULL)
                printf("null\n");
            else
                printASTExpr(expr->v.module_selector->sel, false, "\n");
            break;
        case AliasExpr_kind:
            printf(
                "%*c\"_type\": \"AliasExpr\",\n%*c\"name\": ",
                indent,
                __KAOS_INDENT_CHAR__,
                indent,
                __KAOS_INDENT_CHAR__
            );
            printASTExpr(expr->v.alias_expr->name, false, ",\n");
            printf(
                "%*c\"asname\": ",
                indent,
                __KAOS_INDENT_CHAR__
            );
            if (expr->v.alias_expr->asname == NULL)
                printf("null\n");
            else
                printASTExpr(expr->v.alias_expr->asname, false, "\n");
            break;
        case IndexExpr_kind:
            printf(
                "%*c\"_type\": \"IndexExpr\",\n%*c\"x\": ",
                indent,
                __KAOS_INDENT_CHAR__,
                indent,
                __KAOS_INDENT_CHAR__
            );
            printASTExpr(expr->v.index_expr->x, false, ",\n");
            printf(
                "%*c\"index\": ",
                indent,
                __KAOS_INDENT_CHAR__
            );
            printASTExpr(expr->v.index_expr->index, false, "\n");
            break;
        default:
            break;
    }

    indent = indent - __KAOS_INDENT_LENGTH__;
    printf("%*c}%s", indent, __KAOS_INDENT_CHAR__, end);
}

void printASTSpec(Spec* spec, bool is_list, char *end)
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

    switch (spec->kind)
    {
        case TypeSpec_kind:
            printf(
                "%*c\"_type\": \"TypeSpec\",\n"
                "%*c\"type\": \"%s\",\n"
                "%*c\"sub_type_spec\": ",
                indent,
                __KAOS_INDENT_CHAR__,
                indent,
                __KAOS_INDENT_CHAR__,
                getTypeName(spec->v.type_spec->type),
                indent,
                __KAOS_INDENT_CHAR__
            );
            if (spec->v.type_spec->sub_type_spec == NULL)
                printf("null\n");
            else
                printASTSpec(spec->v.type_spec->sub_type_spec, false, "\n");
            break;
        case PrettySpec_kind:
            printf(
                "%*c\"_type\": \"PrettySpec\"\n",
                indent,
                __KAOS_INDENT_CHAR__
            );
            break;
        case ParentDirSpec_kind:
            printf(
                "%*c\"_type\": \"ParentDirSpec\"\n",
                indent,
                __KAOS_INDENT_CHAR__
            );
            break;
        case AsteriskSpec_kind:
            printf(
                "%*c\"_type\": \"AsteriskSpec\"\n",
                indent,
                __KAOS_INDENT_CHAR__
            );
            break;
        case ImportSpec_kind:
            printf(
                "%*c\"_type\": \"ImportSpec\",\n%*c\"module_selector\": ",
                indent,
                __KAOS_INDENT_CHAR__,
                indent,
                __KAOS_INDENT_CHAR__
            );
            printASTExpr(spec->v.import_spec->module_selector, false, ",\n");
            printf(
                "%*c\"ident\": ",
                indent,
                __KAOS_INDENT_CHAR__
            );
            if (spec->v.import_spec->ident == NULL)
                printf("null,\n");
            else
                printASTExpr(spec->v.import_spec->ident, false, ",\n");
            printf(
                "%*c\"names\": [\n",
                indent,
                __KAOS_INDENT_CHAR__
            );

            indent = indent + __KAOS_INDENT_LENGTH__;
            ExprList* names = spec->v.import_spec->names;
            for (unsigned long j = names->expr_count; 0 < j; j--) {
                if (j - 1 == 0)
                    printASTExpr(names->exprs[j - 1], true, "\n");
                else
                    printASTExpr(names->exprs[j - 1], true, ",\n");
            }
            indent = indent - __KAOS_INDENT_LENGTH__;

            printf(
                "%*c],\n",
                indent,
                __KAOS_INDENT_CHAR__
            );

            printf(
                "%*c\"asterisk\": ",
                indent,
                __KAOS_INDENT_CHAR__
            );
            if (spec->v.import_spec->asterisk == NULL)
                printf("null\n");
            else
                printASTSpec(spec->v.import_spec->asterisk, false, "\n");
        default:
            break;
    }

    indent = indent - __KAOS_INDENT_LENGTH__;
    printf("%*c}%s", indent, __KAOS_INDENT_CHAR__, end);
}

void printASTDecl(Decl* decl, bool is_list, char *end)
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

    switch (decl->kind)
    {
        case VarDecl_kind:
            printf(
                "%*c\"_type\": \"VarDecl\",\n%*c\"type_spec\": ",
                indent,
                __KAOS_INDENT_CHAR__,
                indent,
                __KAOS_INDENT_CHAR__
            );
            printASTSpec(decl->v.var_decl->type_spec, false, ",\n");
            printf(
                "%*c\"ident\": ",
                indent,
                __KAOS_INDENT_CHAR__
            );
            printASTExpr(decl->v.var_decl->ident, false, ",\n");
            printf(
                "%*c\"expr\": ",
                indent,
                __KAOS_INDENT_CHAR__
            );
            printASTExpr(decl->v.var_decl->expr, false, "\n");
            break;
        default:
            break;
    }

    indent = indent - __KAOS_INDENT_LENGTH__;
    printf("%*c}%s", indent, __KAOS_INDENT_CHAR__, end);
}

char *getToken(enum Token tok)
{
    switch (tok)
    {
        case NEWLINE_tok:
            return TT_NEWLINE;
            break;
        case COMMENT1_tok:
            return TT_COMMENT1;
            break;
        case COMMENT2_tok:
            return TT_COMMENT2;
            break;
        case ASSIGN_tok:
            return TT_ASSIGN;
            break;
        case ADD_tok:
            return TT_ADD;
            break;
        case SUB_tok:
            return TT_SUB;
            break;
        case MUL_tok:
            return TT_MUL;
            break;
        case QUO_tok:
            return TT_QUO;
            break;
        case REM_tok:
            return TT_REM;
            break;
        case BACKSLASH_tok:
            return TT_BACKSLASH;
            break;
        case LPAREN_tok:
            return TT_LPAREN;
            break;
        case RPAREN_tok:
            return TT_RPAREN;
            break;
        case LBRACK_tok:
            return TT_LBRACK;
            break;
        case RBRACK_tok:
            return TT_RBRACK;
            break;
        case LBRACE_tok:
            return TT_LBRACE;
            break;
        case RBRACE_tok:
            return TT_RBRACE;
            break;
        case COMMA_tok:
            return TT_COMMA;
            break;
        case PERIOD_tok:
            return TT_PERIOD;
            break;
        case EQL_tok:
            return TT_EQL;
            break;
        case NEQ_tok:
            return TT_NEQ;
            break;
        case GTR_tok:
            return TT_GTR;
            break;
        case LSS_tok:
            return TT_LSS;
            break;
        case GEQ_tok:
            return TT_GEQ;
            break;
        case LEQ_tok:
            return TT_LEQ;
            break;
        case LAND_tok:
            return TT_LAND;
            break;
        case LOR_tok:
            return TT_LOR;
            break;
        case NOT_tok:
            return TT_NOT;
            break;
        case AND_tok:
            return TT_AND;
            break;
        case OR_tok:
            return TT_OR;
            break;
        case XOR_tok:
            return TT_XOR;
            break;
        case TILDE_tok:
            return TT_TILDE;
            break;
        case SHL_tok:
            return TT_SHL;
            break;
        case SHR_tok:
            return TT_SHR;
            break;
        case INC_tok:
            return TT_INC;
            break;
        case DEC_tok:
            return TT_DEC;
            break;
        case COLON_tok:
            return TT_COLON;
            break;
        case EXIT_tok:
            return TT_EXIT;
            break;
        case PRINT_tok:
            return TT_PRINT;
            break;
        case ECHO_tok:
            return TT_ECHO;
            break;
        case PRETTY_tok:
            return TT_PRETTY;
            break;
        case TRUE_tok:
            return TT_TRUE;
            break;
        case FALSE_tok:
            return TT_FALSE;
            break;
        case SYMBOL_TABLE_tok:
            return TT_SYMBOL_TABLE;
            break;
        case FUNCTION_TABLE_tok:
            return TT_FUNCTION_TABLE;
            break;
        case DEL_tok:
            return TT_DEL;
            break;
        case RETURN_tok:
            return TT_RETURN;
            break;
        case DEFAULT_tok:
            return TT_DEFAULT;
            break;
        case TIMES_DO_tok:
            return TT_TIMES_DO;
            break;
        case END_tok:
            return TT_END;
            break;
        case FOREACH_tok:
            return TT_FOREACH;
            break;
        case AS_tok:
            return TT_AS;
            break;
        case FROM_tok:
            return TT_FROM;
            break;
        case INFINITE_tok:
            return TT_INFINITE;
            break;
        default:
            break;
    }
    return TT_ILLEGAL;
}
