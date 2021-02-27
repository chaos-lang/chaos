/*
 * Description: Compiler module of the Chaos Programming Language's source
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

#ifndef KAOS_COMPILER_H
#define KAOS_COMPILER_H

#include <math.h>

#include "../ast/ast.h"
#include "../vm/types.h"
#include "../vm/instructions.h"

typedef struct i64_array {
    i64* arr;
    size_t capacity;
    size_t size;
} i64_array;

i64_array* compile(ASTRoot* ast_root);
void compileStmtList(i64_array* program, StmtList* stmt_list);
void compileStmt(i64_array* program, Stmt* stmt);
void compileExpr(i64_array* program, Expr* expr);
void pushProgram(i64_array* program, i64 el);
i64 popProgram(i64_array* program);
void expandStack(i64_array* program, i64 stack);
void freeProgram(i64_array* program);
i64_array* initProgram();

#endif
