/*
 * Description: Compiler module of the Chaos Programming Language's source
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
 */

#ifndef KAOS_COMPILER_H
#define KAOS_COMPILER_H

#include "../ast/ast.h"
#include "../vm/cpu.h"

typedef struct i64_array {
    i64* arr;
    size_t capacity;
    size_t size;
    i64 heap;
} i64_array;

i64_array* compile(ASTRoot* ast_root);
void compileStmtList(i64_array* program, StmtList* stmt_list);
void compileStmt(i64_array* program, Stmt* stmt);
unsigned short compileExpr(i64_array* program, Expr* expr);
void compileDecl(i64_array* program, Decl* decl);
void compileSpec(i64_array* program, Spec* spec);

void push_instr(i64_array* program, i64 el);
i64 popProgram(i64_array* program);
void freeProgram(i64_array* program);
i64_array* initProgram();
void shift_registers(i64_array* program, size_t shift);

void store_bool(i64_array* program, char *name, bool b, bool is_any);
void store_int(i64_array* program, char *name, i64 i, bool is_any);
void store_float(i64_array* program, char *name, f64 f, bool is_any);
void store_string(i64_array* program, char *name, char *s, size_t len, bool is_any);

void load_bool(i64_array* program, char *name);
void load_int(i64_array* program, char *name);
void load_float(i64_array* program, char *name);
void load_string(i64_array* program, char *name);

#endif
