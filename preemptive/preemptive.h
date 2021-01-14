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

#ifndef KAOS_PREEMPTIVE_H
#define KAOS_PREEMPTIVE_H

#include "../ast/ast.h"
#include "preemptive_symbol.h"
#include "preemptive_function.h"

int preemptive_loop_length;

void preemptive_check();
ASTNode* check_function(ASTNode* ast_node, char *module, _Function* function);
void check_break_continue(ASTNode* ast_node, _Function* function, int super_kaos_lineno);
ASTNode* preemptive_walk_until_end(ASTNode* ast_node, char *module);

#endif
