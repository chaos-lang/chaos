%{
/*
 * Description: Parser of the Chaos Programming Language's source
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
 *          Melih Sahin <melihsahin24@gmail.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <setjmp.h>

#include "parser/parser.h"

extern int yylex();
extern int yyparse();
extern int yylex_destroy();
extern FILE* yyin;

extern int yylineno;
extern char *yytext;

#ifndef CHAOS_COMPILER
extern bool is_interactive;
#endif

bool inject_mode = false;

extern char *main_interpreted_module;
extern unsigned long long loops_inside_function_counter;
extern bool is_complex_parsing;
%}

%union {
    bool bval;
    long long ival;
    long double fval;
    char *sval;
    unsigned long long lluval;
    Expr* expr;
    Stmt* stmt;
    Spec* spec;
    Decl* decl;
}

%token START_PROGRAM START_PREPARSE START_JSON_PARSE
%token<bval> T_TRUE T_FALSE
%token<ival> T_INT T_TIMES_DO_INT
%token<fval> T_FLOAT
%token<sval> T_STRING T_VAR
%token<lluval> T_UNSIGNED_LONG_LONG_INT
%token T_ADD T_SUB T_MUL T_QUO T_REM T_LPAREN T_RPAREN T_ASSIGN
%token T_LBRACK T_RBRACK T_LBRACE T_RBRACE T_COMMA T_PERIOD T_COLON
%token T_NEWLINE T_EXIT
%token T_PRINT T_ECHO T_PRETTY
%token T_VAR_BOOL T_VAR_NUMBER T_VAR_STRING T_VAR_LIST T_VAR_DICT T_VAR_ANY T_NULL
%token T_DEL T_RETURN T_VOID T_DEFAULT T_BREAK T_CONTINUE
%token T_SYMBOL_TABLE T_FUNCTION_TABLE
%token T_TIMES_DO T_FOREACH T_AS T_END T_DEF T_IMPORT T_FROM T_BACKSLASH T_INFINITE
%token T_EQL T_NEQ T_GTR T_LSS T_GEQ T_LEQ
%token T_LAND T_LOR T_NOT
%token T_AND T_OR T_XOR T_TILDE T_SHL T_SHR
%token T_INC T_DEC
%left T_ADD T_SUB T_NOT T_TILDE T_VAR
%left T_MUL T_QUO
%right T_U_ADD T_U_SUB T_U_NOT T_U_TILDE

%type<expr> expr basic_lit ident binary_expr unary_expr paren_expr incdec_expr
%type<stmt> stmt assign_stmt print_stmt echo_stmt return_stmt expr_stmt decl_stmt del_stmt
%type<stmt> symbol_table_stmt function_table_stmt
%type<spec> type_spec sub_type_spec pretty_spec
%type<decl> var_decl

%destructor {
    free($$);
} <sval>

%start meta_start

%%

meta_start:
    | START_PROGRAM parser {}
;

parser:
    | line {}
;

line:
    | T_NEWLINE line {}
    | stmt line {
        addStmt(program->files[0]->stmt_list, $1);
    }
;

expr:
    ident {
        $$ = $1;
    }
    | basic_lit {
        $$ = $1;
    }
    | binary_expr {
        $$ = $1;
    }
    | unary_expr {
        $$ = $1;
    }
    | paren_expr {
        $$ = $1;
    }
    | incdec_expr {
        $$ = $1;
    }
;

ident:
    T_VAR {
        $$ = ident($1, yylineno);
    }
;

basic_lit:
    T_TRUE {
        $$ = basicLitBool($1, yylineno);
    }
    | T_FALSE {
        $$ = basicLitBool($1, yylineno);
    }
    | T_INT {
        $$ = basicLitInt($1, yylineno);
    }
    | T_FLOAT {
        $$ = basicLitFloat($1, yylineno);
    }
    | T_STRING {
        $$ = basicLitString($1, yylineno);
    }
;

binary_expr:
    expr T_ADD expr {
        $$ = binaryExpr($1, ADD_tok, $3, yylineno);
    }
    | expr T_SUB expr {
        $$ = binaryExpr($1, SUB_tok, $3, yylineno);
    }
    | expr T_MUL expr {
        $$ = binaryExpr($1, MUL_tok, $3, yylineno);
    }
    | expr T_QUO expr {
        $$ = binaryExpr($1, QUO_tok, $3, yylineno);
    }
    | expr T_REM expr {
        $$ = binaryExpr($1, REM_tok, $3, yylineno);
    }
    | expr T_EQL expr {
        $$ = binaryExpr($1, EQL_tok, $3, yylineno);
    }
    | expr T_NEQ expr {
        $$ = binaryExpr($1, NEQ_tok, $3, yylineno);
    }
    | expr T_GTR expr {
        $$ = binaryExpr($1, GTR_tok, $3, yylineno);
    }
    | expr T_LSS expr {
        $$ = binaryExpr($1, LSS_tok, $3, yylineno);
    }
    | expr T_GEQ expr {
        $$ = binaryExpr($1, GEQ_tok, $3, yylineno);
    }
    | expr T_LEQ expr {
        $$ = binaryExpr($1, LEQ_tok, $3, yylineno);
    }
    | expr T_LAND expr {
        $$ = binaryExpr($1, LAND_tok, $3, yylineno);
    }
    | expr T_LOR expr {
        $$ = binaryExpr($1, LOR_tok, $3, yylineno);
    }
    | expr T_AND expr {
        $$ = binaryExpr($1, AND_tok, $3, yylineno);
    }
    | expr T_OR expr {
        $$ = binaryExpr($1, OR_tok, $3, yylineno);
    }
    | expr T_XOR expr {
        $$ = binaryExpr($1, XOR_tok, $3, yylineno);
    }
    | expr T_SHL expr {
        $$ = binaryExpr($1, SHL_tok, $3, yylineno);
    }
    | expr T_SHR expr {
        $$ = binaryExpr($1, SHR_tok, $3, yylineno);
    }
;

unary_expr:
    T_ADD expr %prec T_U_ADD {
        $$ = unaryExpr(ADD_tok, $2, yylineno);
    }
    | T_SUB expr %prec T_U_SUB {
        $$ = unaryExpr(SUB_tok, $2, yylineno);
    }
    | T_NOT expr %prec T_U_NOT {
        $$ = unaryExpr(NOT_tok, $2, yylineno);
    }
    | T_TILDE expr %prec T_U_TILDE {
        $$ = unaryExpr(TILDE_tok, $2, yylineno);
    }
;

paren_expr:
    T_LPAREN expr T_RPAREN {
        $$ = parenExpr($2, yylineno);
    }
;

incdec_expr:
    T_INC ident {
        $$ = incDecExpr(INC_tok, $2, true, yylineno);
    }
    | ident T_INC {
        $$ = incDecExpr(INC_tok, $1, false, yylineno);
    }
    | T_DEC ident {
        $$ = incDecExpr(DEC_tok, $2, true, yylineno);
    }
    | ident T_DEC {
        $$ = incDecExpr(DEC_tok, $1, false, yylineno);
    }
;

stmt:
    assign_stmt T_NEWLINE {
        $$ = $1;
    }
    | return_stmt T_NEWLINE {
        $$ = $1;
    }
    | print_stmt T_NEWLINE {
        $$ = $1;
    }
    | echo_stmt T_NEWLINE {
        $$ = $1;
    }
    | expr_stmt T_NEWLINE {
        $$ = $1;
    }
    | decl_stmt T_NEWLINE {
        $$ = $1;
    }
    | del_stmt T_NEWLINE {
        $$ = $1;
    }
    | symbol_table_stmt T_NEWLINE {
        $$ = $1;
    }
    | function_table_stmt T_NEWLINE {
        $$ = $1;
    }
;

assign_stmt:
    expr T_ASSIGN expr {
        $$ = assignStmt($1, EQL_tok, $3, yylineno);
    }
;

return_stmt:
    T_RETURN expr {
        $$ = returnStmt($2, yylineno);
    }
;

print_stmt:
    T_PRINT expr {
        $$ = printStmt(NULL, $2, yylineno);
    }
    | pretty_spec T_PRINT expr {
        $$ = printStmt($1, $3, yylineno);
    }
;

echo_stmt:
    T_ECHO expr {
        $$ = echoStmt(NULL, $2, yylineno);
    }
    | pretty_spec T_ECHO expr {
        $$ = echoStmt($1, $3, yylineno);
    }
;

expr_stmt:
    expr {
        $$ = exprStmt($1, yylineno);
    }
;

decl_stmt:
    var_decl {
        $$ = declStmt($1, yylineno);
    }
;

del_stmt:
    T_DEL ident {
        $$ = delStmt($2, yylineno);
    }
;

symbol_table_stmt:
    T_SYMBOL_TABLE {
        $$ = symbolTableStmt(yylineno);
    }
;

function_table_stmt:
    T_FUNCTION_TABLE {
        $$ = functionTableStmt(yylineno);
    }
;

type_spec:
    T_VAR_BOOL {
        $$ = typeSpec(K_BOOL, NULL, yylineno);
    }
    | T_VAR_NUMBER {
        $$ = typeSpec(K_NUMBER, NULL, yylineno);
    }
    | T_VAR_STRING {
        $$ = typeSpec(K_STRING, NULL, yylineno);
    }
    | T_VAR_ANY {
        $$ = typeSpec(K_ANY, NULL, yylineno);
    }
    | T_VAR_LIST {
        $$ = typeSpec(K_LIST, NULL, yylineno);
    }
    | T_VAR_DICT {
        $$ = typeSpec(K_DICT, NULL, yylineno);
    }
    | T_VAR_BOOL sub_type_spec {
        $$ = typeSpec(K_BOOL, $2, yylineno);
    }
    | T_VAR_NUMBER sub_type_spec {
        $$ = typeSpec(K_NUMBER, $2, yylineno);
    }
    | T_VAR_STRING sub_type_spec {
        $$ = typeSpec(K_STRING, $2, yylineno);
    }
    | T_VAR_ANY sub_type_spec {
        $$ = typeSpec(K_ANY, $2, yylineno);
    }
;

sub_type_spec:
    T_VAR_LIST {
        $$ = typeSpec(K_LIST, NULL, yylineno);
    }
    | T_VAR_DICT {
        $$ = typeSpec(K_DICT, NULL, yylineno);
    }
    | T_VAR_LIST sub_type_spec {
        $$ = typeSpec(K_LIST, $2, yylineno);
    }
    | T_VAR_DICT sub_type_spec {
        $$ = typeSpec(K_DICT, $2, yylineno);
    }
;

pretty_spec:
    T_PRETTY {
        $$ = prettySpec(yylineno);
    }
;

var_decl:
    type_spec ident T_ASSIGN expr {
        $$ = varDecl($1, $2, $4, yylineno);
    }
;

%%

#ifndef CHAOS_COMPILER
int main(int argc, char** argv) {
    initParser(argc, argv);
}
#endif
