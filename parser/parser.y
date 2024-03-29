%{
/*
 * Description: Parser of the Chaos Programming Language's source
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
bool interactively_importing = false;

extern char *main_interpreted_module;
extern unsigned long long shell_indicator_block_counter;
%}

%union {
    bool bval;
    long long ival;
    double fval;
    char *sval;
    unsigned long long lluval;
    Expr* expr;
    Stmt* stmt;
    Spec* spec;
    Decl* decl;
    ExprList* expr_list;
    StmtList* stmt_list;
    FuncDeclCom* func_decl_com;
}

%token START_PROGRAM START_PREPARSE START_JSON_PARSE
%token<bval> T_TRUE T_FALSE
%token<ival> T_INT T_TIMES_DO_INT
%token<fval> T_FLOAT
%token<sval> T_STRING T_VAR
%token<lluval> T_UNSIGNED_LONG_LONG_INT
%token T_ADD T_SUB T_MUL T_QUO T_REM T_LPAREN T_RPAREN T_ASSIGN
%token T_LBRACK T_RBRACK T_LBRACE T_RBRACE T_COMMA T_PERIOD T_COLON T_ARROW
%token T_NEWLINE T_EXIT
%token T_PRINT T_ECHO T_PRETTY
%token T_VAR_BOOL T_VAR_NUMBER T_VAR_STRING T_VAR_LIST T_VAR_DICT T_VAR_ANY T_NULL
%token T_DEL T_RETURN T_VOID T_DEFAULT T_BREAK
%token T_SYMBOL_TABLE T_FUNCTION_TABLE
%token T_TIMES_DO T_FOREACH T_AS T_END T_DEF T_IMPORT T_FROM T_BACKSLASH T_INFINITE
%token T_EQL T_NEQ T_GTR T_LSS T_GEQ T_LEQ
%token T_LAND T_LOR T_NOT
%token T_AND T_OR T_XOR T_TILDE T_SHL T_SHR
%token T_INC T_DEC
%left T_ADD T_SUB T_NOT T_TILDE T_VAR
%left T_MUL T_QUO
%right T_U_ADD T_U_SUB T_U_NOT T_U_TILDE

%type<expr> expr basic_lit ident binary_expr bool_expr unary_expr paren_expr incdec_expr
%type<expr> index_expr composite_lit key_value_expr
%type<expr> module_selector alias_expr
%type<expr> selector_expr call_expr decision_expr default_expr
%type<stmt> stmt assign_stmt print_stmt echo_stmt return_stmt expr_stmt decl_stmt del_stmt exit_stmt
%type<stmt> function_table_stmt
%type<stmt> block_stmt break_stmt
%type<spec> type_spec sub_type_spec pretty_spec import parent_dir_spec asterisk_spec
%type<spec> field_spec optional_field_spec field_list_spec optional_field_list_spec
%type<spec> decision_block
%type<decl> var_decl times_do_decl foreach_as_list_decl foreach_as_dict_decl func_decl
%type<expr_list> alias_expr_list expr_list key_value_list decision_expr_list
%type<stmt_list> stmt_list
%type<func_decl_com> func_type

%destructor {
    free($$);
} <sval>

%start meta_start

%%

meta_start:
    | START_PROGRAM parser {}
;

parser:
    | parser line {
        if (is_interactive) {
            compile_interactive();
#   if defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__)
            if (shell_indicator_block_counter > 0) {
                printf("%s ", __KAOS_SHELL_INDICATOR_BLOCK__);
            } else {
                printf("%s ", __KAOS_SHELL_INDICATOR__);
            }
#   endif
        }
    }
;

line: T_NEWLINE
    | import {
        File* file = _ast_root->files[_ast_root->file_count - 1];
        if (is_interactive && !interactively_importing)
            file = _ast_root->files[0];

        addSpec(file->imports, $1);
    }
    | stmt {
        File* file = _ast_root->files[_ast_root->file_count - 1];
        if (is_interactive && !interactively_importing)
            file = _ast_root->files[0];

        addStmtLine(file->stmt_list, $1);
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
    | index_expr {
        $$ = $1;
    }
    | composite_lit {
        $$ = $1;
    }
    | call_expr {
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
    | bool_expr {
        $$ = $1;
    }
;

bool_expr:
    expr T_EQL expr {
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
    T_INC expr {
        $$ = incDecExpr(INC_tok, $2, true, yylineno);
    }
    | expr T_INC {
        $$ = incDecExpr(INC_tok, $1, false, yylineno);
    }
    | T_DEC expr {
        $$ = incDecExpr(DEC_tok, $2, true, yylineno);
    }
    | expr T_DEC {
        $$ = incDecExpr(DEC_tok, $1, false, yylineno);
    }
;

alias_expr:
    ident {
        $$ = aliasExpr($1, NULL, yylineno);
    }
    | ident T_AS ident {
        $$ = aliasExpr($1, $3, yylineno);
    }
;

alias_expr_list:
    alias_expr {
        $$ = (struct ExprList*)calloc(1, sizeof(ExprList));
        $$->expr_count = 0;
        addExpr($$, $1);
    }
    | alias_expr T_COMMA alias_expr_list {
        $$ = $3;
        addExpr($$, $1);
    }
;

index_expr:
    expr T_LBRACK expr T_RBRACK {
        $$ = indexExpr($1, $3, yylineno);
    }
;

expr_list:
    expr {
        $$ = (struct ExprList*)calloc(1, sizeof(ExprList));
        $$->expr_count = 0;
        addExpr($$, $1);
    }
    | expr T_COMMA expr_list {
        $$ = $3;
        addExpr($$, $1);
    }
    | expr T_COMMA T_NEWLINE expr_list {
        $$ = $4;
        addExpr($$, $1);
    }
;

key_value_expr:
    basic_lit T_COLON expr {
        $$ = keyValueExpr($1, $3, yylineno);
    }
;

key_value_list:
    key_value_expr {
        $$ = (struct ExprList*)calloc(1, sizeof(ExprList));
        $$->expr_count = 0;
        addExpr($$, $1);
    }
    | key_value_expr T_COMMA key_value_list {
        $$ = $3;
        addExpr($$, $1);
    }
    | key_value_expr T_COMMA T_NEWLINE key_value_list {
        $$ = $4;
        addExpr($$, $1);
    }
;

composite_lit:
    T_LBRACK T_RBRACK {
        ExprList* expr_list = (struct ExprList*)calloc(1, sizeof(ExprList));
        expr_list->expr_count = 0;
        $$ = compositeLit(listType(yylineno), expr_list, yylineno);
    }
    | T_LBRACK expr_list T_RBRACK {
        $$ = compositeLit(listType(yylineno), $2, yylineno);
    }
    | T_LBRACK T_NEWLINE expr_list T_RBRACK {
        $$ = compositeLit(listType(yylineno), $3, yylineno);
    }
    | T_LBRACK expr_list T_NEWLINE T_RBRACK {
        $$ = compositeLit(listType(yylineno), $2, yylineno);
    }
    | T_LBRACK T_NEWLINE expr_list T_NEWLINE T_RBRACK {
        $$ = compositeLit(listType(yylineno), $3, yylineno);
    }
    | T_LBRACE T_RBRACE {
        ExprList* key_value_list = (struct ExprList*)calloc(1, sizeof(ExprList));
        key_value_list->expr_count = 0;
        $$ = compositeLit(dictType(yylineno), key_value_list, yylineno);
    }
    | T_LBRACE key_value_list T_RBRACE {
        $$ = compositeLit(dictType(yylineno), $2, yylineno);
    }
    | T_LBRACE T_NEWLINE key_value_list T_RBRACE {
        $$ = compositeLit(dictType(yylineno), $3, yylineno);
    }
    | T_LBRACE key_value_list T_NEWLINE T_RBRACE {
        $$ = compositeLit(dictType(yylineno), $2, yylineno);
    }
    | T_LBRACE T_NEWLINE key_value_list T_NEWLINE T_RBRACE {
        $$ = compositeLit(dictType(yylineno), $3, yylineno);
    }
;

selector_expr:
    ident T_PERIOD ident {
        $$ = selectorExpr($1, $3, yylineno);
    }
    | selector_expr T_PERIOD ident {
        $$ = selectorExpr($1, $3, yylineno);
    }
;

call_expr:
    ident T_LPAREN T_RPAREN {
        ExprList* expr_list = (struct ExprList*)calloc(1, sizeof(ExprList));
        expr_list->expr_count = 0;
        $$ = callExpr($1, expr_list, yylineno);
    }
    | ident T_LPAREN expr_list T_RPAREN {
        $$ = callExpr($1, $3, yylineno);
    }
    | selector_expr T_LPAREN T_RPAREN {
        ExprList* expr_list = (struct ExprList*)calloc(1, sizeof(ExprList));
        expr_list->expr_count = 0;
        $$ = callExpr($1, expr_list, yylineno);
    }
    | selector_expr T_LPAREN expr_list T_RPAREN {
        $$ = callExpr($1, $3, yylineno);
    }
;

decision_expr:
    bool_expr T_COLON call_expr {
        $$ = decisionExpr($1, exprStmt($3, yylineno), yylineno);
    }
    | bool_expr T_COLON return_stmt {
        $$ = decisionExpr($1, $3, yylineno);
    }
    | bool_expr T_COLON break_stmt {
        $$ = decisionExpr($1, $3, yylineno);
    }
;

default_expr:
    T_DEFAULT T_COLON call_expr {
        $$ = defaultExpr(exprStmt($3, yylineno), yylineno);
    }
    | T_DEFAULT T_COLON return_stmt {
        $$ = defaultExpr($3, yylineno);
    }
    | T_DEFAULT T_COLON break_stmt {
        $$ = defaultExpr($3, yylineno);
    }
;

decision_expr_list:
    decision_expr {
        $$ = (struct ExprList*)calloc(1, sizeof(ExprList));
        $$->expr_count = 0;
        addExpr($$, $1);
    }
    | default_expr {
        $$ = (struct ExprList*)calloc(1, sizeof(ExprList));
        $$->expr_count = 0;
        addExpr($$, $1);
    }
    | decision_expr T_COMMA decision_expr_list {
        $$ = $3;
        addExpr($$, $1);
    }
    | decision_expr T_COMMA T_NEWLINE decision_expr_list {
        $$ = $4;
        addExpr($$, $1);
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
    | exit_stmt T_NEWLINE {
        $$ = $1;
    }
    | function_table_stmt T_NEWLINE {
        $$ = $1;
    }
;

stmt_list:
    stmt {
        $$ = (struct StmtList*)calloc(1, sizeof(StmtList));
        $$->stmt_count = 0;
        addStmt($$, $1);
    }
    | {
        $$ = (struct StmtList*)calloc(1, sizeof(StmtList));
        $$->stmt_count = 0;
    }
    | stmt stmt_list {
        $$ = $2;
        addStmt($$, $1);
    }
    | T_NEWLINE stmt_list {
        $$ = $2;
    }
;

assign_stmt:
    expr T_ASSIGN expr {
        $$ = assignStmt($1, ASSIGN_tok, $3, yylineno);
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
    | times_do_decl {
        $$ = declStmt($1, yylineno);
    }
    | foreach_as_list_decl {
        $$ = declStmt($1, yylineno);
    }
    | foreach_as_dict_decl {
        $$ = declStmt($1, yylineno);
    }
    | func_decl {
        $$ = declStmt($1, yylineno);
    }
;

del_stmt:
    T_DEL ident {
        $$ = delStmt($2, yylineno);
    }
    | T_DEL index_expr {
        $$ = delStmt($2, yylineno);
    }
;

exit_stmt:
    T_EXIT {
        $$ = exitStmt(NULL, yylineno);
    }
    | T_EXIT expr {
        $$ = exitStmt($2, yylineno);
    }
;

function_table_stmt:
    T_FUNCTION_TABLE {
        $$ = functionTableStmt(yylineno);
    }
;

block_stmt:
    stmt_list T_END {
        $$ = blockStmt($1, yylineno);
    }
;

break_stmt:
    T_BREAK {
        $$ = breakStmt(yylineno);
    }
;

type_spec:
    T_VOID {
        $$ = typeSpec(K_BOOL, NULL, yylineno);
    }
    | T_VAR_BOOL {
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
    | T_VOID sub_type_spec {
        $$ = typeSpec(K_BOOL, $2, yylineno);
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

func_decl:
    func_type block_stmt {
        $$ = funcDecl($1->func_type, $1->ident, $2, NULL, yylineno);
        free($1);
    }
    | func_type block_stmt decision_block {
        $$ = funcDecl($1->func_type, $1->ident, $2, $3, yylineno);
        free($1);
    }
;

times_do_decl:
    expr T_TIMES_DO T_ARROW call_expr {
        $$ = timesDo($1, NULL, $4, yylineno);
    }
    | expr T_TIMES_DO ident T_ARROW call_expr {
        $$ = timesDo($1, $3, $5, yylineno);
    }
    | expr T_TIMES_DO T_AS ident T_ARROW call_expr {
        $$ = timesDo($1, $4, $6, yylineno);
    }
;

foreach_as_list_decl:
    T_FOREACH expr T_AS ident T_ARROW call_expr {
        $$ = foreachAsList($2, NULL, $4, $6, yylineno);
    }
    | T_FOREACH expr T_AS ident T_COMMA ident T_ARROW call_expr {
        $$ = foreachAsList($2, $4, $6, $8, yylineno);
    }
;

foreach_as_dict_decl:
    T_FOREACH expr T_AS ident T_COLON ident T_ARROW call_expr {
        $$ = foreachAsDict($2, NULL, $4, $6, $8, yylineno);
    }
    | T_FOREACH expr T_AS ident T_COMMA ident T_COLON ident T_ARROW call_expr {
        $$ = foreachAsDict($2, $4, $6, $8, $10, yylineno);
    }
;

import:
    T_IMPORT module_selector {
        $$ = importSpec($2, NULL, NULL, NULL, yylineno);
    }
    | T_IMPORT module_selector T_AS ident {
        $$ = importSpec($2, $4, NULL, NULL, yylineno);
    }
    | T_FROM module_selector T_IMPORT asterisk_spec {
        $$ = importSpec($2, NULL, NULL, $4, yylineno);
    }
    | T_FROM module_selector T_IMPORT alias_expr_list {
        $$ = importSpec($2, NULL, $4, NULL, yylineno);
    }
;

module_selector:
    ident {
        $$ = moduleSelector(NULL, $1, NULL, yylineno);
    }
    | ident T_PERIOD module_selector {
        $$ = moduleSelector(NULL, $1, $3, yylineno);
    }
    | ident T_QUO module_selector {
        $$ = moduleSelector(NULL, $1, $3, yylineno);
    }
    | ident T_BACKSLASH module_selector {
        $$ = moduleSelector(NULL, $1, $3, yylineno);
    }
    | parent_dir_spec {
        $$ = moduleSelector($1, NULL, NULL, yylineno);
    }
    | parent_dir_spec T_PERIOD module_selector {
        $$ = moduleSelector($1, NULL, $3, yylineno);
    }
    | parent_dir_spec T_QUO module_selector {
        $$ = moduleSelector($1, NULL, $3, yylineno);
    }
    | parent_dir_spec T_BACKSLASH module_selector {
        $$ = moduleSelector($1, NULL, $3, yylineno);
    }
    | parent_dir_spec module_selector {
        $$ = moduleSelector($1, NULL, $2, yylineno);
    }
;

parent_dir_spec:
    T_PERIOD T_PERIOD {
        $$ = parentDirSpec(yylineno);
    }
;

asterisk_spec:
    T_MUL {
        $$ = asteriskSpec(yylineno);
    }
;

field_spec:
    type_spec ident {
        $$ = fieldSpec($1, $2, yylineno);
    }
;

optional_field_spec:
    type_spec ident T_ASSIGN expr {
        $$ = optionalFieldSpec($1, $2, $4, yylineno);
    }
;

field_list_spec:
    field_spec {
        SpecList* spec_list = (struct SpecList*)calloc(1, sizeof(SpecList));
        spec_list->spec_count = 0;
        $$ = fieldListSpec(spec_list, yylineno);
        addSpec($$->v.field_list_spec->list, $1);
    }
    | field_spec T_COMMA field_list_spec {
        $$ = $3;
        addSpec($$->v.field_list_spec->list, $1);
    }
    | field_spec T_COMMA T_NEWLINE field_list_spec {
        $$ = $4;
        addSpec($$->v.field_list_spec->list, $1);
    }
    | field_spec T_COMMA optional_field_list_spec {
        $$ = $3;
        addSpec($$->v.field_list_spec->list, $1);
    }
    | field_spec T_COMMA T_NEWLINE optional_field_list_spec {
        $$ = $4;
        addSpec($$->v.field_list_spec->list, $1);
    }
;

optional_field_list_spec:
    optional_field_spec {
        SpecList* spec_list = (struct SpecList*)calloc(1, sizeof(SpecList));
        spec_list->spec_count = 0;
        $$ = fieldListSpec(spec_list, yylineno);
        addSpec($$->v.field_list_spec->list, $1);
    }
    | optional_field_spec T_COMMA optional_field_list_spec {
        $$ = $3;
        addSpec($$->v.field_list_spec->list, $1);
    }
    | optional_field_spec T_COMMA T_NEWLINE optional_field_list_spec {
        $$ = $4;
        addSpec($$->v.field_list_spec->list, $1);
    }
;

func_type:
    type_spec T_DEF ident T_LPAREN T_RPAREN T_NEWLINE {
        SpecList* spec_list = (struct SpecList*)calloc(1, sizeof(SpecList));
        spec_list->spec_count = 0;
        Spec* params = fieldListSpec(spec_list, yylineno);
        Spec* func_type = funcType(params, $1, yylineno);
        $$ = funcDeclCom(func_type, $3);
    }
    | type_spec T_DEF ident T_LPAREN field_list_spec T_RPAREN T_NEWLINE {
        Spec* func_type = funcType($5, $1, yylineno);
        $$ = funcDeclCom(func_type, $3);
    }
;

decision_block:
    T_LBRACE decision_expr_list T_RBRACE {
        $$ = decisionBlock($2, yylineno);
    }
    | T_LBRACE T_NEWLINE decision_expr_list T_RBRACE {
        $$ = decisionBlock($3, yylineno);
    }
    | T_LBRACE decision_expr_list T_NEWLINE T_RBRACE {
        $$ = decisionBlock($2, yylineno);
    }
    | T_LBRACE T_NEWLINE decision_expr_list T_NEWLINE T_RBRACE {
        $$ = decisionBlock($3, yylineno);
    }
;

%%

#ifndef CHAOS_COMPILER
int main(int argc, char** argv) {
    initParser(argc, argv);
}
#endif
