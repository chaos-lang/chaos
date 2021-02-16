/*
 * Description: Abstract Syntax Tree printing module of the Chaos Programming Language's source
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

#ifndef KAOS_AST_PRINT_H
#define KAOS_AST_PRINT_H

#include "ast.h"

void printAST();
void printASTStmt(Stmt* stmt, bool is_list, char *end);
void printASTExpr(Expr* expr, bool is_list, char *end);
void printASTSpec(Spec* spec, bool is_list, char *end);
void printASTDecl(Decl* decl, bool is_list, char *end);
char *getToken(enum Token tok);

#endif
