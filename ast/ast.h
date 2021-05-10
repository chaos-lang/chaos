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

typedef struct ASTNode ASTNode;
bool debug_enabled;

#include "../enums.h"
#include "../utilities/helpers.h"
#include "token.h"

enum ASTNodeType {
    AST_STEP,
    AST_VAR_CREATE_BOOL,
    AST_VAR_CREATE_BOOL_VAR,
    AST_VAR_CREATE_BOOL_VAR_EL,
    AST_VAR_CREATE_BOOL_FUNC_RETURN,
    AST_VAR_CREATE_NUMBER,
    AST_VAR_CREATE_NUMBER_VAR,
    AST_VAR_CREATE_NUMBER_VAR_EL,
    AST_VAR_CREATE_NUMBER_FUNC_RETURN,
    AST_VAR_CREATE_STRING,
    AST_VAR_CREATE_STRING_VAR,
    AST_VAR_CREATE_STRING_VAR_EL,
    AST_VAR_CREATE_STRING_FUNC_RETURN,
    AST_VAR_CREATE_ANY_BOOL,
    AST_VAR_CREATE_ANY_NUMBER,
    AST_VAR_CREATE_ANY_STRING,
    AST_VAR_CREATE_ANY_VAR,
    AST_VAR_CREATE_ANY_VAR_EL,
    AST_VAR_CREATE_ANY_FUNC_RETURN,
    AST_VAR_CREATE_LIST,
    AST_VAR_CREATE_LIST_VAR,
    AST_VAR_CREATE_LIST_FUNC_RETURN,
    AST_VAR_CREATE_DICT,
    AST_VAR_CREATE_DICT_VAR,
    AST_VAR_CREATE_DICT_FUNC_RETURN,
    AST_VAR_CREATE_BOOL_LIST,
    AST_VAR_CREATE_BOOL_LIST_VAR,
    AST_VAR_CREATE_BOOL_LIST_FUNC_RETURN,
    AST_VAR_CREATE_BOOL_DICT,
    AST_VAR_CREATE_BOOL_DICT_VAR,
    AST_VAR_CREATE_BOOL_DICT_FUNC_RETURN,
    AST_VAR_CREATE_NUMBER_LIST,
    AST_VAR_CREATE_NUMBER_LIST_VAR,
    AST_VAR_CREATE_NUMBER_LIST_FUNC_RETURN,
    AST_VAR_CREATE_NUMBER_DICT,
    AST_VAR_CREATE_NUMBER_DICT_VAR,
    AST_VAR_CREATE_NUMBER_DICT_FUNC_RETURN,
    AST_VAR_CREATE_STRING_LIST,
    AST_VAR_CREATE_STRING_LIST_VAR,
    AST_VAR_CREATE_STRING_LIST_FUNC_RETURN,
    AST_VAR_CREATE_STRING_DICT,
    AST_VAR_CREATE_STRING_DICT_VAR,
    AST_VAR_CREATE_STRING_DICT_FUNC_RETURN,
    AST_VAR_UPDATE_BOOL,
    AST_VAR_UPDATE_NUMBER,
    AST_VAR_UPDATE_STRING,
    AST_VAR_UPDATE_LIST,
    AST_VAR_UPDATE_DICT,
    AST_VAR_UPDATE_VAR,
    AST_VAR_UPDATE_VAR_EL,
    AST_VAR_UPDATE_FUNC_RETURN,
    AST_RETURN_VAR,
    AST_PRINT_COMPLEX_EL,
    AST_COMPLEX_EL_UPDATE_BOOL,
    AST_COMPLEX_EL_UPDATE_NUMBER,
    AST_COMPLEX_EL_UPDATE_STRING,
    AST_COMPLEX_EL_UPDATE_LIST,
    AST_COMPLEX_EL_UPDATE_DICT,
    AST_COMPLEX_EL_UPDATE_VAR,
    AST_COMPLEX_EL_UPDATE_VAR_EL,
    AST_COMPLEX_EL_UPDATE_FUNC_RETURN,
    AST_PRINT_VAR,
    AST_PRINT_VAR_EL,
    AST_PRINT_EXPRESSION,
    AST_PRINT_MIXED_EXPRESSION,
    AST_PRINT_STRING,
    AST_PRINT_INTERACTIVE_VAR,
    AST_PRINT_INTERACTIVE_EXPRESSION,
    AST_PRINT_INTERACTIVE_MIXED_EXPRESSION,
    AST_ECHO_VAR,
    AST_ECHO_VAR_EL,
    AST_ECHO_EXPRESSION,
    AST_ECHO_MIXED_EXPRESSION,
    AST_ECHO_STRING,
    AST_PRETTY_PRINT_VAR,
    AST_PRETTY_PRINT_VAR_EL,
    AST_PRETTY_ECHO_VAR,
    AST_PRETTY_ECHO_VAR_EL,
    AST_PARENTHESIS,
    AST_EXPRESSION_VALUE,
    AST_EXPRESSION_PLUS,
    AST_EXPRESSION_MINUS,
    AST_EXPRESSION_MULTIPLY,
    AST_EXPRESSION_BITWISE_AND,
    AST_EXPRESSION_BITWISE_OR,
    AST_EXPRESSION_BITWISE_XOR,
    AST_EXPRESSION_BITWISE_NOT,
    AST_EXPRESSION_BITWISE_LEFT_SHIFT,
    AST_EXPRESSION_BITWISE_RIGHT_SHIFT,
    AST_VAR_EXPRESSION_VALUE,
    AST_VAR_EXPRESSION_INCREMENT,
    AST_VAR_EXPRESSION_DECREMENT,
    AST_VAR_EXPRESSION_INCREMENT_ASSIGN,
    AST_VAR_EXPRESSION_ASSIGN_INCREMENT,
    AST_MIXED_EXPRESSION_VALUE,
    AST_MIXED_EXPRESSION_PLUS,
    AST_MIXED_EXPRESSION_MINUS,
    AST_MIXED_EXPRESSION_MULTIPLY,
    AST_MIXED_EXPRESSION_DIVIDE,
    AST_VAR_MIXED_EXPRESSION_VALUE,
    AST_BOOLEAN_EXPRESSION_VALUE,
    AST_BOOLEAN_EXPRESSION_REL_EQUAL,
    AST_BOOLEAN_EXPRESSION_REL_NOT_EQUAL,
    AST_BOOLEAN_EXPRESSION_REL_GREAT,
    AST_BOOLEAN_EXPRESSION_REL_SMALL,
    AST_BOOLEAN_EXPRESSION_REL_GREAT_EQUAL,
    AST_BOOLEAN_EXPRESSION_REL_SMALL_EQUAL,
    AST_BOOLEAN_EXPRESSION_LOGIC_AND,
    AST_BOOLEAN_EXPRESSION_LOGIC_OR,
    AST_BOOLEAN_EXPRESSION_LOGIC_NOT,
    AST_BOOLEAN_EXPRESSION_REL_EQUAL_MIXED,
    AST_BOOLEAN_EXPRESSION_REL_NOT_EQUAL_MIXED,
    AST_BOOLEAN_EXPRESSION_REL_GREAT_MIXED,
    AST_BOOLEAN_EXPRESSION_REL_SMALL_MIXED,
    AST_BOOLEAN_EXPRESSION_REL_GREAT_EQUAL_MIXED,
    AST_BOOLEAN_EXPRESSION_REL_SMALL_EQUAL_MIXED,
    AST_BOOLEAN_EXPRESSION_LOGIC_AND_MIXED,
    AST_BOOLEAN_EXPRESSION_LOGIC_OR_MIXED,
    AST_BOOLEAN_EXPRESSION_LOGIC_NOT_MIXED,
    AST_BOOLEAN_EXPRESSION_REL_EQUAL_MIXED_BOOLEAN,
    AST_BOOLEAN_EXPRESSION_REL_NOT_EQUAL_MIXED_BOOLEAN,
    AST_BOOLEAN_EXPRESSION_REL_GREAT_MIXED_BOOLEAN,
    AST_BOOLEAN_EXPRESSION_REL_SMALL_MIXED_BOOLEAN,
    AST_BOOLEAN_EXPRESSION_REL_GREAT_EQUAL_MIXED_BOOLEAN,
    AST_BOOLEAN_EXPRESSION_REL_SMALL_EQUAL_MIXED_BOOLEAN,
    AST_BOOLEAN_EXPRESSION_LOGIC_AND_MIXED_BOOLEAN,
    AST_BOOLEAN_EXPRESSION_LOGIC_OR_MIXED_BOOLEAN,
    AST_BOOLEAN_EXPRESSION_REL_EQUAL_BOOLEAN_MIXED,
    AST_BOOLEAN_EXPRESSION_REL_NOT_EQUAL_BOOLEAN_MIXED,
    AST_BOOLEAN_EXPRESSION_REL_GREAT_BOOLEAN_MIXED,
    AST_BOOLEAN_EXPRESSION_REL_SMALL_BOOLEAN_MIXED,
    AST_BOOLEAN_EXPRESSION_REL_GREAT_EQUAL_BOOLEAN_MIXED,
    AST_BOOLEAN_EXPRESSION_REL_SMALL_EQUAL_BOOLEAN_MIXED,
    AST_BOOLEAN_EXPRESSION_LOGIC_AND_BOOLEAN_MIXED,
    AST_BOOLEAN_EXPRESSION_LOGIC_OR_BOOLEAN_MIXED,
    AST_BOOLEAN_EXPRESSION_REL_EQUAL_EXP,
    AST_BOOLEAN_EXPRESSION_REL_NOT_EQUAL_EXP,
    AST_BOOLEAN_EXPRESSION_REL_GREAT_EXP,
    AST_BOOLEAN_EXPRESSION_REL_SMALL_EXP,
    AST_BOOLEAN_EXPRESSION_REL_GREAT_EQUAL_EXP,
    AST_BOOLEAN_EXPRESSION_REL_SMALL_EQUAL_EXP,
    AST_BOOLEAN_EXPRESSION_LOGIC_AND_EXP,
    AST_BOOLEAN_EXPRESSION_LOGIC_OR_EXP,
    AST_BOOLEAN_EXPRESSION_LOGIC_NOT_EXP,
    AST_BOOLEAN_EXPRESSION_REL_EQUAL_EXP_BOOLEAN,
    AST_BOOLEAN_EXPRESSION_REL_NOT_EQUAL_EXP_BOOLEAN,
    AST_BOOLEAN_EXPRESSION_REL_GREAT_EXP_BOOLEAN,
    AST_BOOLEAN_EXPRESSION_REL_SMALL_EXP_BOOLEAN,
    AST_BOOLEAN_EXPRESSION_REL_GREAT_EQUAL_EXP_BOOLEAN,
    AST_BOOLEAN_EXPRESSION_REL_SMALL_EQUAL_EXP_BOOLEAN,
    AST_BOOLEAN_EXPRESSION_LOGIC_AND_EXP_BOOLEAN,
    AST_BOOLEAN_EXPRESSION_LOGIC_OR_EXP_BOOLEAN,
    AST_BOOLEAN_EXPRESSION_REL_EQUAL_BOOLEAN_EXP,
    AST_BOOLEAN_EXPRESSION_REL_NOT_EQUAL_BOOLEAN_EXP,
    AST_BOOLEAN_EXPRESSION_REL_GREAT_BOOLEAN_EXP,
    AST_BOOLEAN_EXPRESSION_REL_SMALL_BOOLEAN_EXP,
    AST_BOOLEAN_EXPRESSION_REL_GREAT_EQUAL_BOOLEAN_EXP,
    AST_BOOLEAN_EXPRESSION_REL_SMALL_EQUAL_BOOLEAN_EXP,
    AST_BOOLEAN_EXPRESSION_LOGIC_AND_BOOLEAN_EXP,
    AST_BOOLEAN_EXPRESSION_LOGIC_OR_BOOLEAN_EXP,
    AST_BOOLEAN_EXPRESSION_REL_EQUAL_MIXED_EXP,
    AST_BOOLEAN_EXPRESSION_REL_NOT_EQUAL_MIXED_EXP,
    AST_BOOLEAN_EXPRESSION_REL_GREAT_MIXED_EXP,
    AST_BOOLEAN_EXPRESSION_REL_SMALL_MIXED_EXP,
    AST_BOOLEAN_EXPRESSION_REL_GREAT_EQUAL_MIXED_EXP,
    AST_BOOLEAN_EXPRESSION_REL_SMALL_EQUAL_MIXED_EXP,
    AST_BOOLEAN_EXPRESSION_LOGIC_AND_MIXED_EXP,
    AST_BOOLEAN_EXPRESSION_LOGIC_OR_MIXED_EXP,
    AST_BOOLEAN_EXPRESSION_REL_EQUAL_EXP_MIXED,
    AST_BOOLEAN_EXPRESSION_REL_NOT_EQUAL_EXP_MIXED,
    AST_BOOLEAN_EXPRESSION_REL_GREAT_EXP_MIXED,
    AST_BOOLEAN_EXPRESSION_REL_SMALL_EXP_MIXED,
    AST_BOOLEAN_EXPRESSION_REL_GREAT_EQUAL_EXP_MIXED,
    AST_BOOLEAN_EXPRESSION_REL_SMALL_EQUAL_EXP_MIXED,
    AST_BOOLEAN_EXPRESSION_LOGIC_AND_EXP_MIXED,
    AST_BOOLEAN_EXPRESSION_LOGIC_OR_EXP_MIXED,
    AST_BOOLEAN_EXPRESSION_REL_EQUAL_UNKNOWN,
    AST_BOOLEAN_EXPRESSION_REL_NOT_EQUAL_UNKNOWN,
    AST_BOOLEAN_EXPRESSION_REL_GREAT_UNKNOWN,
    AST_BOOLEAN_EXPRESSION_REL_SMALL_UNKNOWN,
    AST_BOOLEAN_EXPRESSION_REL_GREAT_EQUAL_UNKNOWN,
    AST_BOOLEAN_EXPRESSION_REL_SMALL_EQUAL_UNKNOWN,
    AST_VAR_BOOLEAN_EXPRESSION_VALUE,
    AST_DELETE_VAR,
    AST_DELETE_VAR_EL,
    AST_PRINT_SYMBOL_TABLE,
    AST_LIST_START,
    AST_LIST_ADD_VAR,
    AST_LIST_ADD_VAR_EL,
    AST_LIST_NESTED_FINISH,
    AST_DICT_START,
    AST_DICT_ADD_VAR,
    AST_DICT_ADD_VAR_EL,
    AST_DICT_NESTED_FINISH,
    AST_POP_NESTED_COMPLEX_STACK,
    AST_LEFT_RIGHT_BRACKET_EXPRESSION,
    AST_LEFT_RIGHT_BRACKET_MINUS_EXPRESSION,
    AST_LEFT_RIGHT_BRACKET_STRING,
    AST_LEFT_RIGHT_BRACKET_VAR,
    AST_LEFT_RIGHT_BRACKET_VAR_MINUS,
    AST_BUILD_COMPLEX_VARIABLE,
    AST_EXIT_SUCCESS,
    AST_EXIT_EXPRESSION,
    AST_EXIT_VAR,
    AST_START_TIMES_DO,
    AST_START_TIMES_DO_INFINITE,
    AST_START_TIMES_DO_VAR,
    AST_START_FOREACH,
    AST_START_FOREACH_DICT,
    AST_END,
    AST_PRINT_FUNCTION_TABLE,
    AST_FUNCTION_PARAMETERS_START,
    AST_FUNCTION_STEP,
    AST_FUNCTION_PARAMETER_BOOL,
    AST_FUNCTION_PARAMETER_NUMBER,
    AST_FUNCTION_PARAMETER_STRING,
    AST_FUNCTION_PARAMETER_LIST,
    AST_FUNCTION_PARAMETER_BOOL_LIST,
    AST_FUNCTION_PARAMETER_NUMBER_LIST,
    AST_FUNCTION_PARAMETER_STRING_LIST,
    AST_FUNCTION_PARAMETER_DICT,
    AST_FUNCTION_PARAMETER_BOOL_DICT,
    AST_FUNCTION_PARAMETER_NUMBER_DICT,
    AST_FUNCTION_PARAMETER_STRING_DICT,
    AST_OPTIONAL_FUNCTION_PARAMETER_BOOL,
    AST_OPTIONAL_FUNCTION_PARAMETER_NUMBER,
    AST_OPTIONAL_FUNCTION_PARAMETER_STRING,
    AST_OPTIONAL_FUNCTION_PARAMETER_LIST,
    AST_OPTIONAL_FUNCTION_PARAMETER_BOOL_LIST,
    AST_OPTIONAL_FUNCTION_PARAMETER_NUMBER_LIST,
    AST_OPTIONAL_FUNCTION_PARAMETER_STRING_LIST,
    AST_OPTIONAL_FUNCTION_PARAMETER_DICT,
    AST_OPTIONAL_FUNCTION_PARAMETER_BOOL_DICT,
    AST_OPTIONAL_FUNCTION_PARAMETER_NUMBER_DICT,
    AST_OPTIONAL_FUNCTION_PARAMETER_STRING_DICT,
    AST_FUNCTION_CALL_PARAMETERS_START,
    AST_FUNCTION_CALL_PARAMETER_BOOL,
    AST_FUNCTION_CALL_PARAMETER_NUMBER,
    AST_FUNCTION_CALL_PARAMETER_STRING,
    AST_FUNCTION_CALL_PARAMETER_VAR,
    AST_FUNCTION_CALL_PARAMETER_LIST,
    AST_FUNCTION_CALL_PARAMETER_DICT,
    AST_DEFINE_FUNCTION_BOOL,
    AST_DEFINE_FUNCTION_NUMBER,
    AST_DEFINE_FUNCTION_STRING,
    AST_DEFINE_FUNCTION_ANY,
    AST_DEFINE_FUNCTION_LIST,
    AST_DEFINE_FUNCTION_DICT,
    AST_DEFINE_FUNCTION_BOOL_LIST,
    AST_DEFINE_FUNCTION_BOOL_DICT,
    AST_DEFINE_FUNCTION_NUMBER_LIST,
    AST_DEFINE_FUNCTION_NUMBER_DICT,
    AST_DEFINE_FUNCTION_STRING_LIST,
    AST_DEFINE_FUNCTION_STRING_DICT,
    AST_DEFINE_FUNCTION_VOID,
    AST_PRINT_FUNCTION_RETURN,
    AST_ECHO_FUNCTION_RETURN,
    AST_PRETTY_PRINT_FUNCTION_RETURN,
    AST_PRETTY_ECHO_FUNCTION_RETURN,
    AST_FUNCTION_RETURN,
    AST_ADD_FUNCTION_NAME,
    AST_APPEND_MODULE,
    AST_PREPEND_MODULE,
    AST_MODULE_IMPORT,
    AST_MODULE_IMPORT_AS,
    AST_MODULE_IMPORT_PARTIAL,
    AST_NESTED_COMPLEX_TRANSITION,
    AST_DECISION_DEFINE,
    AST_DECISION_MAKE_BOOLEAN,
    AST_DECISION_MAKE_BOOLEAN_BREAK,
    AST_DECISION_MAKE_BOOLEAN_CONTINUE,
    AST_DECISION_MAKE_BOOLEAN_RETURN,
    AST_DECISION_MAKE_DEFAULT,
    AST_DECISION_MAKE_DEFAULT_BREAK,
    AST_DECISION_MAKE_DEFAULT_CONTINUE,
    AST_DECISION_MAKE_DEFAULT_RETURN,
    AST_JSON_PARSER,
};

