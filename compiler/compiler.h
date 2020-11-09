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

#include <sys/types.h>
#include <sys/stat.h>
#if defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__)
#include <windows.h>
#else
#include <sys/wait.h>
#endif

#if defined(__APPLE__) && defined(__MACH__)
#include <sys/syslimits.h>
#elif !defined(_WIN32) && !defined(_WIN64) && !defined(__CYGWIN__)
#include <linux/limits.h>
#endif

#include "../ast/ast.h"

string_array transpiled_functions;
string_array transpiled_decisions;

void compile(char *module, enum Phase phase_arg, char *bin_file, char *extra_flags);
ASTNode* transpile_functions(ASTNode* ast_node, char *module, FILE *c_fp, unsigned short indent, FILE *h_fp);
ASTNode* transpile_decisions(ASTNode* ast_node, char *module, FILE *c_fp, unsigned short indent);
ASTNode* compiler_register_functions(ASTNode* ast_node, char *module, FILE *c_fp, unsigned short indent);
ASTNode* transpile_node(ASTNode* ast_node, char *module, FILE *c_fp, unsigned short indent);
bool transpile_common_operator(ASTNode* ast_node, char *operator, enum ValueType left_value_type, enum ValueType right_value_type);
bool transpile_common_mixed_operator(ASTNode* ast_node, char *operator);
void transpile_function_call(FILE *c_fp, char *module, char *name);
void transpile_function_call_decision(FILE *c_fp, char *module_context, char* module_context_compiler, char *name);
void transpile_function_call_create_var(FILE *c_fp, ASTNode* ast_node, char *module, enum Type type1, enum Type type2);
void compiler_handleModuleImport(char *module_name, bool directly_import, FILE *c_fp, unsigned short indent, FILE *h_fp);
void compiler_handleModuleImportRegister(char *module_name, bool directly_import, FILE *c_fp, unsigned short indent);
char* compiler_getCurrentContext();
char* compiler_getCurrentModuleContext();
char* compiler_getCurrentModule();
char* compiler_getFunctionModuleContext(char *name, char *module);
bool isFunctionFromDynamicLibrary(char *name, char *module);
bool isFunctionFromDynamicLibraryByModuleContext(char *name, char *module);
char* fix_bs(char* str);
void compiler_escape_module(char* module);
void free_transpiled_functions();
void free_transpiled_decisions();

#endif
