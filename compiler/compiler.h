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
#include "../interpreter/module_new.h"

typedef struct KaosIR KaosIR;
typedef struct KaosInst KaosInst;
typedef struct KaosOp KaosOp;

typedef struct KaosIR {
    KaosInst* arr;
    i64 capacity;
    i64 size;
    i64 hlt_count;
} KaosIR;

typedef struct KaosInst {
    i64 op_code;
    KaosOp* op1;
    KaosOp* op2;
    KaosOp* op3;
    void* ast_ref;
} KaosInst;


enum IRType { IR_REG, IR_VAL };
enum IRValueType { IR_INT, IR_FLOAT, IR_STRING };

typedef struct KaosOp {
    enum IRType type;
    i64 reg;
    enum IRValueType value_type;
    union IRValue {
        i64 i;
        f64 f;
        byte *s;
    } value;
} KaosOp;

KaosIR* compile(ASTRoot* ast_root);
void initCallJumps();
void fillCallJumps(KaosIR* program);
void compileImports(ASTRoot* ast_root, KaosIR* program);
void compileStmtList(KaosIR* program, StmtList* stmt_list);
void compileStmt(KaosIR* program, Stmt* stmt);
unsigned short compileExpr(KaosIR* program, Expr* expr);
void compileDecl(KaosIR* program, Decl* decl);
void compileSpecList(KaosIR* program, SpecList* spec_list);
unsigned short compileSpec(KaosIR* program, Spec* spec);

void push_instr(KaosIR* program, i64 el);
void pushProgram(KaosIR* program, i64 el);
KaosInst popProgram(KaosIR* program);
void freeProgram(KaosIR* program);
KaosIR* initProgram();
void shift_registers(KaosIR* program, size_t shift);

Symbol* store_bool(KaosIR* program, char *name, bool is_any);
Symbol* store_int(KaosIR* program, char *name, bool is_any);
Symbol* store_float(KaosIR* program, char *name, bool is_any);
Symbol* store_string(KaosIR* program, char *name, size_t len, bool is_any, bool is_dynamic);
Symbol* store_list(KaosIR* program, char *name, size_t len, bool is_dynamic);
Symbol* store_dict(KaosIR* program, char *name, size_t len, bool is_dynamic);
Symbol* store_any(KaosIR* program, char *name);

void load_bool(KaosIR* program, Symbol* symbol);
void load_int(KaosIR* program, Symbol* symbol);
void load_float(KaosIR* program, Symbol* symbol);
void load_string(KaosIR* program, Symbol* symbol);
void load_list(KaosIR* program, Symbol* symbol);
void load_dict(KaosIR* program, Symbol* symbol);
void load_any(KaosIR* program, Symbol* symbol);

char* compile_module_selector(Expr* module_selector);
bool declare_function(Stmt* stmt, File* file, KaosIR* program);
void declare_functions(ASTRoot* ast_root, KaosIR* program);
void compile_functions(ASTRoot* ast_root, KaosIR* program);

void strongly_type(Symbol* symbol_x, Symbol* symbol_y, _Function* function, Expr* expr, enum ValueType value_type);
void strongly_type_basic_check(unsigned short code, char *str1, char *str2, enum Type type, enum ValueType value_type);

cpu *interactive_c;

#endif