typedef struct ASTNode {
    unsigned long long id;
    enum ASTNodeType node_type;
    int lineno;
    struct ASTNode* next;
    struct ASTNode* left;
    struct ASTNode* right;
    struct ASTNode* child;
    struct ASTNode* depend;
    char *module;
    char *transpiled;
    bool is_transpiled;
    bool dont_transpile;
    enum ValueType value_type;
    union Value value;
    size_t strings_size;
    char *strings[];
} ASTNode;

ASTNode* ast_node_cursor;
ASTNode* ast_root_node;
ASTNode* ast_node_cursor_backup;
ASTNode* ast_interactive_cursor;
bool stop_ast_evaluation;

ASTNode* addASTNodeBase(enum ASTNodeType node_type, int lineno, char *strings[], size_t strings_size, union Value value, enum ValueType value_type);
ASTNode* addASTNode(enum ASTNodeType node_type, int lineno, char *strings[], size_t strings_size);
ASTNode* addASTNodeBool(enum ASTNodeType node_type, int lineno, char *strings[], size_t strings_size, bool b, ASTNode* node);
ASTNode* addASTNodeInt(enum ASTNodeType node_type, int lineno, char *strings[], size_t strings_size, long long i, ASTNode* node);
ASTNode* addASTNodeFloat(enum ASTNodeType node_type, int lineno, char *strings[], size_t strings_size, double f, ASTNode* node);
ASTNode* addASTNodeString(enum ASTNodeType node_type, int lineno, char *strings[], size_t strings_size, char *s, ASTNode* node);
ASTNode* addASTNodeBranch(enum ASTNodeType node_type, int lineno, ASTNode* l_node, ASTNode* r_node);
ASTNode* addASTNodeAssign(enum ASTNodeType node_type, int lineno, char *strings[], size_t strings_size, ASTNode* node);
ASTNode* addASTNodeFull(enum ASTNodeType node_type, int lineno, char *strings[], size_t strings_size, ASTNode* l_node, ASTNode* r_node);
void ASTNodeNext(ASTNode* ast_node);
void ASTBranchOut();
void ASTMergeBack();
ASTNode* free_node(ASTNode* ast_node);
void setASTNodeTranspiled(ASTNode* ast_node, char* transpiled);
char* getAstNodeTypeName(unsigned i);


