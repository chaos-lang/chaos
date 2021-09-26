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

#include "ast.h"

extern bool interactively_importing;

ASTRoot* _ast_root = NULL;

AST* ast(int lineno)
{
    AST* ast = (struct AST*)calloc(1, sizeof(AST));
    ast->lineno = lineno;
    File* file = _ast_root->files[_ast_root->file_count - 1];
    if (is_interactive && !interactively_importing)
        file = _ast_root->files[0];
    ast->file = file;
    return ast;
}


// Expr

Expr* buildExpr(enum ExprKind kind, int lineno)
{
    Expr* expr = (struct Expr*)calloc(1, sizeof(Expr));
    expr->ast = ast(lineno);
    expr->kind = kind;
    return expr;
}

Expr* basicLitBool(bool b, int lineno)
{
    union Value value;
    value.b = b;
    BasicLit* basic_lit = (struct BasicLit*)calloc(1, sizeof(BasicLit));
    basic_lit->value_type = V_BOOL;
    basic_lit->value = value;
    Expr* expr = buildExpr(BasicLit_kind, lineno);
    expr->v.basic_lit = basic_lit;
    return expr;
}

Expr* basicLitInt(long long i, int lineno)
{
    union Value value;
    value.i = i;
    BasicLit* basic_lit = (struct BasicLit*)calloc(1, sizeof(BasicLit));
    basic_lit->value_type = V_INT;
    basic_lit->value = value;
    Expr* expr = buildExpr(BasicLit_kind, lineno);
    expr->v.basic_lit = basic_lit;
    return expr;
}

Expr* basicLitFloat(double f, int lineno)
{
    union Value value;
    value.f = f;
    BasicLit* basic_lit = (struct BasicLit*)calloc(1, sizeof(BasicLit));
    basic_lit->value_type = V_FLOAT;
    basic_lit->value = value;
    Expr* expr = buildExpr(BasicLit_kind, lineno);
    expr->v.basic_lit = basic_lit;
    return expr;
}

Expr* basicLitString(char *s, int lineno)
{
    union Value value;
    value.s = malloc(1 + strlen(s));
    strcpy(value.s, s);
    free(s);
    BasicLit* basic_lit = (struct BasicLit*)calloc(1, sizeof(BasicLit));
    basic_lit->value_type = V_STRING;
    basic_lit->value = value;
    Expr* expr = buildExpr(BasicLit_kind, lineno);
    expr->v.basic_lit = basic_lit;
    return expr;
}

Expr* ident(char *s, int lineno)
{
    Ident* ident = (struct Ident*)calloc(1, sizeof(Ident));
    ident->name = malloc(1 + strlen(s));
    strcpy(ident->name, s);
    free(s);
    Expr* expr = buildExpr(Ident_kind, lineno);
    expr->v.ident = ident;
    return expr;
}

Expr* binaryExpr(Expr* x, enum Token op, Expr* y, int lineno)
{
    BinaryExpr* binary_expr = (struct BinaryExpr*)calloc(1, sizeof(BinaryExpr));
    binary_expr->x = x;
    binary_expr->op = op;
    binary_expr->y = y;
    Expr* expr = buildExpr(BinaryExpr_kind, lineno);
    expr->v.binary_expr = binary_expr;
    return expr;
}

Expr* unaryExpr(enum Token op, Expr* x, int lineno)
{
    UnaryExpr* unary_expr = (struct UnaryExpr*)calloc(1, sizeof(UnaryExpr));
    unary_expr->op = op;
    unary_expr->x = x;
    Expr* expr = buildExpr(UnaryExpr_kind, lineno);
    expr->v.unary_expr = unary_expr;
    return expr;
}

Expr* parenExpr(Expr* x, int lineno)
{
    ParenExpr* paren_expr = (struct ParenExpr*)calloc(1, sizeof(ParenExpr));
    paren_expr->x = x;
    Expr* expr = buildExpr(ParenExpr_kind, lineno);
    expr->v.paren_expr = paren_expr;
    return expr;
}

