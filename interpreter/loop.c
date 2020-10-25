/*
 * Description: Loop module of the Chaos Programming Language's source
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

#include <stdio.h>
#include <string.h>

#include "loop.h"
#include "../interpreter/symbol.h"
#include "../interpreter/interpreter.h"

unsigned long long nested_loop_counter = 0;
bool interactive_shell_loop_error_absorbed = false;
bool is_loop_breaked = false;
extern int yyparse();

ASTNode* startTimesDo(unsigned long long iter, bool is_infinite, ASTNode* ast_node) {
    ASTNode* next_node = walk_until_end(ast_node->next, ast_node->module);

    if (is_interactive) {
        if (setjmp(InteractiveShellLoopErrorAbsorber)) {
            interactive_shell_loop_error_absorbed = true;
        }
    }

    nested_loop_counter++;
    if (!interactive_shell_loop_error_absorbed && !is_loop_breaked) {
        if (is_infinite) {
            while (true) {
                if (setjmp(LoopBreak))
                    break;
                if (setjmp(LoopContinue))
                    continue;
                next_node = eval_node(ast_node->next, ast_node->module);
            }
        } else {
            for (unsigned long long i = 0; i < iter; i++) {
                if (setjmp(LoopBreak))
                    break;
                if (setjmp(LoopContinue))
                    continue;
                next_node = eval_node(ast_node->next, ast_node->module);
            }
        }
    }
    nested_loop_counter--;

    return eval_node(next_node->next, ast_node->module);
}

ASTNode* startForeach(char *list_name, char *element_name, ASTNode* ast_node) {
    ASTNode* next_node = walk_until_end(ast_node->next, ast_node->module);

    Symbol* list = getSymbol(list_name);
    if (list->type != K_LIST)
        throw_error(E_NOT_A_LIST, list_name);

    if (is_interactive) {
        if (setjmp(InteractiveShellLoopErrorAbsorber)) {
            interactive_shell_loop_error_absorbed = true;
        }
    }

    nested_loop_counter++;
    if (!interactive_shell_loop_error_absorbed && !is_loop_breaked) {
        for (unsigned long i = 0; i < list->children_count; i++) {
            Symbol* child = list->children[i];
            Symbol* clone_symbol = createCloneFromSymbol(element_name, child->type, child, child->secondary_type);
            if (setjmp(LoopBreak)) {
                removeSymbol(clone_symbol);
                break;
            }
            if (setjmp(LoopContinue)) {
                removeSymbol(clone_symbol);
                continue;
            }
            next_node = eval_node(ast_node->next, ast_node->module);
            removeSymbol(clone_symbol);
        }
    }
    nested_loop_counter--;

    return eval_node(next_node->next, ast_node->module);
}

ASTNode* startForeachDict(char *dict_name, char *element_key, char *element_value, ASTNode* ast_node) {
    ASTNode* next_node = walk_until_end(ast_node->next, ast_node->module);

    Symbol* dict = getSymbol(dict_name);
    if (dict->type != K_DICT)
        throw_error(E_NOT_A_DICT, dict_name);

    if (is_interactive) {
        if (setjmp(InteractiveShellLoopErrorAbsorber)) {
            interactive_shell_loop_error_absorbed = true;
        }
    }

    nested_loop_counter++;
    if (!interactive_shell_loop_error_absorbed && !is_loop_breaked) {
        for (unsigned long i = 0; i < dict->children_count; i++) {
            Symbol* child = dict->children[i];

            addSymbolString(element_key, child->key);
            Symbol* clone_symbol = createCloneFromSymbol(element_value, child->type, child, child->secondary_type);
            if (setjmp(LoopBreak)) {
                removeSymbol(clone_symbol);
                removeSymbolByName(element_key);
                break;
            }
            if (setjmp(LoopContinue)) {
                removeSymbol(clone_symbol);
                removeSymbolByName(element_key);
                continue;
            }
            next_node = eval_node(ast_node->next, ast_node->module);
            removeSymbol(clone_symbol);
            removeSymbolByName(element_key);
        }
    }
    nested_loop_counter--;

    return eval_node(next_node->next, ast_node->module);
}

void breakLoop() {
    longjmp(LoopBreak, 1);
}

void continueLoop() {
    longjmp(LoopContinue, 1);
}