// NEW AST START

typedef struct AST {
    int lineno;
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
    ContinueStmt_kind=13,
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
        struct ContinueStmt* continue_stmt;
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

typedef struct ContinueStmt {
    enum StmtKind kind;
} ContinueStmt;


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
    struct Stmt* body;
} TimesDo;

typedef struct ForeachAsList {
    struct Expr* x;
    struct Expr* el;
    struct Stmt* body;
} ForeachAsList;

typedef struct ForeachAsDict {
    struct Expr* x;
    struct Expr* key;
    struct Expr* value;
    struct Stmt* body;
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
    char *module;
    char *module_path;
    char *context;
    bool imports_handled;
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
Stmt* continueStmt(int lineno);
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
Decl* timesDo(Expr* x, Stmt* body, int lineno);
Decl* foreachAsList(Expr* x, Expr* el, Stmt* body, int lineno);
Decl* foreachAsDict(Expr* x, Expr* key, Expr* value, Stmt* body, int lineno);
Decl* funcDecl(Spec* type, Expr* name, Stmt* body, Spec* decision, int lineno);
void initASTRoot();
void addFile();
void addExpr(ExprList* expr_list, Expr* expr);
void addSpec(SpecList* spec_list, Spec* spec);
void addStmt(StmtList* stmt_list, Stmt* stmt);
FuncDeclCom* funcDeclCom(Spec* func_type, Expr* ident);

#endif