Expr* incDecExpr(enum Token op, Expr* x, bool first, int lineno)
{
    IncDecExpr* incdec_expr = (struct IncDecExpr*)calloc(1, sizeof(IncDecExpr));
    incdec_expr->op = op;
    incdec_expr->x = x;
    incdec_expr->first = first;
    Expr* expr = buildExpr(IncDecExpr_kind, lineno);
    expr->v.incdec_expr = incdec_expr;
    return expr;
}

Expr* moduleSelector(Spec* parent_dir_spec, Expr* x, Expr* sel, int lineno)
{
    ModuleSelector* module_selector = (struct ModuleSelector*)calloc(1, sizeof(ModuleSelector));
    module_selector->parent_dir_spec = parent_dir_spec;
    module_selector->x = x;
    module_selector->sel = sel;
    Expr* expr = buildExpr(ModuleSelector_kind, lineno);
    expr->v.module_selector = module_selector;
    return expr;
}

Expr* aliasExpr(Expr* name, Expr* asname, int lineno)
{
    AliasExpr* alias_expr = (struct AliasExpr*)calloc(1, sizeof(AliasExpr));
    alias_expr->name = name;
    alias_expr->asname = asname;
    Expr* expr = buildExpr(AliasExpr_kind, lineno);
    expr->v.alias_expr = alias_expr;
    return expr;
}

Expr* indexExpr(Expr* x, Expr* index, int lineno)
{
    IndexExpr* index_expr = (struct IndexExpr*)calloc(1, sizeof(IndexExpr));
    index_expr->x = x;
    index_expr->index = index;
    Expr* expr = buildExpr(IndexExpr_kind, lineno);
    expr->v.index_expr = index_expr;
    return expr;
}

Expr* compositeLit(Spec* type, ExprList* elts, int lineno)
{
    CompositeLit* composite_lit = (struct CompositeLit*)calloc(1, sizeof(CompositeLit));
    composite_lit->type = type;
    composite_lit->elts = elts;
    Expr* expr = buildExpr(CompositeLit_kind, lineno);
    expr->v.composite_lit = composite_lit;
    return expr;
}

Expr* keyValueExpr(Expr* key, Expr* value, int lineno)
{
    KeyValueExpr* key_value_expr = (struct KeyValueExpr*)calloc(1, sizeof(KeyValueExpr));
    key_value_expr->key = key;
    key_value_expr->value = value;
    Expr* expr = buildExpr(KeyValueExpr_kind, lineno);
    expr->v.key_value_expr = key_value_expr;
    return expr;
}

Expr* selectorExpr(Expr* x, Expr* sel, int lineno)
{
    SelectorExpr* selector_expr = (struct SelectorExpr*)calloc(1, sizeof(SelectorExpr));
    selector_expr->x = x;
    selector_expr->sel = sel;
    Expr* expr = buildExpr(SelectorExpr_kind, lineno);
    expr->v.selector_expr = selector_expr;
    return expr;
}

Expr* callExpr(Expr* fun, ExprList* args, int lineno)
{
    CallExpr* call_expr = (struct CallExpr*)calloc(1, sizeof(CallExpr));
    call_expr->fun = fun;
    call_expr->args = args;
    Expr* expr = buildExpr(CallExpr_kind, lineno);
    expr->v.call_expr = call_expr;
    return expr;
}

Expr* decisionExpr(Expr* bool_expr, Stmt* outcome, int lineno)
{
    DecisionExpr* decision_expr = (struct DecisionExpr*)calloc(1, sizeof(DecisionExpr));
    decision_expr->bool_expr = bool_expr;
    decision_expr->outcome = outcome;
    Expr* expr = buildExpr(DecisionExpr_kind, lineno);
    expr->v.decision_expr = decision_expr;
    return expr;
}

Expr* defaultExpr(Stmt* outcome, int lineno)
{
    DefaultExpr* default_expr = (struct DefaultExpr*)calloc(1, sizeof(DefaultExpr));
    default_expr->outcome = outcome;
    Expr* expr = buildExpr(DefaultExpr_kind, lineno);
    expr->v.default_expr = default_expr;
    return expr;
}


