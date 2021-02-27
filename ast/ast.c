/*
 * Description: Abstract Syntax Tree module of the Chaos Programming Language's source
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

#include "ast.h"

unsigned long long ast_node_id_counter = 0;
bool enable_branch_out = false;
unsigned long long loops_inside_function_counter = 0;

ASTNode* addASTNodeBase(enum ASTNodeType node_type, int lineno, char *strings[], size_t strings_size, union Value value, enum ValueType value_type) {
    ASTNode* ast_node;
    ast_node = (struct ASTNode*)calloc(1, sizeof(ASTNode) + strings_size * sizeof *ast_node->strings);
    ast_node_id_counter++;

    ast_node->id = ast_node_id_counter;
    ast_node->node_type = node_type;
    ast_node->lineno = lineno;
    ast_node->value = value;
    ast_node->value_type = value_type;
    ast_node->module = malloc(1 + strlen(module_path_stack.arr[module_path_stack.size - 1]));
    strcpy(ast_node->module, module_path_stack.arr[module_path_stack.size - 1]);

    ast_node->transpiled = NULL;
    ast_node->is_transpiled = false;
    ast_node->dont_transpile = false;

    ast_node->strings_size = strings_size;
    for (size_t i = 0; i < ast_node->strings_size; ++i) {
        ast_node->strings[i] = strings[i];
    }

    if (debug_enabled)
        printf(
            "(Create)\tASTNode: {id: %llu, node_type: %s, module: %s, string_size: %zu, lineno: %d}\n",
            ast_node->id,
            getAstNodeTypeName(ast_node->node_type),
            ast_node->module,
            ast_node->strings_size,
            ast_node->lineno
        );

    return ast_node;
}

ASTNode* addASTNode(enum ASTNodeType node_type, int lineno, char *strings[], size_t strings_size) {
    union Value value;
    value.i = 0;
    return addASTNodeBase(node_type, lineno, strings, strings_size, value, V_VOID);
}

ASTNode* addASTNodeBool(enum ASTNodeType node_type, int lineno, char *strings[], size_t strings_size, bool b, ASTNode* node) {
    union Value value;
    value.b = b;
    ASTNode* ast_node = addASTNodeBase(node_type, lineno, strings, strings_size, value, V_BOOL);
    ast_node->right = node;
    return ast_node;
}

ASTNode* addASTNodeInt(enum ASTNodeType node_type, int lineno, char *strings[], size_t strings_size, long long i, ASTNode* node) {
    union Value value;
    value.i = i;
    ASTNode* ast_node = addASTNodeBase(node_type, lineno, strings, strings_size, value, V_INT);
    ast_node->right = node;
    return ast_node;
}

ASTNode* addASTNodeFloat(enum ASTNodeType node_type, int lineno, char *strings[], size_t strings_size, long double f, ASTNode* node) {
    union Value value;
    value.f = f;
    ASTNode* ast_node = addASTNodeBase(node_type, lineno, strings, strings_size, value, V_FLOAT);
    ast_node->right = node;
    return ast_node;
}

ASTNode* addASTNodeString(enum ASTNodeType node_type, int lineno, char *strings[], size_t strings_size, char *s, ASTNode* node) {
    union Value value;
    value.s = malloc(1 + strlen(s));
    strcpy(value.s, s);
    free(s);
    ASTNode* ast_node = addASTNodeBase(node_type, lineno, strings, strings_size, value, V_STRING);
    ast_node->right = node;
    return ast_node;
}

ASTNode* addASTNodeBranch(enum ASTNodeType node_type, int lineno, ASTNode* l_node, ASTNode* r_node) {
    size_t strings_size = 0;
    union Value value;
    value.i = 0;
    ASTNode* ast_node = addASTNodeBase(node_type, lineno, NULL, strings_size, value, V_VOID);
    ast_node->left = l_node;
    ast_node->right = r_node;
    return ast_node;
}

ASTNode* addASTNodeAssign(enum ASTNodeType node_type, int lineno, char *strings[], size_t strings_size, ASTNode* node) {
    ASTNode* ast_node = addASTNodeBase(node_type, lineno, strings, strings_size, node->value, V_VOID);
    ast_node->right = node;
    return ast_node;
}

ASTNode* addASTNodeFull(enum ASTNodeType node_type, int lineno, char *strings[], size_t strings_size, ASTNode* l_node, ASTNode* r_node) {
    union Value value;
    value.i = 0;
    ASTNode* ast_node = addASTNodeBase(node_type, lineno, strings, strings_size, value, V_VOID);
    ast_node->left = l_node;
    ast_node->right = r_node;
    return ast_node;
}

void ASTNodeNext(ASTNode* ast_node) {
    if (ast_node_cursor != NULL) {
        if (enable_branch_out) {
            ast_node_cursor->child = ast_node;
            enable_branch_out = false;
        } else {
            ast_node_cursor->next = ast_node;
        }
    } else {
        ast_root_node = ast_node;
    }
    ast_node_cursor = ast_node;
}

void ASTBranchOut() {
    ast_node_cursor_backup = ast_node_cursor;
    enable_branch_out = true;
}

void ASTMergeBack() {
    if (loops_inside_function_counter > 0)
        loops_inside_function_counter--;
    if (ast_node_cursor_backup != NULL && loops_inside_function_counter == 0) {
        ast_node_cursor = ast_node_cursor_backup;
        ast_node_cursor_backup = NULL;
    }
}

ASTNode* free_node(ASTNode* ast_node) {
    if (ast_node == NULL) {
        return ast_node;
    }

    free_node(ast_node->depend);
    free_node(ast_node->right);
    free_node(ast_node->left);
    free_node(ast_node->child);

    if (debug_enabled)
        printf(
            "(Free)\t\tASTNode: {id: %llu, node_type: %s, module: %s, string_size: %zu}\n",
            ast_node->id,
            getAstNodeTypeName(ast_node->node_type),
            ast_node->module,
            ast_node->strings_size
        );

    ASTNode* next_node = ast_node->next;

    for (size_t i = 0; i < ast_node->strings_size; ++i) {
        free(ast_node->strings[i]);
    }
    if (ast_node->value_type == V_STRING) free(ast_node->value.s);
    free(ast_node->module);
    free(ast_node->transpiled);
    free(ast_node);
    return free_node(next_node);
}

void setASTNodeTranspiled(ASTNode* ast_node, char* transpiled) {
    ast_node->transpiled = transpiled;
    ast_node->is_transpiled = true;
}

char *ast_node_type_names[] = {
    "AST_STEP",
    "AST_VAR_CREATE_BOOL",
    "AST_VAR_CREATE_BOOL_VAR",
    "AST_VAR_CREATE_BOOL_VAR_EL",
    "AST_VAR_CREATE_BOOL_FUNC_RETURN",
    "AST_VAR_CREATE_NUMBER",
    "AST_VAR_CREATE_NUMBER_VAR",
    "AST_VAR_CREATE_NUMBER_VAR_EL",
    "AST_VAR_CREATE_NUMBER_FUNC_RETURN",
    "AST_VAR_CREATE_STRING",
    "AST_VAR_CREATE_STRING_VAR",
    "AST_VAR_CREATE_STRING_VAR_EL",
    "AST_VAR_CREATE_STRING_FUNC_RETURN",
    "AST_VAR_CREATE_ANY_BOOL",
    "AST_VAR_CREATE_ANY_NUMBER",
    "AST_VAR_CREATE_ANY_STRING",
    "AST_VAR_CREATE_ANY_VAR",
    "AST_VAR_CREATE_ANY_VAR_EL",
    "AST_VAR_CREATE_ANY_FUNC_RETURN",
    "AST_VAR_CREATE_LIST",
    "AST_VAR_CREATE_LIST_VAR",
    "AST_VAR_CREATE_LIST_FUNC_RETURN",
    "AST_VAR_CREATE_DICT",
    "AST_VAR_CREATE_DICT_VAR",
    "AST_VAR_CREATE_DICT_FUNC_RETURN",
    "AST_VAR_CREATE_BOOL_LIST",
    "AST_VAR_CREATE_BOOL_LIST_VAR",
    "AST_VAR_CREATE_BOOL_LIST_FUNC_RETURN",
    "AST_VAR_CREATE_BOOL_DICT",
    "AST_VAR_CREATE_BOOL_DICT_VAR",
    "AST_VAR_CREATE_BOOL_DICT_FUNC_RETURN",
    "AST_VAR_CREATE_NUMBER_LIST",
    "AST_VAR_CREATE_NUMBER_LIST_VAR",
    "AST_VAR_CREATE_NUMBER_LIST_FUNC_RETURN",
    "AST_VAR_CREATE_NUMBER_DICT",
    "AST_VAR_CREATE_NUMBER_DICT_VAR",
    "AST_VAR_CREATE_NUMBER_DICT_FUNC_RETURN",
    "AST_VAR_CREATE_STRING_LIST",
    "AST_VAR_CREATE_STRING_LIST_VAR",
    "AST_VAR_CREATE_STRING_LIST_FUNC_RETURN",
    "AST_VAR_CREATE_STRING_DICT",
    "AST_VAR_CREATE_STRING_DICT_VAR",
    "AST_VAR_CREATE_STRING_DICT_FUNC_RETURN",
    "AST_VAR_UPDATE_BOOL",
    "AST_VAR_UPDATE_NUMBER",
    "AST_VAR_UPDATE_STRING",
    "AST_VAR_UPDATE_LIST",
    "AST_VAR_UPDATE_DICT",
    "AST_VAR_UPDATE_VAR",
    "AST_VAR_UPDATE_VAR_EL",
    "AST_VAR_UPDATE_FUNC_RETURN",
    "AST_RETURN_VAR",
    "AST_PRINT_COMPLEX_EL",
    "AST_COMPLEX_EL_UPDATE_BOOL",
    "AST_COMPLEX_EL_UPDATE_NUMBER",
    "AST_COMPLEX_EL_UPDATE_STRING",
    "AST_COMPLEX_EL_UPDATE_LIST",
    "AST_COMPLEX_EL_UPDATE_DICT",
    "AST_COMPLEX_EL_UPDATE_VAR",
    "AST_COMPLEX_EL_UPDATE_VAR_EL",
    "AST_COMPLEX_EL_UPDATE_FUNC_RETURN",
    "AST_PRINT_VAR",
    "AST_PRINT_VAR_EL",
    "AST_PRINT_EXPRESSION",
    "AST_PRINT_MIXED_EXPRESSION",
    "AST_PRINT_STRING",
    "AST_PRINT_INTERACTIVE_VAR",
    "AST_PRINT_INTERACTIVE_EXPRESSION",
    "AST_PRINT_INTERACTIVE_MIXED_EXPRESSION",
    "AST_ECHO_VAR",
    "AST_ECHO_VAR_EL",
    "AST_ECHO_EXPRESSION",
    "AST_ECHO_MIXED_EXPRESSION",
    "AST_ECHO_STRING",
    "AST_PRETTY_PRINT_VAR",
    "AST_PRETTY_PRINT_VAR_EL",
    "AST_PRETTY_ECHO_VAR",
    "AST_PRETTY_ECHO_VAR_EL",
    "AST_PARENTHESIS",
    "AST_EXPRESSION_VALUE",
    "AST_EXPRESSION_PLUS",
    "AST_EXPRESSION_MINUS",
    "AST_EXPRESSION_MULTIPLY",
    "AST_EXPRESSION_BITWISE_AND",
    "AST_EXPRESSION_BITWISE_OR",
    "AST_EXPRESSION_BITWISE_XOR",
    "AST_EXPRESSION_BITWISE_NOT",
    "AST_EXPRESSION_BITWISE_LEFT_SHIFT",
    "AST_EXPRESSION_BITWISE_RIGHT_SHIFT",
    "AST_VAR_EXPRESSION_VALUE",
    "AST_VAR_EXPRESSION_INCREMENT",
    "AST_VAR_EXPRESSION_DECREMENT",
    "AST_VAR_EXPRESSION_INCREMENT_ASSIGN",
    "AST_VAR_EXPRESSION_ASSIGN_INCREMENT",
    "AST_MIXED_EXPRESSION_VALUE",
    "AST_MIXED_EXPRESSION_PLUS",
    "AST_MIXED_EXPRESSION_MINUS",
    "AST_MIXED_EXPRESSION_MULTIPLY",
    "AST_MIXED_EXPRESSION_DIVIDE",
    "AST_VAR_MIXED_EXPRESSION_VALUE",
    "AST_BOOLEAN_EXPRESSION_VALUE",
    "AST_BOOLEAN_EXPRESSION_REL_EQUAL",
    "AST_BOOLEAN_EXPRESSION_REL_NOT_EQUAL",
    "AST_BOOLEAN_EXPRESSION_REL_GREAT",
    "AST_BOOLEAN_EXPRESSION_REL_SMALL",
    "AST_BOOLEAN_EXPRESSION_REL_GREAT_EQUAL",
    "AST_BOOLEAN_EXPRESSION_REL_SMALL_EQUAL",
    "AST_BOOLEAN_EXPRESSION_LOGIC_AND",
    "AST_BOOLEAN_EXPRESSION_LOGIC_OR",
    "AST_BOOLEAN_EXPRESSION_LOGIC_NOT",
    "AST_BOOLEAN_EXPRESSION_REL_EQUAL_MIXED",
    "AST_BOOLEAN_EXPRESSION_REL_NOT_EQUAL_MIXED",
    "AST_BOOLEAN_EXPRESSION_REL_GREAT_MIXED",
    "AST_BOOLEAN_EXPRESSION_REL_SMALL_MIXED",
    "AST_BOOLEAN_EXPRESSION_REL_GREAT_EQUAL_MIXED",
    "AST_BOOLEAN_EXPRESSION_REL_SMALL_EQUAL_MIXED",
    "AST_BOOLEAN_EXPRESSION_LOGIC_AND_MIXED",
    "AST_BOOLEAN_EXPRESSION_LOGIC_OR_MIXED",
    "AST_BOOLEAN_EXPRESSION_LOGIC_NOT_MIXED",
    "AST_BOOLEAN_EXPRESSION_REL_EQUAL_MIXED_BOOLEAN",
    "AST_BOOLEAN_EXPRESSION_REL_NOT_EQUAL_MIXED_BOOLEAN",
    "AST_BOOLEAN_EXPRESSION_REL_GREAT_MIXED_BOOLEAN",
    "AST_BOOLEAN_EXPRESSION_REL_SMALL_MIXED_BOOLEAN",
    "AST_BOOLEAN_EXPRESSION_REL_GREAT_EQUAL_MIXED_BOOLEAN",
    "AST_BOOLEAN_EXPRESSION_REL_SMALL_EQUAL_MIXED_BOOLEAN",
    "AST_BOOLEAN_EXPRESSION_LOGIC_AND_MIXED_BOOLEAN",
    "AST_BOOLEAN_EXPRESSION_LOGIC_OR_MIXED_BOOLEAN",
    "AST_BOOLEAN_EXPRESSION_REL_EQUAL_BOOLEAN_MIXED",
    "AST_BOOLEAN_EXPRESSION_REL_NOT_EQUAL_BOOLEAN_MIXED",
    "AST_BOOLEAN_EXPRESSION_REL_GREAT_BOOLEAN_MIXED",
    "AST_BOOLEAN_EXPRESSION_REL_SMALL_BOOLEAN_MIXED",
    "AST_BOOLEAN_EXPRESSION_REL_GREAT_EQUAL_BOOLEAN_MIXED",
    "AST_BOOLEAN_EXPRESSION_REL_SMALL_EQUAL_BOOLEAN_MIXED",
    "AST_BOOLEAN_EXPRESSION_LOGIC_AND_BOOLEAN_MIXED",
    "AST_BOOLEAN_EXPRESSION_LOGIC_OR_BOOLEAN_MIXED",
    "AST_BOOLEAN_EXPRESSION_REL_EQUAL_EXP",
    "AST_BOOLEAN_EXPRESSION_REL_NOT_EQUAL_EXP",
    "AST_BOOLEAN_EXPRESSION_REL_GREAT_EXP",
    "AST_BOOLEAN_EXPRESSION_REL_SMALL_EXP",
    "AST_BOOLEAN_EXPRESSION_REL_GREAT_EQUAL_EXP",
    "AST_BOOLEAN_EXPRESSION_REL_SMALL_EQUAL_EXP",
    "AST_BOOLEAN_EXPRESSION_LOGIC_AND_EXP",
    "AST_BOOLEAN_EXPRESSION_LOGIC_OR_EXP",
    "AST_BOOLEAN_EXPRESSION_LOGIC_NOT_EXP",
    "AST_BOOLEAN_EXPRESSION_REL_EQUAL_EXP_BOOLEAN",
    "AST_BOOLEAN_EXPRESSION_REL_NOT_EQUAL_EXP_BOOLEAN",
    "AST_BOOLEAN_EXPRESSION_REL_GREAT_EXP_BOOLEAN",
    "AST_BOOLEAN_EXPRESSION_REL_SMALL_EXP_BOOLEAN",
    "AST_BOOLEAN_EXPRESSION_REL_GREAT_EQUAL_EXP_BOOLEAN",
    "AST_BOOLEAN_EXPRESSION_REL_SMALL_EQUAL_EXP_BOOLEAN",
    "AST_BOOLEAN_EXPRESSION_LOGIC_AND_EXP_BOOLEAN",
    "AST_BOOLEAN_EXPRESSION_LOGIC_OR_EXP_BOOLEAN",
    "AST_BOOLEAN_EXPRESSION_REL_EQUAL_BOOLEAN_EXP",
    "AST_BOOLEAN_EXPRESSION_REL_NOT_EQUAL_BOOLEAN_EXP",
    "AST_BOOLEAN_EXPRESSION_REL_GREAT_BOOLEAN_EXP",
    "AST_BOOLEAN_EXPRESSION_REL_SMALL_BOOLEAN_EXP",
    "AST_BOOLEAN_EXPRESSION_REL_GREAT_EQUAL_BOOLEAN_EXP",
    "AST_BOOLEAN_EXPRESSION_REL_SMALL_EQUAL_BOOLEAN_EXP",
    "AST_BOOLEAN_EXPRESSION_LOGIC_AND_BOOLEAN_EXP",
    "AST_BOOLEAN_EXPRESSION_LOGIC_OR_BOOLEAN_EXP",
    "AST_BOOLEAN_EXPRESSION_REL_EQUAL_MIXED_EXP",
    "AST_BOOLEAN_EXPRESSION_REL_NOT_EQUAL_MIXED_EXP",
    "AST_BOOLEAN_EXPRESSION_REL_GREAT_MIXED_EXP",
    "AST_BOOLEAN_EXPRESSION_REL_SMALL_MIXED_EXP",
    "AST_BOOLEAN_EXPRESSION_REL_GREAT_EQUAL_MIXED_EXP",
    "AST_BOOLEAN_EXPRESSION_REL_SMALL_EQUAL_MIXED_EXP",
    "AST_BOOLEAN_EXPRESSION_LOGIC_AND_MIXED_EXP",
    "AST_BOOLEAN_EXPRESSION_LOGIC_OR_MIXED_EXP",
    "AST_BOOLEAN_EXPRESSION_REL_EQUAL_EXP_MIXED",
    "AST_BOOLEAN_EXPRESSION_REL_NOT_EQUAL_EXP_MIXED",
    "AST_BOOLEAN_EXPRESSION_REL_GREAT_EXP_MIXED",
    "AST_BOOLEAN_EXPRESSION_REL_SMALL_EXP_MIXED",
    "AST_BOOLEAN_EXPRESSION_REL_GREAT_EQUAL_EXP_MIXED",
    "AST_BOOLEAN_EXPRESSION_REL_SMALL_EQUAL_EXP_MIXED",
    "AST_BOOLEAN_EXPRESSION_LOGIC_AND_EXP_MIXED",
    "AST_BOOLEAN_EXPRESSION_LOGIC_OR_EXP_MIXED",
    "AST_VAR_BOOLEAN_EXPRESSION_VALUE",
    "AST_BOOLEAN_EXPRESSION_REL_EQUAL_UNKNOWN",
    "AST_BOOLEAN_EXPRESSION_REL_NOT_EQUAL_UNKNOWN",
    "AST_BOOLEAN_EXPRESSION_REL_GREAT_UNKNOWN",
    "AST_BOOLEAN_EXPRESSION_REL_SMALL_UNKNOWN",
    "AST_BOOLEAN_EXPRESSION_REL_GREAT_EQUAL_UNKNOWN",
    "AST_BOOLEAN_EXPRESSION_REL_SMALL_EQUAL_UNKNOWN",
    "AST_DELETE_VAR",
    "AST_DELETE_VAR_EL",
    "AST_PRINT_SYMBOL_TABLE",
    "AST_LIST_START",
    "AST_LIST_ADD_VAR",
    "AST_LIST_ADD_VAR_EL",
    "AST_LIST_NESTED_FINISH",
    "AST_DICT_START",
    "AST_DICT_ADD_VAR",
    "AST_DICT_ADD_VAR_EL",
    "AST_DICT_NESTED_FINISH",
    "AST_POP_NESTED_COMPLEX_STACK",
    "AST_LEFT_RIGHT_BRACKET_EXPRESSION",
    "AST_LEFT_RIGHT_BRACKET_MINUS_EXPRESSION",
    "AST_LEFT_RIGHT_BRACKET_STRING",
    "AST_LEFT_RIGHT_BRACKET_VAR",
    "AST_LEFT_RIGHT_BRACKET_VAR_MINUS",
    "AST_BUILD_COMPLEX_VARIABLE",
    "AST_EXIT_SUCCESS",
    "AST_EXIT_EXPRESSION",
    "AST_EXIT_VAR",
    "AST_START_TIMES_DO",
    "AST_START_TIMES_DO_INFINITE",
    "AST_START_TIMES_DO_VAR",
    "AST_START_FOREACH",
    "AST_START_FOREACH_DICT",
    "AST_END",
    "AST_PRINT_FUNCTION_TABLE",
    "AST_FUNCTION_PARAMETERS_START",
    "AST_FUNCTION_STEP",
    "AST_FUNCTION_PARAMETER_BOOL",
    "AST_FUNCTION_PARAMETER_NUMBER",
    "AST_FUNCTION_PARAMETER_STRING",
    "AST_FUNCTION_PARAMETER_LIST",
    "AST_FUNCTION_PARAMETER_BOOL_LIST",
    "AST_FUNCTION_PARAMETER_NUMBER_LIST",
    "AST_FUNCTION_PARAMETER_STRING_LIST",
    "AST_FUNCTION_PARAMETER_DICT",
    "AST_FUNCTION_PARAMETER_BOOL_DICT",
    "AST_FUNCTION_PARAMETER_NUMBER_DICT",
    "AST_FUNCTION_PARAMETER_STRING_DICT",
    "AST_OPTIONAL_FUNCTION_PARAMETER_BOOL",
    "AST_OPTIONAL_FUNCTION_PARAMETER_NUMBER",
    "AST_OPTIONAL_FUNCTION_PARAMETER_STRING",
    "AST_OPTIONAL_FUNCTION_PARAMETER_LIST",
    "AST_OPTIONAL_FUNCTION_PARAMETER_BOOL_LIST",
    "AST_OPTIONAL_FUNCTION_PARAMETER_NUMBER_LIST",
    "AST_OPTIONAL_FUNCTION_PARAMETER_STRING_LIST",
    "AST_OPTIONAL_FUNCTION_PARAMETER_DICT",
    "AST_OPTIONAL_FUNCTION_PARAMETER_BOOL_DICT",
    "AST_OPTIONAL_FUNCTION_PARAMETER_NUMBER_DICT",
    "AST_OPTIONAL_FUNCTION_PARAMETER_STRING_DICT",
    "AST_FUNCTION_CALL_PARAMETERS_START",
    "AST_FUNCTION_CALL_PARAMETER_BOOL",
    "AST_FUNCTION_CALL_PARAMETER_NUMBER",
    "AST_FUNCTION_CALL_PARAMETER_STRING",
    "AST_FUNCTION_CALL_PARAMETER_VAR",
    "AST_FUNCTION_CALL_PARAMETER_LIST",
    "AST_FUNCTION_CALL_PARAMETER_DICT",
    "AST_DEFINE_FUNCTION_BOOL",
    "AST_DEFINE_FUNCTION_NUMBER",
    "AST_DEFINE_FUNCTION_STRING",
    "AST_DEFINE_FUNCTION_ANY",
    "AST_DEFINE_FUNCTION_LIST",
    "AST_DEFINE_FUNCTION_DICT",
    "AST_DEFINE_FUNCTION_BOOL_LIST",
    "AST_DEFINE_FUNCTION_BOOL_DICT",
    "AST_DEFINE_FUNCTION_NUMBER_LIST",
    "AST_DEFINE_FUNCTION_NUMBER_DICT",
    "AST_DEFINE_FUNCTION_STRING_LIST",
    "AST_DEFINE_FUNCTION_STRING_DICT",
    "AST_DEFINE_FUNCTION_VOID",
    "AST_PRINT_FUNCTION_RETURN",
    "AST_ECHO_FUNCTION_RETURN",
    "AST_PRETTY_PRINT_FUNCTION_RETURN",
    "AST_PRETTY_ECHO_FUNCTION_RETURN",
    "AST_FUNCTION_RETURN",
    "AST_ADD_FUNCTION_NAME",
    "AST_APPEND_MODULE",
    "AST_PREPEND_MODULE",
    "AST_MODULE_IMPORT",
    "AST_MODULE_IMPORT_AS",
    "AST_MODULE_IMPORT_PARTIAL",
    "AST_NESTED_COMPLEX_TRANSITION",
    "AST_DECISION_DEFINE",
    "AST_DECISION_MAKE_BOOLEAN",
    "AST_DECISION_MAKE_BOOLEAN_BREAK",
    "AST_DECISION_MAKE_BOOLEAN_CONTINUE",
    "AST_DECISION_MAKE_BOOLEAN_RETURN",
    "AST_DECISION_MAKE_DEFAULT",
    "AST_DECISION_MAKE_DEFAULT_BREAK",
    "AST_DECISION_MAKE_DEFAULT_CONTINUE",
    "AST_DECISION_MAKE_DEFAULT_RETURN",
    "AST_JSON_PARSER",
};

char* getAstNodeTypeName(unsigned i) {
    return ast_node_type_names[i];
}


// NEW AST START

AST* ast(int lineno)
{
    AST* ast = (struct AST*)calloc(1, sizeof(AST));
    ast->lineno = lineno;
    return ast;
}


// Expr

Expr* buildExpr(enum ExprKind kind, int lineno)
{
    Expr* expr = (struct Expr*)calloc(1, sizeof(Expr));
    expr->ast = ast(lineno);
    expr->kind = kind;
    return expr;
}

Expr* basicLitBool(bool b, int lineno)
{
    union Value value;
    value.b = b;
    BasicLit* basic_lit = (struct BasicLit*)calloc(1, sizeof(BasicLit));
    basic_lit->value_type = V_BOOL;
    basic_lit->value = value;
    Expr* expr = buildExpr(BasicLit_kind, lineno);
    expr->v.basic_lit = basic_lit;
    return expr;
}

Expr* basicLitInt(long long i, int lineno)
{
    union Value value;
    value.i = i;
    BasicLit* basic_lit = (struct BasicLit*)calloc(1, sizeof(BasicLit));
    basic_lit->value_type = V_INT;
    basic_lit->value = value;
    Expr* expr = buildExpr(BasicLit_kind, lineno);
    expr->v.basic_lit = basic_lit;
    return expr;
}

Expr* basicLitFloat(long double f, int lineno)
{
    union Value value;
    value.f = f;
    BasicLit* basic_lit = (struct BasicLit*)calloc(1, sizeof(BasicLit));
    basic_lit->value_type = V_FLOAT;
    basic_lit->value = value;
    Expr* expr = buildExpr(BasicLit_kind, lineno);
    expr->v.basic_lit = basic_lit;
    return expr;
}

Expr* basicLitString(char *s, int lineno)
{
    union Value value;
    value.s = malloc(1 + strlen(s));
    strcpy(value.s, s);
    free(s);
    BasicLit* basic_lit = (struct BasicLit*)calloc(1, sizeof(BasicLit));
    basic_lit->value_type = V_STRING;
    basic_lit->value = value;
    Expr* expr = buildExpr(BasicLit_kind, lineno);
    expr->v.basic_lit = basic_lit;
    return expr;
}

Expr* ident(char *s, int lineno)
{
    Ident* ident = (struct Ident*)calloc(1, sizeof(Ident));
    ident->name = malloc(1 + strlen(s));
    strcpy(ident->name, s);
    free(s);
    Expr* expr = buildExpr(Ident_kind, lineno);
    expr->v.ident = ident;
    return expr;
}

Expr* binaryExpr(Expr* x, enum Token op, Expr* y, int lineno)
{
    BinaryExpr* binary_expr = (struct BinaryExpr*)calloc(1, sizeof(BinaryExpr));
    binary_expr->x = x;
    binary_expr->op = op;
    binary_expr->y = y;
    Expr* expr = buildExpr(BinaryExpr_kind, lineno);
    expr->v.binary_expr = binary_expr;
    return expr;
}

Expr* unaryExpr(enum Token op, Expr* x, int lineno)
{
    UnaryExpr* unary_expr = (struct UnaryExpr*)calloc(1, sizeof(UnaryExpr));
    unary_expr->op = op;
    unary_expr->x = x;
    Expr* expr = buildExpr(UnaryExpr_kind, lineno);
    expr->v.unary_expr = unary_expr;
    return expr;
}

Expr* parenExpr(Expr* x, int lineno)
{
    ParenExpr* paren_expr = (struct ParenExpr*)calloc(1, sizeof(ParenExpr));
    paren_expr->x = x;
    Expr* expr = buildExpr(ParenExpr_kind, lineno);
    expr->v.paren_expr = paren_expr;
    return expr;
}

Expr* incDecExpr(enum Token op, Expr* ident, bool first, int lineno)
{
    IncDecExpr* incdec_expr = (struct IncDecExpr*)calloc(1, sizeof(IncDecExpr));
    incdec_expr->op = op;
    incdec_expr->ident = ident;
    incdec_expr->first = first;
    Expr* expr = buildExpr(IncDecExpr_kind, lineno);
    expr->v.incdec_expr = incdec_expr;
    return expr;
}

Expr* moduleSelector(Spec* parent_dir_spec, Expr* x, Expr* sel, int lineno)
{
    ModuleSelector* module_selector = (struct ModuleSelector*)calloc(1, sizeof(ModuleSelector));
    module_selector->parent_dir_spec = parent_dir_spec;
    module_selector->x = x;
    module_selector->sel = sel;
    Expr* expr = buildExpr(ModuleSelector_kind, lineno);
    expr->v.module_selector = module_selector;
    return expr;
}

Expr* aliasExpr(Expr* name, Expr* asname, int lineno)
{
    AliasExpr* alias_expr = (struct AliasExpr*)calloc(1, sizeof(AliasExpr));
    alias_expr->name = name;
    alias_expr->asname = asname;
    Expr* expr = buildExpr(AliasExpr_kind, lineno);
    expr->v.alias_expr = alias_expr;
    return expr;
}

Expr* indexExpr(Expr* x, Expr* index, int lineno)
{
    IndexExpr* index_expr = (struct IndexExpr*)calloc(1, sizeof(IndexExpr));
    index_expr->x = x;
    index_expr->index = index;
    Expr* expr = buildExpr(IndexExpr_kind, lineno);
    expr->v.index_expr = index_expr;
    return expr;
}

Expr* compositeLit(Spec* type, ExprList* elts, int lineno)
{
    CompositeLit* composite_lit = (struct CompositeLit*)calloc(1, sizeof(CompositeLit));
    composite_lit->type = type;
    composite_lit->elts = elts;
    Expr* expr = buildExpr(CompositeLit_kind, lineno);
    expr->v.composite_lit = composite_lit;
    return expr;
}

Expr* keyValueExpr(Expr* key, Expr* value, int lineno)
{
    KeyValueExpr* key_value_expr = (struct KeyValueExpr*)calloc(1, sizeof(KeyValueExpr));
    key_value_expr->key = key;
    key_value_expr->value = value;
    Expr* expr = buildExpr(KeyValueExpr_kind, lineno);
    expr->v.key_value_expr = key_value_expr;
    return expr;
}

Expr* selectorExpr(Expr* x, Expr* sel, int lineno)
{
    SelectorExpr* selector_expr = (struct SelectorExpr*)calloc(1, sizeof(SelectorExpr));
    selector_expr->x = x;
    selector_expr->sel = sel;
    Expr* expr = buildExpr(SelectorExpr_kind, lineno);
    expr->v.selector_expr = selector_expr;
    return expr;
}

Expr* callExpr(Expr* fun, ExprList* args, int lineno)
{
    CallExpr* call_expr = (struct CallExpr*)calloc(1, sizeof(CallExpr));
    call_expr->fun = fun;
    call_expr->args = args;
    Expr* expr = buildExpr(CallExpr_kind, lineno);
    expr->v.call_expr = call_expr;
    return expr;
}

Expr* decisionExpr(Expr* bool_expr, Stmt* outcome, int lineno)
{
    DecisionExpr* decision_expr = (struct DecisionExpr*)calloc(1, sizeof(DecisionExpr));
    decision_expr->bool_expr = bool_expr;
    decision_expr->outcome = outcome;
    Expr* expr = buildExpr(DecisionExpr_kind, lineno);
    expr->v.decision_expr = decision_expr;
    return expr;
}

Expr* defaultExpr(Stmt* outcome, int lineno)
{
    DefaultExpr* default_expr = (struct DefaultExpr*)calloc(1, sizeof(DefaultExpr));
    default_expr->outcome = outcome;
    Expr* expr = buildExpr(DefaultExpr_kind, lineno);
    expr->v.default_expr = default_expr;
    return expr;
}


// Stmt

Stmt* buildStmt(enum StmtKind kind, int lineno)
{
    Stmt* stmt = (struct Stmt*)calloc(1, sizeof(Stmt));
    stmt->ast = ast(lineno);
    stmt->kind = kind;
    return stmt;
}

Stmt* assignStmt(Expr* x, enum Token tok, Expr* y, int lineno)
{
    AssignStmt* assign_stmt = (struct AssignStmt*)calloc(1, sizeof(AssignStmt));
    assign_stmt->x = x;
    assign_stmt->tok = tok;
    assign_stmt->y = y;
    Stmt* stmt = buildStmt(AssignStmt_kind, lineno);
    stmt->v.assign_stmt = assign_stmt;
    return stmt;
}

Stmt* returnStmt(Expr* x, int lineno)
{
    ReturnStmt* return_stmt = (struct ReturnStmt*)calloc(1, sizeof(ReturnStmt));
    return_stmt->x = x;
    Stmt* stmt = buildStmt(ReturnStmt_kind, lineno);
    stmt->v.return_stmt = return_stmt;
    return stmt;
}

Stmt* printStmt(Spec* mod, Expr* x, int lineno)
{
    PrintStmt* print_stmt = (struct PrintStmt*)calloc(1, sizeof(PrintStmt));
    print_stmt->mod = mod;
    print_stmt->x = x;
    Stmt* stmt = buildStmt(PrintStmt_kind, lineno);
    stmt->v.print_stmt = print_stmt;
    return stmt;
}

Stmt* echoStmt(Spec* mod, Expr* x, int lineno)
{
    EchoStmt* echo_stmt = (struct EchoStmt*)calloc(1, sizeof(EchoStmt));
    echo_stmt->mod = mod;
    echo_stmt->x = x;
    Stmt* stmt = buildStmt(EchoStmt_kind, lineno);
    stmt->v.echo_stmt = echo_stmt;
    return stmt;
}

Stmt* exprStmt(Expr* x, int lineno)
{
    ExprStmt* expr_stmt = (struct ExprStmt*)calloc(1, sizeof(ExprStmt));
    expr_stmt->x = x;
    Stmt* stmt = buildStmt(ExprStmt_kind, lineno);
    stmt->v.expr_stmt = expr_stmt;
    return stmt;
}

Stmt* declStmt(Decl* decl, int lineno)
{
    DeclStmt* decl_stmt = (struct DeclStmt*)calloc(1, sizeof(DeclStmt));
    decl_stmt->decl = decl;
    Stmt* stmt = buildStmt(DeclStmt_kind, lineno);
    stmt->v.decl_stmt = decl_stmt;
    return stmt;
}

Stmt* delStmt(Expr* ident, int lineno)
{
    DelStmt* del_stmt = (struct DelStmt*)calloc(1, sizeof(DelStmt));
    del_stmt->ident = ident;
    Stmt* stmt = buildStmt(DelStmt_kind, lineno);
    stmt->v.del_stmt = del_stmt;
    return stmt;
}

Stmt* exitStmt(Expr* x, int lineno)
{
    ExitStmt* exit_stmt = (struct ExitStmt*)calloc(1, sizeof(ExitStmt));
    exit_stmt->x = x;
    Stmt* stmt = buildStmt(ExitStmt_kind, lineno);
    stmt->v.exit_stmt = exit_stmt;
    return stmt;
}

Stmt* symbolTableStmt(int lineno)
{
    SymbolTableStmt* symbol_table_stmt = (struct SymbolTableStmt*)calloc(1, sizeof(SymbolTableStmt));
    symbol_table_stmt->kind = SymbolTableStmt_kind;
    Stmt* stmt = buildStmt(SymbolTableStmt_kind, lineno);
    stmt->v.symbol_table_stmt = symbol_table_stmt;
    return stmt;
}

Stmt* functionTableStmt(int lineno)
{
    FunctionTableStmt* function_table_stmt = (struct FunctionTableStmt*)calloc(1, sizeof(FunctionTableStmt));
    function_table_stmt->kind = FunctionTableStmt_kind;
    Stmt* stmt = buildStmt(FunctionTableStmt_kind, lineno);
    stmt->v.function_table_stmt = function_table_stmt;
    return stmt;
}

Stmt* blockStmt(StmtList* stmt_list, int lineno)
{
    BlockStmt* block_stmt = (struct BlockStmt*)calloc(1, sizeof(BlockStmt));
    block_stmt->stmt_list = stmt_list;
    Stmt* stmt = buildStmt(BlockStmt_kind, lineno);
    stmt->v.block_stmt = block_stmt;
    return stmt;
}

Stmt* breakStmt(int lineno)
{
    BreakStmt* break_stmt = (struct BreakStmt*)calloc(1, sizeof(BreakStmt));
    break_stmt->kind = BreakStmt_kind;
    Stmt* stmt = buildStmt(BreakStmt_kind, lineno);
    stmt->v.break_stmt = break_stmt;
    return stmt;
}

Stmt* continueStmt(int lineno)
{
    ContinueStmt* continue_stmt = (struct ContinueStmt*)calloc(1, sizeof(ContinueStmt));
    continue_stmt->kind = ContinueStmt_kind;
    Stmt* stmt = buildStmt(ContinueStmt_kind, lineno);
    stmt->v.continue_stmt = continue_stmt;
    return stmt;
}


// Spec

Spec* buildSpec(enum SpecKind kind, int lineno)
{
    Spec* spec = (struct Spec*)calloc(1, sizeof(Spec));
    spec->ast = ast(lineno);
    spec->kind = kind;
    return spec;
}

Spec* typeSpec(enum Type type, Spec* sub_type_spec, int lineno)
{
    TypeSpec* type_spec = (struct TypeSpec*)calloc(1, sizeof(TypeSpec));
    type_spec->type = type;
    type_spec->sub_type_spec = sub_type_spec;
    Spec* spec = buildSpec(TypeSpec_kind, lineno);
    spec->v.type_spec = type_spec;
    return spec;
}

Spec* prettySpec(int lineno)
{
    PrettySpec* pretty_spec = (struct PrettySpec*)calloc(1, sizeof(PrettySpec));
    pretty_spec->kind = PrettySpec_kind;
    Spec* spec = buildSpec(PrettySpec_kind, lineno);
    spec->v.pretty_spec = pretty_spec;
    return spec;
}

Spec* parentDirSpec(int lineno)
{
    ParentDirSpec* parent_dir_spec = (struct ParentDirSpec*)calloc(1, sizeof(ParentDirSpec));
    parent_dir_spec->kind = ParentDirSpec_kind;
    Spec* spec = buildSpec(ParentDirSpec_kind, lineno);
    spec->v.parent_dir_spec = parent_dir_spec;
    return spec;
}

Spec* asteriskSpec(int lineno)
{
    AsteriskSpec* asterisk_spec = (struct AsteriskSpec*)calloc(1, sizeof(AsteriskSpec));
    asterisk_spec->kind = AsteriskSpec_kind;
    Spec* spec = buildSpec(AsteriskSpec_kind, lineno);
    spec->v.asterisk_spec = asterisk_spec;
    return spec;
}

Spec* listType(int lineno)
{
    ListType* list_type = (struct ListType*)calloc(1, sizeof(ListType));
    list_type->kind = ListType_kind;
    Spec* spec = buildSpec(ListType_kind, lineno);
    spec->v.list_type = list_type;
    return spec;
}

Spec* dictType(int lineno)
{
    DictType* dict_type = (struct DictType*)calloc(1, sizeof(DictType));
    dict_type->kind = DictType_kind;
    Spec* spec = buildSpec(DictType_kind, lineno);
    spec->v.dict_type = dict_type;
    return spec;
}

Spec* importSpec(Expr* module_selector, Expr* ident, ExprList* names, Spec* asterisk, int lineno)
{
    ImportSpec* import_spec = (struct ImportSpec*)calloc(1, sizeof(ImportSpec));
    import_spec->module_selector = module_selector;
    import_spec->ident = ident;
    if (names == NULL) {
        names = (struct ExprList*)calloc(1, sizeof(ExprList));
        names->expr_count = 0;
    }
    import_spec->names = names;
    import_spec->asterisk = asterisk;
    Spec* spec = buildSpec(ImportSpec_kind, lineno);
    spec->v.import_spec = import_spec;
    return spec;
}

Spec* funcType(Spec* params, Spec* result, int lineno)
{
    FuncType* func_type = (struct FuncType*)calloc(1, sizeof(FuncType));
    func_type->params = params;
    func_type->result = result;
    Spec* spec = buildSpec(FuncType_kind, lineno);
    spec->v.func_type = func_type;
    return spec;
}

Spec* fieldListSpec(SpecList* list, int lineno)
{
    FieldListSpec* field_list_spec = (struct FieldListSpec*)calloc(1, sizeof(FieldListSpec));
    field_list_spec->list = list;
    Spec* spec = buildSpec(FieldListSpec_kind, lineno);
    spec->v.field_list_spec = field_list_spec;
    return spec;
}

Spec* fieldSpec(Spec* type_spec, Expr* ident, int lineno)
{
    FieldSpec* field_spec = (struct FieldSpec*)calloc(1, sizeof(FieldSpec));
    field_spec->type_spec = type_spec;
    field_spec->ident = ident;
    Spec* spec = buildSpec(FieldSpec_kind, lineno);
    spec->v.field_spec = field_spec;
    return spec;
}

Spec* optionalFieldSpec(Spec* type_spec, Expr* ident, Expr* expr, int lineno)
{
    OptionalFieldSpec* optional_field_spec = (struct OptionalFieldSpec*)calloc(1, sizeof(OptionalFieldSpec));
    optional_field_spec->type_spec = type_spec;
    optional_field_spec->ident = ident;
    optional_field_spec->expr = expr;
    Spec* spec = buildSpec(OptionalFieldSpec_kind, lineno);
    spec->v.optional_field_spec = optional_field_spec;
    return spec;
}

Spec* decisionBlock(ExprList* decisions, int lineno)
{
    DecisionBlock* decision_block = (struct DecisionBlock*)calloc(1, sizeof(DecisionBlock));
    decision_block->decisions = decisions;
    Spec* spec = buildSpec(DecisionBlock_kind, lineno);
    spec->v.decision_block = decision_block;
    return spec;
}


// Decl

Decl* buildDecl(enum DeclKind kind, int lineno)
{
    Decl* decl = (struct Decl*)calloc(1, sizeof(Decl));
    decl->ast = ast(lineno);
    decl->kind = kind;
    return decl;
}

Decl* varDecl(Spec* type_spec, Expr* ident, Expr* expr, int lineno)
{
    VarDecl* var_decl = (struct VarDecl*)calloc(1, sizeof(VarDecl));
    var_decl->type_spec = type_spec;
    var_decl->ident = ident;
    var_decl->expr = expr;
    Decl* decl = buildDecl(VarDecl_kind, lineno);
    decl->v.var_decl = var_decl;
    return decl;
}

Decl* timesDo(Expr* x, Stmt* body, int lineno)
{
    TimesDo* times_do = (struct TimesDo*)calloc(1, sizeof(TimesDo));
    times_do->x = x;
    times_do->body = body;
    Decl* decl = buildDecl(TimesDo_kind, lineno);
    decl->v.times_do = times_do;
    return decl;
}

Decl* foreachAsList(Expr* x, Expr* el, Stmt* body, int lineno)
{
    ForeachAsList* foreach_as_list = (struct ForeachAsList*)calloc(1, sizeof(ForeachAsList));
    foreach_as_list->x = x;
    foreach_as_list->el = el;
    foreach_as_list->body = body;
    Decl* decl = buildDecl(ForeachAsList_kind, lineno);
    decl->v.foreach_as_list = foreach_as_list;
    return decl;
}

Decl* foreachAsDict(Expr* x, Expr* key, Expr* value, Stmt* body, int lineno)
{
    ForeachAsDict* foreach_as_dict = (struct ForeachAsDict*)calloc(1, sizeof(ForeachAsDict));
    foreach_as_dict->x = x;
    foreach_as_dict->key = key;
    foreach_as_dict->value = value;
    foreach_as_dict->body = body;
    Decl* decl = buildDecl(ForeachAsDict_kind, lineno);
    decl->v.foreach_as_dict = foreach_as_dict;
    return decl;
}

Decl* funcDecl(Spec* type, Expr* name, Stmt* body, Spec* decision, int lineno)
{
    FuncDecl* func_decl = (struct FuncDecl*)calloc(1, sizeof(FuncDecl));
    func_decl->type = type;
    func_decl->name = name;
    func_decl->body = body;
    func_decl->decision = decision;
    Decl* decl = buildDecl(FuncDecl_kind, lineno);
    decl->v.func_decl = func_decl;
    return decl;
}


// Generic

void initASTRoot()
{
    _ast_root = (struct ASTRoot*)calloc(1, sizeof(ASTRoot));
    File* file = (struct File*)calloc(1, sizeof(File));
    _ast_root->files = realloc(
        _ast_root->files,
        sizeof(File) * ++_ast_root->file_count
    );
    _ast_root->files[0] = file;
    StmtList* stmt_list = (struct StmtList*)calloc(1, sizeof(StmtList));
    stmt_list->stmt_count = 0;
    file->stmt_list = stmt_list;
    SpecList* imports = (struct SpecList*)calloc(1, sizeof(SpecList));
    imports->spec_count = 0;
    file->imports = imports;
}

void addExpr(ExprList* expr_list, Expr* expr)
{
    expr_list->exprs = realloc(
        expr_list->exprs,
        sizeof(Expr) * ++expr_list->expr_count
    );
    expr_list->exprs[expr_list->expr_count - 1] = expr;
}

void addStmt(StmtList* stmt_list, Stmt* stmt)
{
    stmt_list->stmts = realloc(
        stmt_list->stmts,
        sizeof(Stmt) * ++stmt_list->stmt_count
    );
    stmt_list->stmts[stmt_list->stmt_count - 1] = stmt;
}

void addSpec(SpecList* spec_list, Spec* spec)
{
    spec_list->specs = realloc(
        spec_list->specs,
        sizeof(Spec) * ++spec_list->spec_count
    );
    spec_list->specs[spec_list->spec_count - 1] = spec;
}

FuncDeclCom* funcDeclCom(Spec* func_type, Expr* ident)
{
    FuncDeclCom* func_decl_com = (struct FuncDeclCom*)calloc(1, sizeof(FuncDeclCom));
    func_decl_com->func_type = func_type;
    func_decl_com->ident = ident;
    return func_decl_com;
}
