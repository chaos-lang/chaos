/*
 * Description: Abstract Syntax Tree module of the Chaos Programming Language's source
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

#ifndef KAOS_AST_H
#define KAOS_AST_H

#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "../enums.h"
#include "../utilities/helpers.h"
#include "token.h"

typedef struct File File;

typedef struct AST {
    int lineno;
    File* file;
} AST;


// Expr

enum ExprKind {
    BasicLit_kind=1,
    Ident_kind=2,
    BinaryExpr_kind=3,
    UnaryExpr_kind=4,
    ParenExpr_kind=5,
    IncDecExpr_kind=6,
    ModuleSelector_kind=7,
    AliasExpr_kind=8,
    IndexExpr_kind=9,
    CompositeLit_kind=10,
    KeyValueExpr_kind=11,
    SelectorExpr_kind=12,
    CallExpr_kind=13,
    DecisionExpr_kind=14,
    DefaultExpr_kind=15,
};

typedef struct Expr {
    struct AST* ast;
    enum ExprKind kind;
    union {
        struct BasicLit* basic_lit;
        struct Ident* ident;
        struct BinaryExpr* binary_expr;
        struct UnaryExpr* unary_expr;
        struct ParenExpr* paren_expr;
        struct IncDecExpr* incdec_expr;
        struct ModuleSelector* module_selector;
        struct AliasExpr* alias_expr;
        struct IndexExpr* index_expr;
        struct CompositeLit* composite_lit;
        struct KeyValueExpr* key_value_expr;
        struct SelectorExpr* selector_expr;
        struct CallExpr* call_expr;
        struct DecisionExpr* decision_expr;
        struct DefaultExpr* default_expr;
    } v;
} Expr;

typedef struct BasicLit {
    enum ValueType value_type;
    union Value value;
} BasicLit;

typedef struct Ident {
    char *name;
} Ident;

typedef struct BinaryExpr {
    struct Expr* x;
    enum Token op;
    struct Expr* y;
} BinaryExpr;

typedef struct UnaryExpr {
    enum Token op;
    struct Expr* x;
} UnaryExpr;

typedef struct ParenExpr {
    struct Expr* x;
} ParenExpr;

typedef struct IncDecExpr {
    enum Token op;
    struct Expr* x;
    bool first;
} IncDecExpr;

typedef struct ModuleSelector {
    struct Spec* parent_dir_spec;
    struct Expr* x;
    struct Expr* sel;
} ModuleSelector;

typedef struct AliasExpr {
    struct Expr* name;
    struct Expr* asname;
} AliasExpr;

typedef struct IndexExpr {
    struct Expr* x;
    struct Expr* index;
} IndexExpr;

typedef struct CompositeLit {
    struct Spec* type;
    struct ExprList* elts;
} CompositeLit;

typedef struct KeyValueExpr {
    struct Expr* key;
    struct Expr* value;
} KeyValueExpr;

typedef struct SelectorExpr {
    struct Expr* x;
    struct Expr* sel;
} SelectorExpr;

typedef struct CallExpr {
    struct Expr* fun;
    struct ExprList* args;
} CallExpr;

typedef struct DecisionExpr {
    struct Expr* bool_expr;
    struct Stmt* outcome;
} DecisionExpr;

typedef struct DefaultExpr {
    struct Stmt* outcome;
} DefaultExpr;


// Stmt

enum StmtKind {
    AssignStmt_kind=1,
    PrintStmt_kind=2,
    EchoStmt_kind=3,
    ReturnStmt_kind=4,
    ExprStmt_kind=5,
    DeclStmt_kind=6,
    DelStmt_kind=7,
    ExitStmt_kind=8,
    SymbolTableStmt_kind=9,
    FunctionTableStmt_kind=10,
    BlockStmt_kind=11,
    BreakStmt_kind=12,
};

typedef struct Stmt {
    struct AST* ast;
    enum StmtKind kind;
    union {
        struct AssignStmt* assign_stmt;
        struct PrintStmt* print_stmt;
        struct EchoStmt* echo_stmt;
        struct ReturnStmt* return_stmt;
        struct ExprStmt* expr_stmt;
        struct DeclStmt* decl_stmt;
        struct DelStmt* del_stmt;
        struct ExitStmt* exit_stmt;
        struct FunctionTableStmt* function_table_stmt;
        struct BlockStmt* block_stmt;
        struct BreakStmt* break_stmt;
    } v;
} Stmt;

typedef struct AssignStmt {
    struct Expr* x;
    enum Token tok;
    struct Expr* y;
} AssignStmt;

typedef struct ReturnStmt {
    struct Expr* x;
    bool dont_push_callx;
} ReturnStmt;

typedef struct PrintStmt {
    struct Spec* mod;
    struct Expr* x;
} PrintStmt;

typedef struct EchoStmt {
    struct Spec* mod;
    struct Expr* x;
} EchoStmt;

typedef struct ExprStmt {
    struct Expr* x;
} ExprStmt;

typedef struct DeclStmt {
    struct Decl* decl;
} DeclStmt;

typedef struct DelStmt {
    struct Expr* ident;
} DelStmt;

typedef struct ExitStmt {
    struct Expr* x;
} ExitStmt;

typedef struct SymbolTableStmt {
    enum StmtKind kind;
} SymbolTableStmt;

typedef struct FunctionTableStmt {
    enum StmtKind kind;
} FunctionTableStmt;

typedef struct BlockStmt {
    struct StmtList* stmt_list;
} BlockStmt;

typedef struct BreakStmt {
    enum StmtKind kind;
} BreakStmt;


// Spec

enum SpecKind {
    TypeSpec_kind=1,
    PrettySpec_kind=2,
    ParentDirSpec_kind=3,
    AsteriskSpec_kind=4,
    ImportSpec_kind=5,
    ListType_kind=6,
    DictType_kind=7,
    FuncType_kind=8,
    FieldListSpec_kind=9,
    FieldSpec_kind=10,
    OptionalFieldSpec_kind=11,
    DecisionBlock_kind=12,
};

typedef struct Spec {
    struct AST* ast;
    enum SpecKind kind;
    union {
        struct TypeSpec* type_spec;
        struct PrettySpec* pretty_spec;
        struct ParentDirSpec* parent_dir_spec;
        struct AsteriskSpec* asterisk_spec;
        struct ImportSpec* import_spec;
        struct ListType* list_type;
        struct DictType* dict_type;
        struct FuncType* func_type;
        struct FieldListSpec* field_list_spec;
        struct FieldSpec* field_spec;
        struct OptionalFieldSpec* optional_field_spec;
        struct DecisionBlock* decision_block;
    } v;
} Spec;

typedef struct TypeSpec {
    enum Type type;
    struct Spec* sub_type_spec;
} TypeSpec;

typedef struct PrettySpec {
    enum SpecKind kind;
} PrettySpec;

typedef struct ParentDirSpec {
    enum SpecKind kind;
} ParentDirSpec;

typedef struct AsteriskSpec {
    enum SpecKind kind;
} AsteriskSpec;

typedef struct ImportSpec {
    struct Expr* module_selector;
    struct Expr* ident;
    struct ExprList* names;
    struct Spec* asterisk;
    bool handled;
} ImportSpec;

typedef struct ListType {
    enum SpecKind kind;
} ListType;

typedef struct DictType {
    enum SpecKind kind;
} DictType;

typedef struct FuncType {
    struct Spec* params;
    struct Spec* result;
} FuncType;

typedef struct FieldListSpec {
    struct SpecList* list;
} FieldListSpec;

typedef struct FieldSpec {
    struct Spec* type_spec;
    struct Expr* ident;
} FieldSpec;

typedef struct OptionalFieldSpec {
    struct Spec* type_spec;
    struct Expr* ident;
    struct Expr* expr;
} OptionalFieldSpec;

typedef struct DecisionBlock {
    struct ExprList* decisions;
} DecisionBlock;


// Decl

enum DeclKind {
    VarDecl_kind=1,
    TimesDo_kind=2,
    ForeachAsList_kind=3,
    ForeachAsDict_kind=4,
    FuncDecl_kind=5,
};

typedef struct Decl {
    struct AST* ast;
    enum DeclKind kind;
    union {
        struct VarDecl* var_decl;
        struct TimesDo* times_do;
        struct ForeachAsList* foreach_as_list;
        struct ForeachAsDict* foreach_as_dict;
        struct FuncDecl* func_decl;
    } v;
} Decl;

typedef struct VarDecl {
    struct Spec* type_spec;
    struct Expr* ident;
    struct Expr* expr;
} VarDecl;

typedef struct TimesDo {
    struct Expr* x;
    struct Expr* call_expr;
} TimesDo;

typedef struct ForeachAsList {
    struct Expr* x;
    struct Expr* el;
    struct Expr* call_expr;
} ForeachAsList;

typedef struct ForeachAsDict {
    struct Expr* x;
    struct Expr* key;
    struct Expr* value;
    struct Expr* call_expr;
} ForeachAsDict;

typedef struct FuncDecl {
    struct Spec* type;
    struct Expr* name;
    struct Stmt* body;
    struct Spec* decision;
} FuncDecl;


// Generic

typedef struct ExprList {
    struct Expr** exprs;
    unsigned long expr_count;
} ExprList;

typedef struct StmtList {
    struct Stmt** stmts;
    unsigned long stmt_count;
} StmtList;

typedef struct SpecList {
    struct Spec** specs;
    unsigned long spec_count;
} SpecList;

typedef struct File {
    struct SpecList* imports;
    struct StmtList* stmt_list;
    struct ExprList* aliases;
    char *module;
    char *module_path;
    char *context;
    bool imports_handled;
    bool is_interactive;
} File;

typedef struct ASTRoot {
    struct File** files;
    unsigned long file_count;
} ASTRoot;

ASTRoot* _ast_root;

// Communication

typedef struct FuncDeclCom {
    struct Spec* func_type;
    struct Expr* ident;
} FuncDeclCom;

AST* ast(int lineno);
Expr* buildExpr(enum ExprKind kind, int lineno);
Expr* basicLitBool(bool b, int lineno);
Expr* basicLitInt(long long i, int lineno);
Expr* basicLitFloat(double f, int lineno);
Expr* basicLitString(char *s, int lineno);
Expr* ident(char *s, int lineno);
Expr* binaryExpr(Expr* x, enum Token op, Expr* y, int lineno);
Expr* unaryExpr(enum Token op, Expr* x, int lineno);
Expr* parenExpr(Expr* x, int lineno);
Expr* incDecExpr(enum Token op, Expr* ident, bool first, int lineno);
Expr* moduleSelector(Spec* parent_dir_spec, Expr* x, Expr* sel, int lineno);
Expr* aliasExpr(Expr* name, Expr* asname, int lineno);
Expr* indexExpr(Expr* x, Expr* index, int lineno);
Expr* compositeLit(Spec* type, ExprList* elts, int lineno);
Expr* keyValueExpr(Expr* key, Expr* value, int lineno);
Expr* selectorExpr(Expr* x, Expr* sel, int lineno);
Expr* callExpr(Expr* fun, ExprList* args, int lineno);
Expr* decisionExpr(Expr* bool_expr, Stmt* outcome, int lineno);
Expr* defaultExpr(Stmt* outcome, int lineno);
Stmt* buildStmt(enum StmtKind kind, int lineno);
Stmt* assignStmt(Expr* x, enum Token tok, Expr* y, int lineno);
Stmt* returnStmt(Expr* x, int lineno);
Stmt* printStmt(Spec* mod, Expr* x, int lineno);
Stmt* echoStmt(Spec* mod, Expr* x, int lineno);
Stmt* exprStmt(Expr* x, int lineno);
Stmt* declStmt(Decl* decl, int lineno);
Stmt* delStmt(Expr* ident, int lineno);
Stmt* exitStmt(Expr* x, int lineno);
Stmt* functionTableStmt(int lineno);
Stmt* breakStmt(int lineno);
Stmt* blockStmt(StmtList* stmt_list, int lineno);
Spec* buildSpec(enum SpecKind kind, int lineno);
Spec* typeSpec(enum Type type, Spec* sub_type_spec, int lineno);
Spec* prettySpec(int lineno);
Spec* parentDirSpec(int lineno);
Spec* asteriskSpec(int lineno);
Spec* listType(int lineno);
Spec* dictType(int lineno);
Spec* importSpec(Expr* module_selector, Expr* ident, ExprList* names, Spec* asterisk, int lineno);
Spec* funcType(Spec* params, Spec* result, int lineno);
Spec* fieldListSpec(SpecList* list, int lineno);
Spec* fieldSpec(Spec* type_spec, Expr* ident, int lineno);
Spec* optionalFieldSpec(Spec* type_spec, Expr* ident, Expr* expr, int lineno);
Spec* decisionBlock(ExprList* decisions, int lineno);
Decl* buildDecl(enum DeclKind kind, int lineno);
Decl* varDecl(Spec* type_spec, Expr* ident, Expr* expr, int lineno);
Decl* timesDo(Expr* x, Expr* call_expr, int lineno);
Decl* foreachAsList(Expr* x, Expr* el, Expr* call_expr, int lineno);
Decl* foreachAsDict(Expr* x, Expr* key, Expr* value, Expr* call_expr, int lineno);
Decl* funcDecl(Spec* type, Expr* name, Stmt* body, Spec* decision, int lineno);
void initASTRoot();
void addFile();
void addExpr(ExprList* expr_list, Expr* expr);
void addSpec(SpecList* spec_list, Spec* spec);
void addStmt(StmtList* stmt_list, Stmt* stmt);
void addStmtLine(StmtList* stmt_list, Stmt* stmt);
void addSpecLine(SpecList* spec_list, Spec* spec);
FuncDeclCom* funcDeclCom(Spec* func_type, Expr* ident);
void turnLastExprStmtIntoPrintStmt();

#endif