// Stmt

Stmt* buildStmt(enum StmtKind kind, int lineno)
{
    Stmt* stmt = (struct Stmt*)calloc(1, sizeof(Stmt));
    stmt->ast = ast(lineno);
    stmt->kind = kind;
    return stmt;
}

Stmt* assignStmt(Expr* x, enum Token tok, Expr* y, int lineno)
{
    AssignStmt* assign_stmt = (struct AssignStmt*)calloc(1, sizeof(AssignStmt));
    assign_stmt->x = x;
    assign_stmt->tok = tok;
    assign_stmt->y = y;
    Stmt* stmt = buildStmt(AssignStmt_kind, lineno);
    stmt->v.assign_stmt = assign_stmt;
    return stmt;
}

Stmt* returnStmt(Expr* x, int lineno)
{
    ReturnStmt* return_stmt = (struct ReturnStmt*)calloc(1, sizeof(ReturnStmt));
    return_stmt->x = x;
    return_stmt->dont_push_callx = false;
    Stmt* stmt = buildStmt(ReturnStmt_kind, lineno);
    stmt->v.return_stmt = return_stmt;
    return stmt;
}

Stmt* printStmt(Spec* mod, Expr* x, int lineno)
{
    PrintStmt* print_stmt = (struct PrintStmt*)calloc(1, sizeof(PrintStmt));
    print_stmt->mod = mod;
    print_stmt->x = x;
    Stmt* stmt = buildStmt(PrintStmt_kind, lineno);
    stmt->v.print_stmt = print_stmt;
    return stmt;
}

Stmt* echoStmt(Spec* mod, Expr* x, int lineno)
{
    EchoStmt* echo_stmt = (struct EchoStmt*)calloc(1, sizeof(EchoStmt));
    echo_stmt->mod = mod;
    echo_stmt->x = x;
    Stmt* stmt = buildStmt(EchoStmt_kind, lineno);
    stmt->v.echo_stmt = echo_stmt;
    return stmt;
}

Stmt* exprStmt(Expr* x, int lineno)
{
    ExprStmt* expr_stmt = (struct ExprStmt*)calloc(1, sizeof(ExprStmt));
    expr_stmt->x = x;
    Stmt* stmt = buildStmt(ExprStmt_kind, lineno);
    stmt->v.expr_stmt = expr_stmt;
    return stmt;
}

Stmt* declStmt(Decl* decl, int lineno)
{
    DeclStmt* decl_stmt = (struct DeclStmt*)calloc(1, sizeof(DeclStmt));
    decl_stmt->decl = decl;
    Stmt* stmt = buildStmt(DeclStmt_kind, lineno);
    stmt->v.decl_stmt = decl_stmt;
    return stmt;
}

Stmt* delStmt(Expr* ident, int lineno)
{
    DelStmt* del_stmt = (struct DelStmt*)calloc(1, sizeof(DelStmt));
    del_stmt->ident = ident;
    Stmt* stmt = buildStmt(DelStmt_kind, lineno - 1);  // TODO: Why do we need `lineno - 1` here?
    stmt->v.del_stmt = del_stmt;
    return stmt;
}

Stmt* exitStmt(Expr* x, int lineno)
{
    ExitStmt* exit_stmt = (struct ExitStmt*)calloc(1, sizeof(ExitStmt));
    exit_stmt->x = x;
    Stmt* stmt = buildStmt(ExitStmt_kind, lineno - 1);  // TODO: Why do we need `lineno - 1` here?
    stmt->v.exit_stmt = exit_stmt;
    return stmt;
}

Stmt* functionTableStmt(int lineno)
{
    FunctionTableStmt* function_table_stmt = (struct FunctionTableStmt*)calloc(1, sizeof(FunctionTableStmt));
    function_table_stmt->kind = FunctionTableStmt_kind;
    Stmt* stmt = buildStmt(FunctionTableStmt_kind, lineno);
    stmt->v.function_table_stmt = function_table_stmt;
    return stmt;
}

