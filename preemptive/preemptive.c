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
        ASTNode* ast_node = function->node->child;
        char *module = function->module_context;
        check_function(ast_node, module);
    }
}

ASTNode* check_function(ASTNode* ast_node, char *module) {
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
        check_function(ast_node->depend, module);
    }

    if (ast_node->right != NULL) {
        check_function(ast_node->right, module);
    }

    if (ast_node->left != NULL) {
        check_function(ast_node->left, module);
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

    switch (ast_node->node_type)
    {
        case AST_VAR_CREATE_NUMBER:
            preemptive_addSymbol(ast_node->strings[0], K_NUMBER, ast_node->right->value_type);
            break;
        case AST_PRINT_VAR:
            // preemptive_getSymbol(ast_node->strings[0]);
            break;
        default:
            break;
    }

    ast_node = ast_node->next;
    goto check_function_label;
    return ast_node;
}
