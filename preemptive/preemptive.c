/*
 * Description: Preemptive module of the Chaos Programming Language's source
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

#include "preemptive.h"

extern int kaos_lineno;

void preemptive_check() {
    function_cursor = start_function;
    while (function_cursor != NULL) {
        _Function* function = function_cursor;
        function_cursor = function_cursor->next;
        check_function(function->node->child, function->module_context, function);
        check_function(function->decision_node, function->module_context, function);
        preemptive_freeAllSymbols();
    }
}

ASTNode* check_function(ASTNode* ast_node, char *module, _Function* function) {
check_function_label:
    if (ast_node == NULL || stop_ast_evaluation) {
        return ast_node;
    }

    if (strcmp(ast_node->module, module) != 0) {
        ast_node = ast_node->next;
        goto check_function_label;
    }

    if (ast_node->node_type == AST_DECISION_DEFINE) {
        ast_node = ast_node->next;
        goto check_function_label;
    }

    if (ast_node->node_type != AST_FUNCTION_STEP)
        if (is_node_function_related(ast_node)) {
            ast_node = ast_node->next;
            goto check_function_label;
        }

    if (ast_node->depend != NULL) {
        check_function(ast_node->depend, module, function);
    }

    if (ast_node->right != NULL) {
        check_function(ast_node->right, module, function);
    }

    if (ast_node->left != NULL) {
        check_function(ast_node->left, module, function);
    }

    kaos_lineno = ast_node->lineno;

    if (debug_enabled)
        printf(
            "(Check)\t\tASTNode: {id: %llu, node_type: %s, module: %s, string_size: %zu}\n",
            ast_node->id,
            getAstNodeTypeName(ast_node->node_type),
            ast_node->module,
            ast_node->strings_size
        );

    Symbol* symbol;

    switch (ast_node->node_type)
    {
        case AST_VAR_CREATE_BOOL:
            preemptive_addSymbol(ast_node->strings[0], K_BOOL, V_BOOL);
            break;
        case AST_VAR_CREATE_BOOL_VAR:
            preemptive_addSymbol(ast_node->strings[0], K_BOOL, V_BOOL);
            break;
        case AST_VAR_CREATE_BOOL_VAR_EL:
            preemptive_addSymbol(ast_node->strings[0], K_BOOL, V_BOOL);
            break;
        case AST_VAR_CREATE_BOOL_FUNC_RETURN:
            preemptive_addSymbol(ast_node->strings[0], K_BOOL, V_BOOL);
            break;
        case AST_VAR_CREATE_NUMBER:
            preemptive_addSymbol(ast_node->strings[0], K_NUMBER, ast_node->right->value_type);
            break;
        case AST_VAR_CREATE_NUMBER_VAR:
            preemptive_addSymbol(ast_node->strings[0], K_NUMBER, V_INT);
            break;
        case AST_VAR_CREATE_NUMBER_VAR_EL:
            preemptive_addSymbol(ast_node->strings[0], K_NUMBER, V_INT);
            break;
        case AST_VAR_CREATE_NUMBER_FUNC_RETURN:
            preemptive_addSymbol(ast_node->strings[0], K_NUMBER, V_INT);
            break;
        case AST_VAR_CREATE_STRING:
            preemptive_addSymbol(ast_node->strings[0], K_STRING, V_STRING);
            break;
        case AST_VAR_CREATE_STRING_VAR:
            preemptive_addSymbol(ast_node->strings[0], K_STRING, V_STRING);
            break;
        case AST_VAR_CREATE_STRING_VAR_EL:
            preemptive_addSymbol(ast_node->strings[0], K_STRING, V_STRING);
            break;
        case AST_VAR_CREATE_STRING_FUNC_RETURN:
            preemptive_addSymbol(ast_node->strings[0], K_STRING, V_STRING);
            break;
        case AST_VAR_CREATE_ANY_BOOL:
            preemptive_addSymbol(ast_node->strings[0], K_ANY, V_BOOL);
            break;
        case AST_VAR_CREATE_ANY_NUMBER:
            preemptive_addSymbol(ast_node->strings[0], K_ANY, ast_node->right->value_type);
            break;
        case AST_VAR_CREATE_ANY_STRING:
            preemptive_addSymbol(ast_node->strings[0], K_ANY, V_STRING);
            break;
        case AST_VAR_CREATE_ANY_VAR:
            preemptive_addSymbol(ast_node->strings[0], K_ANY, V_VOID);
            break;
        case AST_VAR_CREATE_ANY_VAR_EL:
            preemptive_addSymbol(ast_node->strings[0], K_ANY, V_VOID);
            break;
        case AST_VAR_CREATE_ANY_FUNC_RETURN:
            preemptive_addSymbol(ast_node->strings[0], K_ANY, V_VOID);
            break;
        case AST_VAR_CREATE_LIST:
            symbol = preemptive_addSymbol(ast_node->strings[0], K_LIST, V_VOID);
            symbol->secondary_type = K_ANY;
            break;
        case AST_VAR_CREATE_LIST_VAR:
            symbol = preemptive_addSymbol(ast_node->strings[0], K_LIST, V_VOID);
            symbol->secondary_type = K_ANY;
            break;
        case AST_VAR_CREATE_LIST_FUNC_RETURN:
            symbol = preemptive_addSymbol(ast_node->strings[0], K_LIST, V_VOID);
            symbol->secondary_type = K_ANY;
            break;
        case AST_VAR_CREATE_DICT:
            symbol = preemptive_addSymbol(ast_node->strings[0], K_DICT, V_VOID);
            symbol->secondary_type = K_ANY;
            break;
        case AST_VAR_CREATE_DICT_VAR:
            symbol = preemptive_addSymbol(ast_node->strings[0], K_DICT, V_VOID);
            symbol->secondary_type = K_ANY;
            break;
        case AST_VAR_CREATE_DICT_FUNC_RETURN:
            symbol = preemptive_addSymbol(ast_node->strings[0], K_DICT, V_VOID);
            symbol->secondary_type = K_ANY;
            break;
        case AST_VAR_CREATE_BOOL_LIST:
            symbol = preemptive_addSymbol(ast_node->strings[0], K_LIST, V_VOID);
            symbol->secondary_type = K_BOOL;
            break;
        case AST_VAR_CREATE_BOOL_LIST_VAR:
            symbol = preemptive_addSymbol(ast_node->strings[0], K_LIST, V_VOID);
            symbol->secondary_type = K_BOOL;
            break;
        case AST_VAR_CREATE_BOOL_LIST_FUNC_RETURN:
            symbol = preemptive_addSymbol(ast_node->strings[0], K_LIST, V_VOID);
            symbol->secondary_type = K_BOOL;
            break;
        case AST_VAR_CREATE_BOOL_DICT:
            symbol = preemptive_addSymbol(ast_node->strings[0], K_DICT, V_VOID);
            symbol->secondary_type = K_BOOL;
            break;
        case AST_VAR_CREATE_BOOL_DICT_VAR:
            symbol = preemptive_addSymbol(ast_node->strings[0], K_DICT, V_VOID);
            symbol->secondary_type = K_BOOL;
            break;
        case AST_VAR_CREATE_BOOL_DICT_FUNC_RETURN:
            symbol = preemptive_addSymbol(ast_node->strings[0], K_DICT, V_VOID);
            symbol->secondary_type = K_BOOL;
            break;
        case AST_VAR_CREATE_NUMBER_LIST:
            symbol = preemptive_addSymbol(ast_node->strings[0], K_LIST, V_VOID);
            symbol->secondary_type = K_NUMBER;
            break;
        case AST_VAR_CREATE_NUMBER_LIST_VAR:
            symbol = preemptive_addSymbol(ast_node->strings[0], K_LIST, V_VOID);
            symbol->secondary_type = K_NUMBER;
            break;
        case AST_VAR_CREATE_NUMBER_LIST_FUNC_RETURN:
            symbol = preemptive_addSymbol(ast_node->strings[0], K_LIST, V_VOID);
            symbol->secondary_type = K_NUMBER;
            break;
        case AST_VAR_CREATE_NUMBER_DICT:
            symbol = preemptive_addSymbol(ast_node->strings[0], K_DICT, V_VOID);
            symbol->secondary_type = K_NUMBER;
            break;
        case AST_VAR_CREATE_NUMBER_DICT_VAR:
            symbol = preemptive_addSymbol(ast_node->strings[0], K_DICT, V_VOID);
            symbol->secondary_type = K_NUMBER;
            break;
        case AST_VAR_CREATE_NUMBER_DICT_FUNC_RETURN:
            symbol = preemptive_addSymbol(ast_node->strings[0], K_DICT, V_VOID);
            symbol->secondary_type = K_NUMBER;
            break;
        case AST_VAR_CREATE_STRING_LIST:
            symbol = preemptive_addSymbol(ast_node->strings[0], K_LIST, V_VOID);
            symbol->secondary_type = K_STRING;
            break;
        case AST_VAR_CREATE_STRING_LIST_VAR:
            symbol = preemptive_addSymbol(ast_node->strings[0], K_LIST, V_VOID);
            symbol->secondary_type = K_STRING;
            break;
        case AST_VAR_CREATE_STRING_LIST_FUNC_RETURN:
            symbol = preemptive_addSymbol(ast_node->strings[0], K_LIST, V_VOID);
            symbol->secondary_type = K_STRING;
            break;
        case AST_VAR_CREATE_STRING_DICT:
            symbol = preemptive_addSymbol(ast_node->strings[0], K_DICT, V_VOID);
            symbol->secondary_type = K_STRING;
            break;
        case AST_VAR_CREATE_STRING_DICT_VAR:
            symbol = preemptive_addSymbol(ast_node->strings[0], K_DICT, V_VOID);
            symbol->secondary_type = K_STRING;
            break;
        case AST_VAR_CREATE_STRING_DICT_FUNC_RETURN:
            symbol = preemptive_addSymbol(ast_node->strings[0], K_DICT, V_VOID);
            symbol->secondary_type = K_STRING;
            break;
        case AST_VAR_UPDATE_BOOL:
            preemptive_getSymbol(ast_node->strings[0], function);
            break;
        case AST_VAR_UPDATE_NUMBER:
            preemptive_getSymbol(ast_node->strings[0], function);
            break;
        case AST_VAR_UPDATE_STRING:
            preemptive_getSymbol(ast_node->strings[0], function);
            break;
        case AST_VAR_UPDATE_LIST:
            preemptive_getSymbol(ast_node->strings[0], function);
            break;
        case AST_VAR_UPDATE_DICT:
            preemptive_getSymbol(ast_node->strings[0], function);
            break;
        case AST_VAR_UPDATE_VAR:
            preemptive_getSymbol(ast_node->strings[0], function);
            preemptive_getSymbol(ast_node->strings[1], function);
            break;
        case AST_VAR_UPDATE_VAR_EL:
            preemptive_getSymbol(ast_node->strings[0], function);
            preemptive_getSymbol(ast_node->strings[1], function);
            break;
        case AST_COMPLEX_EL_UPDATE_VAR:
            preemptive_getSymbol(ast_node->strings[0], function);
            break;
        case AST_COMPLEX_EL_UPDATE_VAR_EL:
            preemptive_getSymbol(ast_node->strings[0], function);
            break;
        case AST_PRINT_VAR:
            preemptive_getSymbol(ast_node->strings[0], function);
            break;
        case AST_PRINT_VAR_EL:
            preemptive_getSymbol(ast_node->strings[0], function);
            break;
        case AST_ECHO_VAR:
            preemptive_getSymbol(ast_node->strings[0], function);
            break;
        case AST_ECHO_VAR_EL:
            preemptive_getSymbol(ast_node->strings[0], function);
            break;
        case AST_PRETTY_PRINT_VAR:
            preemptive_getSymbol(ast_node->strings[0], function);
            break;
        case AST_PRETTY_PRINT_VAR_EL:
            preemptive_getSymbol(ast_node->strings[0], function);
            break;
        case AST_PRETTY_ECHO_VAR:
            preemptive_getSymbol(ast_node->strings[0], function);
            break;
        case AST_PRETTY_ECHO_VAR_EL:
            preemptive_getSymbol(ast_node->strings[0], function);
            break;
        case AST_VAR_EXPRESSION_VALUE:
            preemptive_getSymbol(ast_node->strings[0], function);
            break;
        case AST_VAR_EXPRESSION_INCREMENT_ASSIGN:
            preemptive_getSymbol(ast_node->strings[0], function);
            break;
        case AST_VAR_EXPRESSION_ASSIGN_INCREMENT:
            preemptive_getSymbol(ast_node->strings[0], function);
            break;
        case AST_VAR_MIXED_EXPRESSION_VALUE:
            preemptive_getSymbol(ast_node->strings[0], function);
            break;
        case AST_BOOLEAN_EXPRESSION_REL_EQUAL_UNKNOWN:
            preemptive_getSymbol(ast_node->strings[0], function);
            preemptive_getSymbol(ast_node->strings[1], function);
            break;
        case AST_BOOLEAN_EXPRESSION_REL_NOT_EQUAL_UNKNOWN:
            preemptive_getSymbol(ast_node->strings[0], function);
            preemptive_getSymbol(ast_node->strings[1], function);
            break;
        case AST_BOOLEAN_EXPRESSION_REL_GREAT_UNKNOWN:
            preemptive_getSymbol(ast_node->strings[0], function);
            preemptive_getSymbol(ast_node->strings[1], function);
            break;
        case AST_BOOLEAN_EXPRESSION_REL_SMALL_UNKNOWN:
            preemptive_getSymbol(ast_node->strings[0], function);
            preemptive_getSymbol(ast_node->strings[1], function);
            break;
        case AST_BOOLEAN_EXPRESSION_REL_GREAT_EQUAL_UNKNOWN:
            preemptive_getSymbol(ast_node->strings[0], function);
            preemptive_getSymbol(ast_node->strings[1], function);
            break;
        case AST_BOOLEAN_EXPRESSION_REL_SMALL_EQUAL_UNKNOWN:
            preemptive_getSymbol(ast_node->strings[0], function);
            preemptive_getSymbol(ast_node->strings[1], function);
            break;
        case AST_VAR_BOOLEAN_EXPRESSION_VALUE:
            preemptive_getSymbol(ast_node->strings[0], function);
            break;
        case AST_DELETE_VAR:
            preemptive_getSymbol(ast_node->strings[0], function);
            break;
        case AST_DELETE_VAR_EL:
            preemptive_getSymbol(ast_node->strings[0], function);
            break;
        case AST_LIST_ADD_VAR:
            preemptive_getSymbol(ast_node->strings[0], function);
            break;
        case AST_LIST_ADD_VAR_EL:
            preemptive_getSymbol(ast_node->strings[0], function);
            break;
        case AST_DICT_ADD_VAR:
            preemptive_getSymbol(ast_node->strings[1], function);
            break;
        case AST_DICT_ADD_VAR_EL:
            preemptive_getSymbol(ast_node->strings[1], function);
            break;
        case AST_LEFT_RIGHT_BRACKET_VAR:
            preemptive_getSymbol(ast_node->strings[0], function);
            break;
        case AST_LEFT_RIGHT_BRACKET_VAR_MINUS:
            preemptive_getSymbol(ast_node->strings[0], function);
            break;
        case AST_BUILD_COMPLEX_VARIABLE:
            preemptive_getSymbol(ast_node->strings[0], function);
            break;
        case AST_EXIT_VAR:
            preemptive_getSymbol(ast_node->strings[0], function);
            break;
        case AST_FUNCTION_CALL_PARAMETER_VAR:
            preemptive_getSymbol(ast_node->strings[0], function);
            break;
        default:
            break;
    }

    ast_node = ast_node->next;
    goto check_function_label;
    return ast_node;
}