Stmt* blockStmt(StmtList* stmt_list, int lineno)
{
    BlockStmt* block_stmt = (struct BlockStmt*)calloc(1, sizeof(BlockStmt));
    block_stmt->stmt_list = stmt_list;
    Stmt* stmt = buildStmt(BlockStmt_kind, lineno);
    stmt->v.block_stmt = block_stmt;
    return stmt;
}

Stmt* breakStmt(int lineno)
{
    BreakStmt* break_stmt = (struct BreakStmt*)calloc(1, sizeof(BreakStmt));
    break_stmt->kind = BreakStmt_kind;
    Stmt* stmt = buildStmt(BreakStmt_kind, lineno);
    stmt->v.break_stmt = break_stmt;
    return stmt;
}


// Spec

Spec* buildSpec(enum SpecKind kind, int lineno)
{
    Spec* spec = (struct Spec*)calloc(1, sizeof(Spec));
    spec->ast = ast(lineno);
    spec->kind = kind;
    return spec;
}

Spec* typeSpec(enum Type type, Spec* sub_type_spec, int lineno)
{
    TypeSpec* type_spec = (struct TypeSpec*)calloc(1, sizeof(TypeSpec));
    type_spec->type = type;
    type_spec->sub_type_spec = sub_type_spec;
    Spec* spec = buildSpec(TypeSpec_kind, lineno);
    spec->v.type_spec = type_spec;
    return spec;
}

Spec* prettySpec(int lineno)
{
    PrettySpec* pretty_spec = (struct PrettySpec*)calloc(1, sizeof(PrettySpec));
    pretty_spec->kind = PrettySpec_kind;
    Spec* spec = buildSpec(PrettySpec_kind, lineno);
    spec->v.pretty_spec = pretty_spec;
    return spec;
}

Spec* parentDirSpec(int lineno)
{
    ParentDirSpec* parent_dir_spec = (struct ParentDirSpec*)calloc(1, sizeof(ParentDirSpec));
    parent_dir_spec->kind = ParentDirSpec_kind;
    Spec* spec = buildSpec(ParentDirSpec_kind, lineno);
    spec->v.parent_dir_spec = parent_dir_spec;
    return spec;
}

Spec* asteriskSpec(int lineno)
{
    AsteriskSpec* asterisk_spec = (struct AsteriskSpec*)calloc(1, sizeof(AsteriskSpec));
    asterisk_spec->kind = AsteriskSpec_kind;
    Spec* spec = buildSpec(AsteriskSpec_kind, lineno);
    spec->v.asterisk_spec = asterisk_spec;
    return spec;
}

Spec* listType(int lineno)
{
    ListType* list_type = (struct ListType*)calloc(1, sizeof(ListType));
    list_type->kind = ListType_kind;
    Spec* spec = buildSpec(ListType_kind, lineno);
    spec->v.list_type = list_type;
    return spec;
}

Spec* dictType(int lineno)
{
    DictType* dict_type = (struct DictType*)calloc(1, sizeof(DictType));
    dict_type->kind = DictType_kind;
    Spec* spec = buildSpec(DictType_kind, lineno);
    spec->v.dict_type = dict_type;
    return spec;
}

Spec* importSpec(Expr* module_selector, Expr* ident, ExprList* names, Spec* asterisk, int lineno)
{
    ImportSpec* import_spec = (struct ImportSpec*)calloc(1, sizeof(ImportSpec));
    import_spec->module_selector = module_selector;
    import_spec->ident = ident;
    if (names == NULL) {
        names = (struct ExprList*)calloc(1, sizeof(ExprList));
        names->expr_count = 0;
    }
    import_spec->names = names;
    import_spec->asterisk = asterisk;
    import_spec->handled = false;
    Spec* spec = buildSpec(ImportSpec_kind, lineno);
    spec->v.import_spec = import_spec;
    return spec;
}

