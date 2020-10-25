/*
 * Description: Interpreter module of the Chaos Programming Language's source
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

#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "../ast/ast.h"

extern unsigned long long nested_loop_counter;

void interpret(char *module, enum Phase phase_arg, bool is_interactive);
ASTNode* startTimesDo(unsigned long long iter, bool is_infinite, ASTNode* ast_node);
ASTNode* startForeach(char *list_name, char *element_name, ASTNode* ast_node);
ASTNode* startForeachDict(char *list_name, char *element_key, char *element_value, ASTNode* ast_node);
bool is_node_function_related(ASTNode* ast_node);
ASTNode* register_functions(ASTNode* ast_node, char *module);
ASTNode* eval_node(ASTNode* ast_node, char *module);
ASTNode* walk_until_end(ASTNode* ast_node, char *module);
void breakLoop();
void continueLoop();

#endif
