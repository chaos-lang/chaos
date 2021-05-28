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
    i64 capacity;
    i64 size;
    i64 heap;
    i64 start;
} i64_array;

i64_array* compile(ASTRoot* ast_root);
void fillCallJumps(i64_array* program);
void compileImports(ASTRoot* ast_root, i64_array* program);
void compileStmtList(i64_array* program, StmtList* stmt_list);
void compileStmt(i64_array* program, Stmt* stmt);
unsigned short compileExpr(i64_array* program, Expr* expr);
void compileDecl(i64_array* program, Decl* decl);
void compileSpecList(i64_array* program, SpecList* spec_list);
unsigned short compileSpec(i64_array* program, Spec* spec);

void push_instr(i64_array* program, i64 el);
i64 popProgram(i64_array* program);
void freeProgram(i64_array* program);
i64_array* initProgram();
void shift_registers(i64_array* program, size_t shift);

Symbol* store_bool(i64_array* program, char *name, bool is_any);
Symbol* store_int(i64_array* program, char *name, bool is_any);
Symbol* store_float(i64_array* program, char *name, bool is_any);
Symbol* store_string(i64_array* program, char *name, size_t len, bool is_any, bool is_dynamic);
Symbol* store_list(i64_array* program, char *name, size_t len, bool is_dynamic);
Symbol* store_dict(i64_array* program, char *name, size_t len, bool is_dynamic);
Symbol* store_any(i64_array* program, char *name);

void load_bool(i64_array* program, Symbol* symbol);
void load_int(i64_array* program, Symbol* symbol);
void load_float(i64_array* program, Symbol* symbol);
void load_string(i64_array* program, Symbol* symbol);
void load_list(i64_array* program, Symbol* symbol);
void load_dict(i64_array* program, Symbol* symbol);
void load_any(i64_array* program, Symbol* symbol);

char* compile_module_selector(Expr* module_selector);

#endif