Spec* funcType(Spec* params, Spec* result, int lineno)
{
    FuncType* func_type = (struct FuncType*)calloc(1, sizeof(FuncType));
    func_type->params = params;
    func_type->result = result;
    Spec* spec = buildSpec(FuncType_kind, lineno);
    spec->v.func_type = func_type;
    return spec;
}

Spec* fieldListSpec(SpecList* list, int lineno)
{
    FieldListSpec* field_list_spec = (struct FieldListSpec*)calloc(1, sizeof(FieldListSpec));
    field_list_spec->list = list;
    Spec* spec = buildSpec(FieldListSpec_kind, lineno);
    spec->v.field_list_spec = field_list_spec;
    return spec;
}

Spec* fieldSpec(Spec* type_spec, Expr* ident, int lineno)
{
    FieldSpec* field_spec = (struct FieldSpec*)calloc(1, sizeof(FieldSpec));
    field_spec->type_spec = type_spec;
    field_spec->ident = ident;
    Spec* spec = buildSpec(FieldSpec_kind, lineno);
    spec->v.field_spec = field_spec;
    return spec;
}

Spec* optionalFieldSpec(Spec* type_spec, Expr* ident, Expr* expr, int lineno)
{
    OptionalFieldSpec* optional_field_spec = (struct OptionalFieldSpec*)calloc(1, sizeof(OptionalFieldSpec));
    optional_field_spec->type_spec = type_spec;
    optional_field_spec->ident = ident;
    optional_field_spec->expr = expr;
    Spec* spec = buildSpec(OptionalFieldSpec_kind, lineno);
    spec->v.optional_field_spec = optional_field_spec;
    return spec;
}

Spec* decisionBlock(ExprList* decisions, int lineno)
{
    DecisionBlock* decision_block = (struct DecisionBlock*)calloc(1, sizeof(DecisionBlock));
    decision_block->decisions = decisions;
    Spec* spec = buildSpec(DecisionBlock_kind, lineno);
    spec->v.decision_block = decision_block;
    return spec;
}


// Decl

Decl* buildDecl(enum DeclKind kind, int lineno)
{
    Decl* decl = (struct Decl*)calloc(1, sizeof(Decl));
    decl->ast = ast(lineno);
    decl->kind = kind;
    return decl;
}

Decl* varDecl(Spec* type_spec, Expr* ident, Expr* expr, int lineno)
{
    VarDecl* var_decl = (struct VarDecl*)calloc(1, sizeof(VarDecl));
    var_decl->type_spec = type_spec;
    var_decl->ident = ident;
    var_decl->expr = expr;
    Decl* decl = buildDecl(VarDecl_kind, lineno);
    decl->v.var_decl = var_decl;
    return decl;
}

Decl* timesDo(Expr* x, Stmt* body, int lineno)
{
    TimesDo* times_do = (struct TimesDo*)calloc(1, sizeof(TimesDo));
    times_do->x = x;
    times_do->body = body;
    Decl* decl = buildDecl(TimesDo_kind, lineno);
    decl->v.times_do = times_do;
    return decl;
}

Decl* foreachAsList(Expr* x, Expr* el, Stmt* body, int lineno)
{
    ForeachAsList* foreach_as_list = (struct ForeachAsList*)calloc(1, sizeof(ForeachAsList));
    foreach_as_list->x = x;
    foreach_as_list->el = el;
    foreach_as_list->body = body;
    Decl* decl = buildDecl(ForeachAsList_kind, lineno);
    decl->v.foreach_as_list = foreach_as_list;
    return decl;
}

Decl* foreachAsDict(Expr* x, Expr* key, Expr* value, Stmt* body, int lineno)
{
    ForeachAsDict* foreach_as_dict = (struct ForeachAsDict*)calloc(1, sizeof(ForeachAsDict));
    foreach_as_dict->x = x;
    foreach_as_dict->key = key;
    foreach_as_dict->value = value;
    foreach_as_dict->body = body;
    Decl* decl = buildDecl(ForeachAsDict_kind, lineno);
    decl->v.foreach_as_dict = foreach_as_dict;
    return decl;
}

