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

#include "../vm/cpu.h"
#include "../ast/ast.h"
#include "../interpreter/module_new.h"

KaosIR* compile(ASTRoot* ast_root);
void initCallJumps();
void fillCallJumps(KaosIR* program);
void compileImports(ASTRoot* ast_root, KaosIR* program);
void compileStmtList(KaosIR* program, StmtList* stmt_list);
void compileStmt(KaosIR* program, Stmt* stmt);
unsigned short compileExpr(KaosIR* program, Expr* expr);
void compileDecl(KaosIR* program, Decl* decl);
void declareSpecList(KaosIR* program, SpecList* spec_list);
void compileSpecList(KaosIR* program, SpecList* spec_list);
unsigned short declareSpec(KaosIR* program, Spec* spec);
unsigned short compileSpec(KaosIR* program, Spec* spec);

void push_inst_(KaosIR* program, enum IROpCode op_code);
void push_inst_i(KaosIR* program, enum IROpCode op_code, i64 i);
void push_inst_r(KaosIR* program, enum IROpCode op_code, enum IRRegister reg);
void push_inst_i_i(KaosIR* program, enum IROpCode op_code, i64 i1, i64 i2);
void push_inst_r_i(KaosIR* program, enum IROpCode op_code, enum IRRegister reg, i64 i);
void push_inst_r_f(KaosIR* program, enum IROpCode op_code, enum IRRegister reg, f64 f);
void push_inst_r_r(KaosIR* program, enum IROpCode op_code, enum IRRegister reg1, enum IRRegister reg2);
void push_inst_r_r_i(KaosIR* program, enum IROpCode op_code, enum IRRegister reg1, enum IRRegister reg2, i64 i);
void push_inst_r_i_i(KaosIR* program, enum IROpCode op_code, enum IRRegister reg1, i64 i1, i64 i2);
void push_inst_r_r_f(KaosIR* program, enum IROpCode op_code, enum IRRegister reg1, enum IRRegister reg2, f64 f);
void push_inst_r_r_r(KaosIR* program, enum IROpCode op_code, enum IRRegister reg1, enum IRRegister reg2, enum IRRegister reg3);
void push_inst_r_r_r_i(KaosIR* program, enum IROpCode op_code, enum IRRegister reg1, enum IRRegister reg2, enum IRRegister reg3, i64 i);

void pushProgram(KaosIR* program, KaosInst* el);
KaosInst* popProgram(KaosIR* program);
void freeProgram(KaosIR* program);
KaosIR* initProgram();
void shift_registers(KaosIR* program);

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
