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

#ifndef KAOS_LOOP_H
#define KAOS_LOOP_H

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <setjmp.h>

#include "symbol.h"
#include "../ast/ast.h"

enum LoopType { TIMESDO, FOREACH, FOREACH_DICT };

typedef struct {
    char *name;
    char *key;
    char *value;
} LoopElement;

typedef struct {
    enum LoopType type;
    unsigned long long iter;
    bool is_infinite;
    unsigned nested_counter;
    char *list;
    LoopElement element;
    struct ASTNode* ast_node;
} Loop;

Loop* loop_mode;
bool loop_execution_mode;

jmp_buf InteractiveShellLoopErrorAbsorber;

jmp_buf LoopBreak;
jmp_buf LoopContinue;

ASTNode* startTimesDo(long long iter, bool is_infinite, ASTNode* ast_node);
ASTNode* startForeach(char *list_name, char *element_name, ASTNode* ast_node);
ASTNode* startForeachDict(char *dict_name, char *element_key, char *element_value, ASTNode* ast_node);
void breakLoop();
void continueLoop();

#endif