Decl* funcDecl(Spec* type, Expr* name, Stmt* body, Spec* decision, int lineno)
{
    FuncDecl* func_decl = (struct FuncDecl*)calloc(1, sizeof(FuncDecl));
    func_decl->type = type;
    func_decl->name = name;
    func_decl->body = body;
    func_decl->decision = decision;
    Decl* decl = buildDecl(FuncDecl_kind, lineno);
    decl->v.func_decl = func_decl;
    return decl;
}


// Generic

void initASTRoot()
{
    _ast_root = (struct ASTRoot*)calloc(1, sizeof(ASTRoot));
    addFile();
}

void addFile()
{
    File* file = (struct File*)calloc(1, sizeof(File));
    file->imports_handled = false;
    StmtList* stmt_list = (struct StmtList*)calloc(1, sizeof(StmtList));
    stmt_list->stmt_count = 0;
    file->stmt_list = stmt_list;
    SpecList* imports = (struct SpecList*)calloc(1, sizeof(SpecList));
    imports->spec_count = 0;
    file->imports = imports;
    ExprList* aliases = (struct ExprList*)calloc(1, sizeof(ExprList));
    aliases->expr_count = 0;
    file->aliases = aliases;
    _ast_root->files = realloc(
        _ast_root->files,
        sizeof(File) * ++_ast_root->file_count
    );
    _ast_root->files[_ast_root->file_count - 1] = file;
}

void addExpr(ExprList* expr_list, Expr* expr)
{
    expr_list->exprs = realloc(
        expr_list->exprs,
        sizeof(Expr) * ++expr_list->expr_count
    );
    expr_list->exprs[expr_list->expr_count - 1] = expr;
}

void addStmt(StmtList* stmt_list, Stmt* stmt)
{
    stmt_list->stmts = realloc(
        stmt_list->stmts,
        sizeof(Stmt) * ++stmt_list->stmt_count
    );
    stmt_list->stmts[stmt_list->stmt_count - 1] = stmt;
}

void addSpec(SpecList* spec_list, Spec* spec)
{
    spec_list->specs = realloc(
        spec_list->specs,
        sizeof(Spec) * ++spec_list->spec_count
    );
    spec_list->specs[spec_list->spec_count - 1] = spec;
}

void addStmtLine(StmtList* stmt_list, Stmt* stmt)
{
    stmt_list->stmts = realloc(
        stmt_list->stmts,
        sizeof(Stmt) * ++stmt_list->stmt_count
    );

    for (size_t k = stmt_list->stmt_count; k > 0; k--) {
        stmt_list->stmts[k] = stmt_list->stmts[k - 1];
    }

    stmt_list->stmts[0] = stmt;
}

void addSpecLine(SpecList* spec_list, Spec* spec)
{
    spec_list->specs = realloc(
        spec_list->specs,
        sizeof(Spec) * ++spec_list->spec_count
    );

    for (size_t k = spec_list->spec_count; k > 0; k--) {
        spec_list->specs[k] = spec_list->specs[k - 1];
    }

    spec_list->specs[0] = spec;
}

FuncDeclCom* funcDeclCom(Spec* func_type, Expr* ident)
{
    FuncDeclCom* func_decl_com = (struct FuncDeclCom*)calloc(1, sizeof(FuncDeclCom));
    func_decl_com->func_type = func_type;
    func_decl_com->ident = ident;
    return func_decl_com;
}

void turnLastExprStmtIntoPrintStmt()
{
    if (_ast_root->files[0]->stmt_list->stmt_count < 1)
        return;

    Stmt* stmt = _ast_root->files[0]->stmt_list->stmts[0];
    if (stmt->kind == ExprStmt_kind && stmt->v.expr_stmt->x->kind != CallExpr_kind) {
        _ast_root->files[0]->stmt_list->stmts[0] = printStmt(NULL, stmt->v.expr_stmt->x, stmt->ast->lineno);
    }
}
