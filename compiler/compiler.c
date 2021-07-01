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

#include "compiler.h"

KaosIR* call_body_jumps;
unsigned long call_body_jumps_index = 0;
KaosIR* call_optional_jumps;
unsigned long call_optional_jumps_index = 0;

extern bool interactively_importing;

File* import_parent_context = NULL;

AST* ast_ref = 0;
i64 label_counter = 0;
int stack_counter = 0;

KaosIR* compile(ASTRoot* ast_root)
{
    KaosIR* program = initProgram();
    initCallJumps();

    // Compile imports
    compileImports(ast_root, program);

    // Declare functions in all parsed files
    declare_functions(ast_root, program);

    // Compile functions in all parsed files
    compile_functions(ast_root, program);

    StmtList* stmt_list = ast_root->files[0]->stmt_list;
    current_file_index = 0;

    // Compile other statements in the first parsed file
    push_inst_(program, PROLOG);
    for (unsigned long j = stmt_list->stmt_count; 0 < j; j--) {
        Stmt* stmt = stmt_list->stmts[j - 1];
        if (
            (stmt->kind == DeclStmt_kind && stmt->v.decl_stmt->decl->kind != FuncDecl_kind)
            ||
            (stmt->kind != DeclStmt_kind)
        )
            compileStmt(program, stmt);
    }

    push_inst_(program, HLT);
    program->hlt_count++;
    fillCallJumps(program);
    return program;
}

void initCallJumps()
{
    call_body_jumps = initProgram();
    call_optional_jumps = initProgram();
}

void fillCallJumps(KaosIR* program)
{
    // for (unsigned long i = call_optional_jumps_index; i < call_optional_jumps->size; i++) {
    //     call_optional_jumps_index++;
    //     i64 addr = call_optional_jumps->arr[i];
    //     _Function* function = (void *)program->arr[addr];
    //     program->arr[addr] = function->optional_parameters_addr;
    // }

    // for (unsigned long i = call_body_jumps_index; i < call_body_jumps->size; i++) {
    //     call_body_jumps_index++;
    //     i64 addr = call_body_jumps->arr[i];
    //     _Function* function = (void *)program->arr[addr];
    //     if (function->ref != NULL)
    //         program->arr[addr] = function->ref->body_addr;
    //     else
    //         program->arr[addr] = function->body_addr;
    // }
}

void compileImports(ASTRoot* ast_root, KaosIR* program)
{
    while (true) {
        bool all_imports_handled = true;
        for (unsigned long i = 0; i < ast_root->file_count; i++) {
            File* file = ast_root->files[i];
            import_parent_context = file;
            if (file->imports_handled)
                continue;
            all_imports_handled = false;

            SpecList* spec_list = file->imports;
            for (unsigned long i = spec_list->spec_count; 0 < i; i--) {
                Spec* spec = spec_list->specs[i - 1];
                compileSpec(program, spec);
            }
            file->imports_handled = true;
        }

        if (all_imports_handled)
            break;
    }
}

void compileStmtList(KaosIR* program, StmtList* stmt_list)
{
    for (unsigned long i = stmt_list->stmt_count; 0 < i; i--) {
        compileStmt(program, stmt_list->stmts[i - 1]);
    }
}

void compileStmt(KaosIR* program, Stmt* stmt)
{
    ast_ref = stmt->ast;

    switch (stmt->kind) {
    case EchoStmt_kind:
        compileExpr(program, stmt->v.echo_stmt->x);
        if (stmt->v.echo_stmt->mod != NULL && stmt->v.echo_stmt->mod->kind == PrettySpec_kind) {
        } else {
        }
        break;
    case PrintStmt_kind:
        compileExpr(program, stmt->v.print_stmt->x);
        if (stmt->v.print_stmt->mod != NULL && stmt->v.print_stmt->mod->kind == PrettySpec_kind) {
        } else {
        }
        push_inst_(program, PRNT);
        break;
    case ExprStmt_kind:
        compileExpr(program, stmt->v.expr_stmt->x);
        break;
    case DeclStmt_kind:
        compileDecl(program, stmt->v.decl_stmt->decl);
        break;
    case AssignStmt_kind: {
        compileExpr(program, stmt->v.assign_stmt->x);
        shift_registers(program, 4);
        compileExpr(program, stmt->v.assign_stmt->y);
        switch (stmt->v.assign_stmt->x->kind) {
        case Ident_kind: {
            Symbol* symbol_x = getSymbol(stmt->v.assign_stmt->x->v.ident->name);
            Symbol* symbol_y = NULL;
            if (stmt->v.assign_stmt->y->kind == Ident_kind)
                symbol_y = getSymbol(stmt->v.assign_stmt->y->v.ident->name);
            // i64 addr = symbol_x->addr;
            if (symbol_x->type == K_ANY)
                symbol_x->value_type = V_ANY;
            else if (symbol_x->type == K_NUMBER && symbol_y != NULL && (
                symbol_y->value_type == V_INT || symbol_y->value_type == V_FLOAT
            ))
                symbol_x->value_type = symbol_y->value_type;

            strongly_type(symbol_x, symbol_y, NULL, stmt->v.assign_stmt->y, symbol_x->value_type);

            switch (symbol_x->value_type) {
            case V_BOOL:
                push_inst_r_i(program, MOVI, R3, sizeof(long long));
                push_inst_r_r_r_i(program, STXR, R2, R3, R1, sizeof(long long));
                break;
            case V_INT:
                push_inst_r_i(program, MOVI, R3, sizeof(long long));
                push_inst_r_r_r_i(program, STXR, R2, R3, R1, sizeof(long long));
                break;
            case V_FLOAT:
                push_inst_r_i(program, MOVI, R3, sizeof(double));
                push_inst_r_r_r_i(program, FSTXR, R2, R3, R1, sizeof(double));
                break;
            case V_STRING: {
                size_t len = symbol_x->len;
                for (size_t i = 0; i < len; i++) {
                }
                break;
            }
            case V_ANY: {
                break;
            }
            case V_LIST: {
                if (symbol_x->type == K_ANY) {
                } else {
                    Symbol* symbol = getSymbol(stmt->v.assign_stmt->x->v.ident->name);
                    removeSymbol(symbol);
                    store_list(
                        program,
                        stmt->v.assign_stmt->x->v.ident->name,
                        stmt->v.assign_stmt->y->v.composite_lit->elts->expr_count,
                        false
                    );
                }
                break;
            }
            case V_DICT: {
                if (symbol_x->type == K_ANY) {
                } else {
                    Symbol* symbol = getSymbol(stmt->v.assign_stmt->x->v.ident->name);
                    removeSymbol(symbol);
                    store_dict(
                        program,
                        stmt->v.assign_stmt->x->v.ident->name,
                        stmt->v.assign_stmt->y->v.composite_lit->elts->expr_count,
                        false
                    );
                }
                break;
            }
            case V_REF: {
                break;
            }
            default:
                break;
            }
            break;
        }
        case IndexExpr_kind: {
            Symbol* symbol = getSymbol(stmt->v.assign_stmt->x->v.index_expr->x->v.ident->name);
            // i64 addr = symbol->addr;

            switch (symbol->type) {
            case K_STRING: {
                switch (stmt->v.assign_stmt->y->kind) {
                case BasicLit_kind: {
                    if (stmt->v.assign_stmt->y->v.basic_lit->value_type != V_STRING) {
                        throw_error(E_ILLEGAL_CHARACTER_ASSIGNMENT_FOR_STRING, symbol->name);
                    } else {
                        if (strlen(stmt->v.assign_stmt->y->v.basic_lit->value.s) != 1)
                            throw_error(E_NOT_A_CHARACTER, symbol->name);
                    }
                    break;
                }
                case Ident_kind: {
                    Symbol* symbol_y = getSymbol(stmt->v.assign_stmt->y->v.ident->name);
                    if (symbol_y->value_type != V_STRING)
                        throw_error(E_ILLEGAL_CHARACTER_ASSIGNMENT_FOR_STRING, symbol->name);
                    break;
                }
                default:
                    break;
                }
                break;
            }
            case K_LIST: {
                switch (stmt->v.assign_stmt->x->v.index_expr->index->kind) {
                case BasicLit_kind:
                    if (stmt->v.assign_stmt->x->v.index_expr->index->v.basic_lit->value_type != V_INT)
                        throw_error(E_UNEXPECTED_ACCESSOR_DATA_TYPE, getTypeName(symbol->type), symbol->name);
                    break;
                case Ident_kind: {
                    Symbol* symbol_y = getSymbol(stmt->v.assign_stmt->x->v.index_expr->index->v.ident->name);
                    if (symbol_y->value_type != V_INT)
                        throw_error(E_UNEXPECTED_ACCESSOR_DATA_TYPE, getTypeName(symbol->type), symbol->name);
                    break;
                }
                default:
                    break;
                }
                break;
            }
            default:
                break;
            }

            strongly_type(symbol, NULL, NULL, stmt->v.assign_stmt->y, symbol->value_type);

            switch (symbol->type) {
            case K_STRING:
                if (symbol->value_type == V_REF) {
                } else {
                }
                break;
            case K_LIST:
                if (symbol->value_type == V_REF) {
                } else {
                }
                break;
            case K_DICT:
                if (symbol->value_type == V_REF) {
                } else {
                }
                break;
            default:
                break;
            }
            break;
        }
        default:
            break;
        }
        break;
    }
    case DelStmt_kind: {
        switch (stmt->v.del_stmt->ident->kind) {
        case Ident_kind: {
            Symbol* symbol = getSymbol(stmt->v.del_stmt->ident->v.ident->name);
            removeSymbol(symbol);
            break;
        }
        case IndexExpr_kind: {
            compileExpr(program, stmt->v.del_stmt->ident->v.index_expr->index);
            Symbol* symbol = getSymbol(stmt->v.del_stmt->ident->v.index_expr->x->v.ident->name);

            if (symbol->type != K_LIST && symbol->type != K_DICT && symbol->type != K_STRING)
                throw_error(E_UNRECOGNIZED_COMPLEX_DATA_TYPE, getTypeName(symbol->type), symbol->name);
            break;
        }
        default:
            break;
        }
        break;
    }
    case BlockStmt_kind:
        compileStmtList(program, stmt->v.block_stmt->stmt_list);
        break;
    case ReturnStmt_kind: {
        ReturnStmt* return_stmt = stmt->v.return_stmt;
        enum ValueType value_type = compileExpr(program, return_stmt->x) - 1;
        function_mode->value_type = value_type;

        if (!return_stmt->dont_push_callx) {
        }
        break;
    }
    case ExitStmt_kind: {
        if (stmt->v.exit_stmt->x == NULL) {
        } else {
        }
        break;
    }
    case BreakStmt_kind: {
        break;
    }
    case ContinueStmt_kind: {
        break;
    }
    default:
        break;
    }
}

unsigned short compileExpr(KaosIR* program, Expr* expr)
{
    ast_ref = expr->ast;

    switch (expr->kind) {
    case BasicLit_kind:
        switch (expr->v.basic_lit->value_type) {
        case V_BOOL:
            push_inst_r_i(program, MOVI, R0, V_BOOL);
            push_inst_r_i(program, MOVI, R1, expr->v.basic_lit->value.b ? 1 : 0);
            break;
        case V_INT:
            push_inst_r_i(program, MOVI, R0, V_INT);
            push_inst_r_i(program, MOVI, R1, expr->v.basic_lit->value.i);
            break;
        case V_FLOAT:
            push_inst_r_i(program, MOVI, R0, V_FLOAT);
            push_inst_r_f(program, FMOV, R1, expr->v.basic_lit->value.f);
            break;
        case V_STRING: {
            break;
        }
        default:
            break;
        }
        return expr->v.basic_lit->value_type + 1;
        break;
    case Ident_kind: {
        Symbol* symbol = getSymbol(expr->v.ident->name);
        switch (symbol->value_type) {
        case V_BOOL:
            load_bool(program, symbol);
            break;
        case V_INT:
            load_int(program, symbol);
            break;
        case V_FLOAT:
            load_float(program, symbol);
            break;
        case V_STRING:
            load_string(program, symbol);
            break;
        case V_LIST:
            load_list(program, symbol);
            break;
        case V_DICT:
            load_dict(program, symbol);
            break;
        default:
            load_any(program, symbol);
            break;
        }
        return symbol->value_type + 1;
        break;
    }
    case BinaryExpr_kind: {
        enum ValueType type = compileExpr(program, expr->v.binary_expr->y);
        shift_registers(program, 4);
        // i64 addr = program->heap;
        if (expr->v.binary_expr->x->kind == ParenExpr_kind || expr->v.binary_expr->x->kind == BinaryExpr_kind) {
        }
        compileExpr(program, expr->v.binary_expr->x);
        if (expr->v.binary_expr->x->kind == ParenExpr_kind || expr->v.binary_expr->x->kind == BinaryExpr_kind) {
        }
        switch (expr->v.binary_expr->op) {
        case ADD_tok:
            push_inst_r_r_r(program, ADDR, R1, R1, R5);
            break;
        case SUB_tok:
            push_inst_r_r_r(program, SUBR, R1, R1, R5);
            break;
        case MUL_tok:
            push_inst_r_r_r(program, MULR, R1, R1, R5);
            break;
        case QUO_tok:
            push_inst_r_r_r(program, DIVR, R1, R1, R5);
            break;
        case REM_tok:
            push_inst_r_r_r(program, MODR, R1, R1, R5);
            break;
        case AND_tok:
            break;
        case OR_tok:
            break;
        case XOR_tok:
            break;
        case SHL_tok:
            break;
        case SHR_tok:
            break;
        case EQL_tok:
            break;
        case NEQ_tok:
            break;
        case GTR_tok:
            break;
        case LSS_tok:
            break;
        case GEQ_tok:
            break;
        case LEQ_tok:
            break;
        case LAND_tok:
            break;
        case LOR_tok:
            break;
        default:
            break;
        }
        return type;
        break;
    }
    case UnaryExpr_kind: {
        enum ValueType type = compileExpr(program, expr->v.unary_expr->x);
        switch (expr->v.unary_expr->op) {
        case ADD_tok:
            break;
        case SUB_tok:
            push_inst_r_r_i(program, MULI, R1, R1, -1);
            break;
        case NOT_tok:
            break;
        case TILDE_tok:
            break;
        default:
            break;
        }
        return type;
        break;
    }
    case ParenExpr_kind:
        return compileExpr(program, expr->v.paren_expr->x);
        break;
    case IndexExpr_kind: {
        enum ValueType type1 = compileExpr(program, expr->v.index_expr->x) - 1;
        shift_registers(program, 4);

        enum ValueType type2 = compileExpr(program, expr->v.index_expr->index) - 1;
        if (type1 == V_STRING) {
        } else if ((type2 == V_ANY && type1 == V_LIST) || type2 == V_INT) {
        } else if ((type2 == V_ANY && type1 == V_DICT) || type2 == V_STRING) {
        }
        return type1 + 1;
        break;
    }
    case IncDecExpr_kind: {
        enum ValueType type = compileExpr(program, expr->v.incdec_expr->x);
        switch (expr->v.incdec_expr->op) {
        case INC_tok:
            break;
        case DEC_tok:
            break;
        default:
            break;
        }
        if (expr->v.incdec_expr->x->kind == Ident_kind) {
            Symbol* symbol = getSymbol(expr->v.incdec_expr->x->v.ident->name);
            // i64 addr = symbol->addr;
            if (symbol->value_type == V_REF) {
            } else {
            }
        }
        if (!expr->v.incdec_expr->first) {
            switch (expr->v.incdec_expr->op) {
            case INC_tok:
                break;
            case DEC_tok:
                break;
            default:
                break;
            }
        }
        return type;
        break;
    }
    case CompositeLit_kind: {
        ExprList* expr_list = expr->v.composite_lit->elts;
        enum ValueType value_type = V_LIST;
        for (size_t i = 0; i < expr_list->expr_count; i++) {
            compileExpr(program, expr_list->exprs[i]);
            if (expr_list->exprs[i]->kind != KeyValueExpr_kind) {
            } else {
                value_type = V_DICT;
            }
        }
        compileSpec(program, expr->v.composite_lit->type);
        return value_type + 1;
        break;
    }
    case KeyValueExpr_kind: {
        compileExpr(program, expr->v.key_value_expr->value);
        compileExpr(program, expr->v.key_value_expr->key);
        break;
    }
    case CallExpr_kind: {
        _Function* function = NULL;
        switch (expr->v.call_expr->fun->kind) {
        case Ident_kind:
            function = getFunction(expr->v.call_expr->fun->v.ident->name, NULL);
            break;
        case SelectorExpr_kind:
            function = getFunction(
                expr->v.call_expr->fun->v.selector_expr->sel->v.ident->name,
                expr->v.call_expr->fun->v.selector_expr->x->v.ident->name
            );
            break;
        default:
            break;
        }

        ExprList* expr_list = expr->v.call_expr->args;

        if (!function->is_dynamic) {
        }

        for (unsigned long i = expr_list->expr_count; 0 < i; i--) {
            Expr* expr = expr_list->exprs[expr_list->expr_count - i];
            enum ValueType value_type = compileExpr(program, expr) - 1;
            Symbol* parameter = function->parameters[i - 1];

            strongly_type(parameter, NULL, function, expr, value_type);

            enum Type type = parameter->type;
            // i64 addr = parameter->addr;

            if (function->is_dynamic) {
                continue;
            }

            switch (type) {
            case K_BOOL:
                break;
            case K_NUMBER:
                if (value_type == V_FLOAT) {
                } else {
                }
                break;
            case K_STRING: {
                size_t len = 0;
                bool is_dynamic = false;

                switch (expr->kind) {
                case BasicLit_kind:
                    len = strlen(expr->v.basic_lit->value.s);
                    break;
                case BinaryExpr_kind:
                    is_dynamic = true;
                    break;
                case IndexExpr_kind:
                    len = 1;
                    break;
                case Ident_kind:
                    is_dynamic = true;
                    break;
                default:
                    break;
                }

                if (is_dynamic) {
                } else {
                    for (size_t i = len; i > 0; i--) {
                    }
                }
                break;
            }
            case K_ANY: {
                switch (value_type) {
                case V_BOOL:
                    break;
                case V_INT: {
                    break;
                }
                case V_FLOAT: {
                    break;
                }
                case V_STRING: {
                    size_t len = 0;
                    bool is_dynamic = false;

                    switch (expr->kind) {
                    case BasicLit_kind:
                        len = strlen(expr->v.basic_lit->value.s);
                        break;
                    case BinaryExpr_kind:
                        len =
                            strlen(expr->v.binary_expr->x->v.basic_lit->value.s)
                            +
                            strlen(expr->v.binary_expr->y->v.basic_lit->value.s);
                        break;
                    case IndexExpr_kind:
                        len = 1;
                        break;
                    case Ident_kind:
                        is_dynamic = true;
                        break;
                    default:
                        break;
                    }

                    if (is_dynamic) {
                    } else {
                        for (size_t i = len; i > 0; i--) {
                        }
                    }
                    break;
                }
                case V_LIST: {
                    size_t len = 0;
                    bool is_dynamic = false;

                    switch (expr->kind) {
                    case CompositeLit_kind:
                        len = expr->v.composite_lit->elts->expr_count;
                        break;
                    case Ident_kind:
                        is_dynamic = true;
                        break;
                    default:
                        break;
                    }

                    if (is_dynamic) {
                    } else {
                        // program->heap += len;
                        for (size_t i = len; i > 0; i--) {
                        }
                        // program->heap += len;
                    }
                    break;
                }
                case V_DICT: {
                    size_t len = 0;
                    bool is_dynamic = false;

                    switch (expr->kind) {
                    case CompositeLit_kind:
                        len = expr->v.composite_lit->elts->expr_count;
                        break;
                    case Ident_kind:
                        is_dynamic = true;
                        break;
                    default:
                        break;
                    }

                    if (is_dynamic) {
                    } else {
                        // program->heap += len * 2;
                        for (size_t i = len; i > 0; i--) {
                        }
                        // program->heap += len * 2;
                    }
                    break;
                }
                default:
                    break;
                }
                break;
            }
            case K_LIST: {
                size_t len = 0;
                bool is_dynamic = false;

                switch (expr->kind) {
                case CompositeLit_kind:
                    len = expr->v.composite_lit->elts->expr_count;
                    break;
                case Ident_kind:
                    is_dynamic = true;
                    break;
                default:
                    break;
                }

                if (is_dynamic) {
                } else {
                    // program->heap += len;
                    for (size_t i = len; i > 0; i--) {
                    }
                    // program->heap += len;
                }
                break;
            }
            case K_DICT: {
                size_t len = 0;
                bool is_dynamic = false;

                switch (expr->kind) {
                case CompositeLit_kind:
                    len = expr->v.composite_lit->elts->expr_count;
                    break;
                case Ident_kind:
                    is_dynamic = true;
                    break;
                default:
                    break;
                }

                if (is_dynamic) {
                } else {
                    // program->heap += len * 2;
                    for (size_t i = len; i > 0; i--) {
                    }
                    // program->heap += len * 2;
                }
                break;
            }
            default:
                break;
            }
        }

        if (function->is_dynamic) {
        } else {
        }
        return function->value_type + 1;
        break;
    }
    case DecisionExpr_kind: {
        compileExpr(program, expr->v.decision_expr->bool_expr);

        // This check is here to mitigate two CALLX in ReturnStmt and FuncDecl
        if (expr->v.decision_expr->outcome->kind == ReturnStmt_kind)
            expr->v.decision_expr->outcome->v.return_stmt->dont_push_callx = true;

        compileStmt(program, expr->v.decision_expr->outcome);
        break;
    }
    case DefaultExpr_kind: {
        // This check is here to mitigate two CALLX in ReturnStmt and FuncDecl
        if (expr->v.default_expr->outcome->kind == ReturnStmt_kind)
            expr->v.default_expr->outcome->v.return_stmt->dont_push_callx = true;

        compileStmt(program, expr->v.default_expr->outcome);
        break;
    }
    default:
        break;
    }

    return 0;
}

void compileDecl(KaosIR* program, Decl* decl)
{
    ast_ref = decl->ast;

    switch (decl->kind) {
    case VarDecl_kind: {
        enum ValueType value_type = compileExpr(program, decl->v.var_decl->expr) - 1;
        enum Type type = compileSpec(program, decl->v.var_decl->type_spec);
        // enum Type secondary_type = K_ANY;
        // if (decl->v.var_decl->type_spec->v.type_spec->sub_type_spec != NULL)
        //     secondary_type = decl->v.var_decl->type_spec->v.type_spec->type;
        // Symbol* symbol = NULL;

        switch (type) {
        case K_BOOL:
            store_bool(
                program,
                decl->v.var_decl->ident->v.ident->name,
                false
            );
            break;
        case K_NUMBER:
            if (value_type == V_FLOAT) {
                store_float(
                    program,
                    decl->v.var_decl->ident->v.ident->name,
                    false
                );
            } else {
                store_int(
                    program,
                    decl->v.var_decl->ident->v.ident->name,
                    false
                );
            }
            break;
        case K_STRING:
            break;
        case K_VOID:
            break;
        case K_ANY:
            break;
        case K_LIST:
            break;
        case K_DICT:
            break;
        }
        break;
    }
    case TimesDo_kind: {
        compileExpr(program, decl->v.times_do->x);

        break;
    }
    case ForeachAsList_kind: {
        compileExpr(program, decl->v.foreach_as_list->x);

        // size_t len = 0;
        // bool is_dynamic = false;

        switch (decl->v.foreach_as_list->x->kind) {
        case CompositeLit_kind:
            // len = decl->v.foreach_as_list->x->v.composite_lit->elts->expr_count;
            break;
        case Ident_kind: {
            Symbol* symbol_x = getSymbol(decl->v.foreach_as_list->x->v.ident->name);
            if (symbol_x->type != K_LIST && symbol_x->type != K_ANY)
                throw_error(E_NOT_A_LIST, symbol_x->name);
            // is_dynamic = true;
            break;
        }
        default:
            break;
        }

        // Symbol* _symbol = store_list(
        //     program,
        //     NULL,
        //     len,
        //     is_dynamic
        // );

        // Symbol* el_symbol = store_any(
        //     program,
        //     decl->v.foreach_as_list->el->v.ident->name
        // );

        compileStmt(program, decl->v.foreach_as_list->body);
        // removeSymbol(el_symbol);
        break;
    }
    case ForeachAsDict_kind: {
        compileExpr(program, decl->v.foreach_as_dict->x);
        // size_t len = 0;
        // bool is_dynamic = false;

        switch (decl->v.foreach_as_dict->x->kind) {
        case CompositeLit_kind:
            // len = decl->v.foreach_as_dict->x->v.composite_lit->elts->expr_count;
            break;
        case Ident_kind: {
            Symbol* symbol_x = getSymbol(decl->v.foreach_as_dict->x->v.ident->name);
            if (symbol_x->type != K_DICT && symbol_x->type != K_ANY)
                throw_error(E_NOT_A_DICT, symbol_x->name);
            // is_dynamic = true;
            break;
        }
        default:
            break;
        }

        // Symbol* _symbol = store_dict(
        //     program,
        //     NULL,
        //     len,
        //     is_dynamic
        // );

        // Symbol* key_symbol = store_any(
        //     program,
        //     decl->v.foreach_as_dict->key->v.ident->name
        // );

        // Symbol* value_symbol = store_any(
        //     program,
        //     decl->v.foreach_as_dict->value->v.ident->name
        // );

        compileStmt(program, decl->v.foreach_as_dict->body);

        // removeSymbol(key_symbol);
        // removeSymbol(value_symbol);
        break;
    }
    case FuncDecl_kind: {
        _Function* function = startFunctionNew(decl->v.func_decl->name->v.ident->name);
        if (function->is_compiled)
            break;

        function->body_addr = program->size - 1;
        compileStmt(program, decl->v.func_decl->body);
        if (decl->v.func_decl->decision != NULL)
            compileSpec(program, decl->v.func_decl->decision);

        function_mode->is_compiled = true;
        endFunction();
        break;
    }
    default:
        break;
    }
}

void compileSpecList(KaosIR* program, SpecList* spec_list)
{
    for (unsigned long i = spec_list->spec_count; 0 < i; i--) {
        compileSpec(program, spec_list->specs[i - 1]);
    }
}

unsigned short compileSpec(KaosIR* program, Spec* spec)
{
    ast_ref = spec->ast;

    switch (spec->kind) {
    case ListType_kind:
        break;
    case DictType_kind:
        break;
    case TypeSpec_kind:
        if (spec->v.type_spec->sub_type_spec != NULL)
            return compileSpec(program, spec->v.type_spec->sub_type_spec);
        else
            return spec->v.type_spec->type;
        break;
    case FuncType_kind:
        compileSpec(program, spec->v.func_type->params);
        compileSpec(program, spec->v.func_type->result);
        break;
    case FieldListSpec_kind:
        compileSpecList(program, spec->v.field_list_spec->list);
        break;
    case FieldSpec_kind: {
        enum Type type = compileSpec(program, spec->v.field_spec->type_spec);
        enum Type secondary_type = K_ANY;
        if (spec->v.field_spec->type_spec->v.type_spec->sub_type_spec != NULL)
            secondary_type = spec->v.field_spec->type_spec->v.type_spec->type;

        Symbol* parameter = NULL;
        union Value value;
        value.i = 0;

        switch (type) {
        case K_BOOL:
            parameter = addSymbol(spec->v.field_spec->ident->v.ident->name, K_BOOL, value, V_REF);
            break;
        case K_NUMBER:
            parameter = addSymbol(spec->v.field_spec->ident->v.ident->name, K_NUMBER, value, V_REF);
            break;
        case K_STRING:
            parameter = addSymbol(spec->v.field_spec->ident->v.ident->name, K_STRING, value, V_REF);
            break;
        case K_LIST:
            parameter = addSymbol(spec->v.field_spec->ident->v.ident->name, K_LIST, value, V_REF);
            break;
        case K_DICT:
            parameter = addSymbol(spec->v.field_spec->ident->v.ident->name, K_DICT, value, V_REF);
            break;
        case K_ANY:
            parameter = addSymbol(spec->v.field_spec->ident->v.ident->name, K_ANY, value, V_REF);
            break;
        default:
            break;
        }
        parameter->secondary_type = secondary_type;

        // parameter->addr = program->heap;
        // program->heap += 2;
        addFunctionParameterNew(function_mode, parameter);
        break;
    }
    case OptionalFieldSpec_kind: {
        enum Type type = compileSpec(program, spec->v.optional_field_spec->type_spec);
        enum Type secondary_type = K_ANY;
        if (spec->v.optional_field_spec->type_spec->v.type_spec->sub_type_spec != NULL)
            secondary_type = spec->v.optional_field_spec->type_spec->v.type_spec->type;

        Symbol* parameter = NULL;
        union Value value;
        value.i = 0;

        switch (type) {
        case K_BOOL:
            parameter = addSymbol(spec->v.field_spec->ident->v.ident->name, K_BOOL, value, V_REF);
            break;
        case K_NUMBER:
            parameter = addSymbol(spec->v.field_spec->ident->v.ident->name, K_NUMBER, value, V_REF);
            break;
        case K_STRING:
            parameter = addSymbol(spec->v.field_spec->ident->v.ident->name, K_STRING, value, V_REF);
            break;
        case K_LIST:
            parameter = addSymbol(spec->v.field_spec->ident->v.ident->name, K_LIST, value, V_REF);
            break;
        case K_DICT:
            parameter = addSymbol(spec->v.field_spec->ident->v.ident->name, K_DICT, value, V_REF);
            break;
        case K_ANY:
            parameter = addSymbol(spec->v.field_spec->ident->v.ident->name, K_ANY, value, V_REF);
            break;
        default:
            break;
        }
        parameter->secondary_type = secondary_type;

        // parameter->addr = program->heap;
        // program->heap += 2;

        addFunctionParameterNew(function_mode, parameter);

        // Load the default value
        Expr* expr = spec->v.optional_field_spec->expr;
        enum ValueType value_type = compileExpr(program, expr) - 1;
        // i64 addr = parameter->addr;

        switch (type) {
        case K_BOOL:
            break;
        case K_NUMBER:
            if (value_type == V_FLOAT) {
            } else {
            }
            break;
        case K_STRING: {
            size_t len = 0;
            bool is_dynamic = false;

            switch (expr->kind) {
            case BasicLit_kind:
                len = strlen(expr->v.basic_lit->value.s);
                break;
            case BinaryExpr_kind:
                is_dynamic = true;
                break;
            case IndexExpr_kind:
                len = 1;
                break;
            case Ident_kind:
                is_dynamic = true;
                break;
            default:
                break;
            }

            if (is_dynamic) {
            } else {
                for (size_t i = len; i > 0; i--) {
                }
            }
            break;
        }
        case K_ANY: {
            switch (value_type) {
            case V_BOOL:
                break;
            case V_INT: {
                break;
            }
            case V_FLOAT: {
                break;
            }
            case V_STRING: {
                size_t len = 0;
                bool is_dynamic = false;

                switch (expr->kind) {
                case BasicLit_kind:
                    len = strlen(expr->v.basic_lit->value.s);
                    break;
                case BinaryExpr_kind:
                    len =
                        strlen(expr->v.binary_expr->x->v.basic_lit->value.s)
                        +
                        strlen(expr->v.binary_expr->y->v.basic_lit->value.s);
                    break;
                case IndexExpr_kind:
                    len = 1;
                    break;
                case Ident_kind:
                    is_dynamic = true;
                    break;
                default:
                    break;
                }

                if (is_dynamic) {
                } else {
                    for (size_t i = len; i > 0; i--) {
                    }
                }
                break;
            }
            case V_LIST: {
                size_t len = 0;
                bool is_dynamic = false;

                switch (expr->kind) {
                case CompositeLit_kind:
                    len = expr->v.composite_lit->elts->expr_count;
                    break;
                case Ident_kind:
                    is_dynamic = true;
                    break;
                default:
                    break;
                }

                if (is_dynamic) {
                } else {
                    // program->heap += len;
                    for (size_t i = len; i > 0; i--) {
                    }
                    // program->heap += len;
                }
                break;
            }
            case V_DICT: {
                size_t len = 0;
                bool is_dynamic = false;

                switch (expr->kind) {
                case CompositeLit_kind:
                    len = expr->v.composite_lit->elts->expr_count;
                    break;
                case Ident_kind:
                    is_dynamic = true;
                    break;
                default:
                    break;
                }

                if (is_dynamic) {
                } else {
                    // program->heap += len * 2;
                    for (size_t i = len; i > 0; i--) {
                    }
                    // program->heap += len * 2;
                }
                break;
            }
            default:
                break;
            }
            break;
        }
        case K_LIST: {
            size_t len = 0;
            bool is_dynamic = false;

            switch (expr->kind) {
            case CompositeLit_kind:
                len = expr->v.composite_lit->elts->expr_count;
                break;
            case Ident_kind:
                is_dynamic = true;
                break;
            default:
                break;
            }

            if (is_dynamic) {
            } else {
                // program->heap += len;
                for (size_t i = len; i > 0; i--) {
                }
                // program->heap += len;
            }
            break;
        }
        case K_DICT: {
            size_t len = 0;
            bool is_dynamic = false;

            switch (expr->kind) {
            case CompositeLit_kind:
                len = expr->v.composite_lit->elts->expr_count;
                break;
            case Ident_kind:
                is_dynamic = true;
                break;
            default:
                break;
            }

            if (is_dynamic) {
            } else {
                // program->heap += len * 2;
                for (size_t i = len; i > 0; i--) {
                }
                // program->heap += len * 2;
            }
            break;
        }
        default:
            break;
        }
        break;
    }
    case ImportSpec_kind: {
        if (spec->v.import_spec->handled)
            break;

        char* name = compile_module_selector(spec->v.import_spec->module_selector);
        File* file = NULL;
        interactively_importing = true;
        if (spec->v.import_spec->ident != NULL) {
            file = handleModuleImport(spec->v.import_spec->ident->v.ident->name, false, import_parent_context->module_path);
        } else {
            if (spec->v.import_spec->asterisk != NULL || spec->v.import_spec->names->expr_count > 0)
                file = handleModuleImport(name, true, import_parent_context->module_path);
            else
                file = handleModuleImport(name, false, import_parent_context->module_path);
        }
        interactively_importing = false;

        for (unsigned long i = 0; i < spec->v.import_spec->names->expr_count; i++) {
            addExpr(file->aliases, spec->v.import_spec->names->exprs[i]);
        }

        spec->v.import_spec->handled = true;
        break;
    }
    case DecisionBlock_kind: {
        // i64 jump_back_point = program->size;

        ExprList* expr_list = spec->v.decision_block->decisions;
        for (unsigned long i = expr_list->expr_count; 0 < i; i--) {
            compileExpr(program, expr_list->exprs[i - 1]);
        }

        // program->arr[jump_back_point] = program->size - 1;
        break;
    }
    default:
        break;
    }
    return 0;
}

void push_inst_(KaosIR* program, enum IROpCode op_code)
{
    KaosInst* inst = malloc(sizeof *inst);
    inst->op_code = op_code;
    inst->ast = ast_ref;

    pushProgram(program, inst);
}

void push_inst_i(KaosIR* program, enum IROpCode op_code, i64 i)
{
    KaosOp* op1 = malloc(sizeof *op1);
    op1->type = IR_VAL;
    union IRValue value1;
    value1.i = i;
    op1->value = value1;

    KaosInst* inst = malloc(sizeof *inst);
    inst->op_code = op_code;
    inst->op1 = op1;
    inst->ast = ast_ref;

    pushProgram(program, inst);
}

void push_inst_i_i(KaosIR* program, enum IROpCode op_code, i64 i1, i64 i2)
{
    KaosOp* op1 = malloc(sizeof *op1);
    op1->type = IR_VAL;
    union IRValue value1;
    value1.i = i1;
    op1->value = value1;

    KaosOp* op2 = malloc(sizeof *op2);
    op2->type = IR_VAL;
    union IRValue value2;
    value2.i = i2;
    op2->value = value2;

    KaosInst* inst = malloc(sizeof *inst);
    inst->op_code = op_code;
    inst->op1 = op1;
    inst->op2 = op2;
    inst->ast = ast_ref;

    pushProgram(program, inst);
}

void push_inst_r_i(KaosIR* program, enum IROpCode op_code, enum IRRegister reg, i64 i)
{
    KaosOp* op1 = malloc(sizeof *op1);
    op1->type = IR_REG;
    op1->reg = reg;

    KaosOp* op2 = malloc(sizeof *op2);
    op2->type = IR_VAL;
    union IRValue value2;
    value2.i = i;
    op2->value = value2;

    KaosInst* inst = malloc(sizeof *inst);
    inst->op_code = op_code;
    inst->op1 = op1;
    inst->op2 = op2;
    inst->ast = ast_ref;

    pushProgram(program, inst);
}

void push_inst_r_f(KaosIR* program, enum IROpCode op_code, enum IRRegister reg, f64 f)
{
    KaosOp* op1 = malloc(sizeof *op1);
    op1->type = IR_REG;
    op1->reg = reg;

    KaosOp* op2 = malloc(sizeof *op2);
    op2->type = IR_VAL;
    union IRValue value2;
    value2.f = f;
    op2->value = value2;

    KaosInst* inst = malloc(sizeof *inst);
    inst->op_code = op_code;
    inst->op1 = op1;
    inst->op2 = op2;
    inst->ast = ast_ref;

    pushProgram(program, inst);
}

void push_inst_r_r(KaosIR* program, enum IROpCode op_code, enum IRRegister reg1, enum IRRegister reg2)
{
    KaosOp* op1 = malloc(sizeof *op1);
    op1->type = IR_REG;
    op1->reg = reg1;

    KaosOp* op2 = malloc(sizeof *op2);
    op2->type = IR_REG;
    op2->reg = reg2;

    KaosInst* inst = malloc(sizeof *inst);
    inst->op_code = op_code;
    inst->op1 = op1;
    inst->op2 = op2;
    inst->ast = ast_ref;

    pushProgram(program, inst);
}

void push_inst_r_r_i(KaosIR* program, enum IROpCode op_code, enum IRRegister reg1, enum IRRegister reg2, i64 i)
{
    KaosOp* op1 = malloc(sizeof *op1);
    op1->type = IR_REG;
    op1->reg = reg1;

    KaosOp* op2 = malloc(sizeof *op2);
    op2->type = IR_REG;
    op2->reg = reg2;

    KaosOp* op3 = malloc(sizeof *op3);
    op3->type = IR_VAL;
    union IRValue value3;
    value3.i = i;
    op3->value = value3;

    KaosInst* inst = malloc(sizeof *inst);
    inst->op_code = op_code;
    inst->op1 = op1;
    inst->op2 = op2;
    inst->op3 = op3;
    inst->ast = ast_ref;

    pushProgram(program, inst);
}

void push_inst_r_r_f(KaosIR* program, enum IROpCode op_code, enum IRRegister reg1, enum IRRegister reg2, f64 f)
{
    KaosOp* op1 = malloc(sizeof *op1);
    op1->type = IR_REG;
    op1->reg = reg1;

    KaosOp* op2 = malloc(sizeof *op2);
    op2->type = IR_REG;
    op2->reg = reg2;

    KaosOp* op3 = malloc(sizeof *op3);
    op3->type = IR_VAL;
    union IRValue value3;
    value3.f = f;
    op3->value = value3;

    KaosInst* inst = malloc(sizeof *inst);
    inst->op_code = op_code;
    inst->op1 = op1;
    inst->op2 = op2;
    inst->op3 = op3;
    inst->ast = ast_ref;

    pushProgram(program, inst);
}

void push_inst_r_r_r(KaosIR* program, enum IROpCode op_code, enum IRRegister reg1, enum IRRegister reg2, enum IRRegister reg3)
{
    KaosOp* op1 = malloc(sizeof *op1);
    op1->type = IR_REG;
    op1->reg = reg1;

    KaosOp* op2 = malloc(sizeof *op2);
    op2->type = IR_REG;
    op2->reg = reg2;

    KaosOp* op3 = malloc(sizeof *op3);
    op3->type = IR_REG;
    op3->reg = reg3;

    KaosInst* inst = malloc(sizeof *inst);
    inst->op_code = op_code;
    inst->op1 = op1;
    inst->op2 = op2;
    inst->op3 = op3;
    inst->ast = ast_ref;

    pushProgram(program, inst);
}

void push_inst_r_r_r_i(KaosIR* program, enum IROpCode op_code, enum IRRegister reg1, enum IRRegister reg2, enum IRRegister reg3, i64 i)
{
    KaosOp* op1 = malloc(sizeof *op1);
    op1->type = IR_REG;
    op1->reg = reg1;

    KaosOp* op2 = malloc(sizeof *op2);
    op2->type = IR_REG;
    op2->reg = reg2;

    KaosOp* op3 = malloc(sizeof *op3);
    op3->type = IR_REG;
    op3->reg = reg3;

    KaosOp* op4 = malloc(sizeof *op4);
    op4->type = IR_VAL;
    union IRValue value4;
    value4.i = i;
    op4->value = value4;

    KaosInst* inst = malloc(sizeof *inst);
    inst->op_code = op_code;
    inst->op1 = op1;
    inst->op2 = op2;
    inst->op3 = op3;
    inst->op4 = op4;
    inst->ast = ast_ref;

    pushProgram(program, inst);
}

void push_inst_r_r_r_f(KaosIR* program, enum IROpCode op_code, enum IRRegister reg1, enum IRRegister reg2, enum IRRegister reg3, f64 f)
{
    KaosOp* op1 = malloc(sizeof *op1);
    op1->type = IR_REG;
    op1->reg = reg1;

    KaosOp* op2 = malloc(sizeof *op2);
    op2->type = IR_REG;
    op2->reg = reg2;

    KaosOp* op3 = malloc(sizeof *op3);
    op3->type = IR_REG;
    op3->reg = reg3;

    KaosOp* op4 = malloc(sizeof *op4);
    op4->type = IR_VAL;
    union IRValue value4;
    value4.f = f;
    op4->value = value4;

    KaosInst* inst = malloc(sizeof *inst);
    inst->op_code = op_code;
    inst->op1 = op1;
    inst->op2 = op2;
    inst->op3 = op3;
    inst->op4 = op4;
    inst->ast = ast_ref;

    pushProgram(program, inst);
}

void pushProgram(KaosIR* program, KaosInst* inst)
{
    if (program->capacity == 0) {
        program->arr = (KaosInst**)malloc((++program->capacity) * sizeof(KaosInst*));
    } else {
        program->arr = (KaosInst**)realloc(program->arr, (++program->capacity) * sizeof(KaosInst*));
    }
    program->arr[program->size++] = inst;
}

KaosInst* popProgram(KaosIR* program)
{
    return program->arr[program->size--];
}

void freeProgram(KaosIR* program)
{
    free(program->arr);
    initProgram(program);
}

KaosIR* initProgram()
{
    KaosIR* program = malloc(sizeof *program);
    program->capacity = 0;
    program->arr = NULL;
    program->size = 0;
    program->hlt_count = 0;

    return program;
}

void shift_registers(KaosIR* program, size_t shift)
{
    for (size_t i = 0; i < shift; i++) {
        push_inst_r_r(program, MOVR, shift + i, i);
    }
}

Symbol* store_bool(KaosIR* program, char *name, bool is_any)
{
    union Value value;
    value.i = 0;
    Symbol* symbol;
    if (is_any)
        symbol = addSymbol(name, K_ANY, value, V_BOOL);
    else {
        symbol = addSymbol(name, K_BOOL, value, V_BOOL);
    }
    symbol->addr = stack_counter++;
    push_inst_i_i(program, ALLOCAI, symbol->addr, 2 * sizeof(long long));
    push_inst_r_i(program, REF_ALLOCAI, R2, symbol->addr);
    push_inst_r_r_i(program, STR, R2, R0, sizeof(long long));
    push_inst_r_i(program, MOVI, R3, sizeof(long long));
    push_inst_r_r_r_i(program, STXR, R2, R3, R1, sizeof(long long));

    return symbol;
}

Symbol* store_int(KaosIR* program, char *name, bool is_any)
{
    union Value value;
    value.i = 0;
    Symbol* symbol;
    if (is_any)
        symbol = addSymbol(name, K_ANY, value, V_INT);
    else {
        symbol = addSymbol(name, K_NUMBER, value, V_INT);
    }
    symbol->addr = stack_counter++;
    push_inst_i_i(program, ALLOCAI, symbol->addr, 2 * sizeof(long long));
    push_inst_r_i(program, REF_ALLOCAI, R2, symbol->addr);
    push_inst_r_r_i(program, STR, R2, R0, sizeof(long long));
    push_inst_r_i(program, MOVI, R3, sizeof(long long));
    push_inst_r_r_r_i(program, STXR, R2, R3, R1, sizeof(long long));

    return symbol;
}

Symbol* store_float(KaosIR* program, char *name, bool is_any)
{
    union Value value;
    value.i = 0;
    Symbol* symbol;
    if (is_any)
        symbol = addSymbol(name, K_ANY, value, V_FLOAT);
    else {
        symbol = addSymbol(name, K_NUMBER, value, V_FLOAT);
    }
    symbol->addr = stack_counter++;
    push_inst_i_i(program, ALLOCAI, symbol->addr, 2 * sizeof(double));
    push_inst_r_i(program, REF_ALLOCAI, R2, symbol->addr);
    push_inst_r_r_i(program, STR, R2, R0, sizeof(double));
    push_inst_r_i(program, MOVI, R3, sizeof(double));
    push_inst_r_r_r_i(program, FSTXR, R2, R3, R1, sizeof(double));

    return symbol;
}

Symbol* store_string(KaosIR* program, char *name, size_t len, bool is_any, bool is_dynamic)
{
    union Value value;
    value.i = 0;
    Symbol* symbol;
    if (is_any)
        symbol = addSymbol(name, K_ANY, value, V_STRING);
    else {
        symbol = addSymbol(name, K_STRING, value, V_STRING);
    }
    // symbol->addr = program->heap;
    symbol->len = len;
    symbol->is_dynamic = is_dynamic;

    if (is_dynamic) {
    } else {
        for (size_t i = len; i > 0; i--) {
        }
    }

    return symbol;
}

Symbol* store_list(KaosIR* program, char *name, size_t len, bool is_dynamic)
{
    union Value value;
    value.i = 0;
    Symbol* symbol = addSymbol(name, K_LIST, value, V_LIST);

    // symbol->addr = program->heap;
    symbol->len = len;
    symbol->is_dynamic = is_dynamic;

    if (is_dynamic) {
    } else {
        for (size_t i = len; i > 0; i--) {
        }
    }

    return symbol;
}

Symbol* store_dict(KaosIR* program, char *name, size_t len, bool is_dynamic)
{
    union Value value;
    value.i = 0;
    Symbol* symbol = addSymbol(name, K_DICT, value, V_DICT);

    // symbol->addr = program->heap;
    symbol->len = len;
    symbol->is_dynamic = is_dynamic;

    if (is_dynamic) {
    } else {
        for (size_t i = len; i > 0; i--) {
        }
    }

    return symbol;
}

Symbol* store_any(KaosIR* program, char *name)
{
    union Value value;
    value.i = 0;
    Symbol* symbol = addSymbol(name, K_ANY, value, V_ANY);
    // symbol->addr = program->heap;
    symbol->is_dynamic = true;

    return symbol;
}

void load_bool(KaosIR* program, Symbol* symbol)
{
    i64 addr = symbol->addr;
    push_inst_r_i(program, REF_ALLOCAI, R2, addr);
    push_inst_r_r_i(program, LDR, R0, R2, sizeof(long long));
    push_inst_r_i(program, MOVI, R3, sizeof(long long));
    push_inst_r_r_r_i(program, LDXR, R1, R2, R3, sizeof(long long));
}

void load_int(KaosIR* program, Symbol* symbol)
{
    i64 addr = symbol->addr;
    push_inst_r_i(program, REF_ALLOCAI, R2, addr);
    push_inst_r_r_i(program, LDR, R0, R2, sizeof(long long));
    push_inst_r_i(program, MOVI, R3, sizeof(long long));
    push_inst_r_r_r_i(program, LDXR, R1, R2, R3, sizeof(long long));
}

void load_float(KaosIR* program, Symbol* symbol)
{
    i64 addr = symbol->addr;
    push_inst_r_i(program, REF_ALLOCAI, R2, addr);
    push_inst_r_r_i(program, LDR, R0, R2, sizeof(double));
    push_inst_r_i(program, MOVI, R3, sizeof(double));
    push_inst_r_r_r_i(program, FLDXR, R1, R2, R3, sizeof(double));
}

void load_string(KaosIR* program, Symbol* symbol)
{
    i64 addr = symbol->addr;

    if (symbol->is_dynamic) {
    } else {
        size_t len = symbol->len;
        addr += len - 1;
        for (size_t i = len; i > 0; i--) {
        }
    }
}

void load_list(KaosIR* program, Symbol* symbol)
{
    i64 addr = symbol->addr;

    if (symbol->is_dynamic) {
    } else {
        size_t len = symbol->len;
        addr += len + 1;
        for (size_t i = len; i > 0; i--) {
        }
        addr--;
    }
}

void load_dict(KaosIR* program, Symbol* symbol)
{
    i64 addr = symbol->addr;

    if (symbol->is_dynamic) {
    } else {
        size_t len = symbol->len;
        addr += len * 2 - 1 + 2;
        for (size_t i = len; i > 0; i--) {
        }
        addr--;
    }
}

void load_any(KaosIR* program, Symbol* symbol)
{
    // i64 addr = symbol->addr;
}

char* compile_module_selector(Expr* module_selector)
{
    char* name = NULL;
    if (module_selector->v.module_selector->parent_dir_spec != NULL) {
        name = malloc(1 + strlen(".."));
        strcpy(name, "..");
    } else {
        name = module_selector->v.module_selector->x->v.ident->name;
    }
    appendModuleToModuleBuffer(name);

    if (
        module_selector->v.module_selector->sel != NULL
        &&
        module_selector->v.module_selector->sel->kind == ModuleSelector_kind
    ) {
        return compile_module_selector(module_selector->v.module_selector->sel);
    } else {
        return name;
    }
}

bool declare_function(Stmt* stmt, File* file, KaosIR* program)
{
    if (stmt->kind != DeclStmt_kind || stmt->v.decl_stmt->decl->kind != FuncDecl_kind)
        return false;

    Decl* decl = stmt->v.decl_stmt->decl;

    if (file->aliases->expr_count != 0) {
        bool _return = true;
        for (unsigned long i = 0; i < file->aliases->expr_count; i++) {
            if (strcmp(decl->v.func_decl->name->v.ident->name, file->aliases->exprs[i]->v.alias_expr->name->v.ident->name) == 0)
                _return = false;
        }

        if (_return) {
            function_mode = declareFunction(
                decl->v.func_decl->name->v.ident->name,
                "",
                file->module_path,
                file->module_path,
                K_VOID,
                K_VOID
            );

            startFunctionScope(function_mode);
            function_mode->optional_parameters_addr = program->size - 1;
            compileSpec(program, decl->v.func_decl->type->v.func_type->params);
            endFunction();

            return true;
        }
    }

    _Function* duplicate_function = (checkDuplicateFunction(
        decl->v.func_decl->name->v.ident->name,
        file->module_path
    ));

    function_mode = declareFunction(
        decl->v.func_decl->name->v.ident->name,
        file->module,
        file->module_path,
        file->context,
        K_VOID,
        K_VOID
    );

    if (duplicate_function != NULL) {
        function_mode->ref = duplicate_function;
        function_mode->parameter_count = duplicate_function->parameter_count;
        function_mode->optional_parameter_count = duplicate_function->optional_parameter_count;
        function_mode->parameters = duplicate_function->parameters;
        return true;
    }

    startFunctionScope(function_mode);
    function_mode->optional_parameters_addr = program->size - 1;
    compileSpec(program, decl->v.func_decl->type->v.func_type->params);

    if (strcmp(file->module_path, file->context) != 0) {
        _Function* context_function = declareFunction(
            decl->v.func_decl->name->v.ident->name,
            "",
            file->module_path,
            file->module_path,
            K_VOID,
            K_VOID
        );

        context_function->ref = function_mode;
        context_function->parameter_count = function_mode->parameter_count;
        context_function->optional_parameter_count = function_mode->optional_parameter_count;
        context_function->parameters = function_mode->parameters;
    }

    endFunction();

    return true;
}

void declare_functions(ASTRoot* ast_root, KaosIR* program)
{
    for (unsigned long i = 0; i < ast_root->file_count; i++) {
        File* file = ast_root->files[i];
        current_file_index = i;
        StmtList* stmt_list = file->stmt_list;
        pushModuleStack(file->module_path, file->module);

        // Declare functions
        for (unsigned long j = stmt_list->stmt_count; 0 < j; j--) {
            Stmt* stmt = stmt_list->stmts[j - 1];
            declare_function(stmt, file, program);
        }

        popModuleStack();
    }
}

void compile_functions(ASTRoot* ast_root, KaosIR* program)
{
    for (unsigned long i = 0; i < ast_root->file_count; i++) {
        File* file = ast_root->files[i];
        current_file_index = i;
        StmtList* stmt_list = file->stmt_list;
        pushModuleStack(file->module_path, file->module);

        // Compile functions
        for (unsigned long j = stmt_list->stmt_count; 0 < j; j--) {
            Stmt* stmt = stmt_list->stmts[j - 1];
            if (stmt->kind == DeclStmt_kind && stmt->v.decl_stmt->decl->kind == FuncDecl_kind) {
                compileStmt(program, stmt);
            }
        }

        popModuleStack();
    }
}

void strongly_type(Symbol* symbol_x, Symbol* symbol_y, _Function* function, Expr* expr, enum ValueType value_type)
{
    if (expr != NULL) {
        if ((symbol_x->type == K_LIST || symbol_x->type == K_DICT) && symbol_x->secondary_type != K_ANY) {
            switch (expr->kind) {
            case CompositeLit_kind: {
                CompositeLit* composite_lit = expr->v.composite_lit;
                for (unsigned long i = composite_lit->elts->expr_count; 0 < i; i--) {
                    Expr* elt = composite_lit->elts->exprs[i - 1];
                    switch (elt->kind) {
                    case CompositeLit_kind: {
                        if (function != NULL)
                            throw_error(E_ILLEGAL_VARIABLE_TYPE_FOR_FUNCTION_PARAMETER, symbol_x->name, function->name);
                        break;
                    }
                    case BasicLit_kind: {
                        BasicLit* basic_lit = elt->v.basic_lit;
                        strongly_type_basic_check(E_ILLEGAL_VARIABLE_TYPE_FOR_FUNCTION_PARAMETER, symbol_x->name, function->name, symbol_x->secondary_type, basic_lit->value_type);
                        break;
                    }
                    case KeyValueExpr_kind: {
                        KeyValueExpr* key_value_expr = elt->v.key_value_expr;
                        if (key_value_expr->value->kind == BasicLit_kind) {
                            BasicLit* basic_lit = key_value_expr->value->v.basic_lit;
                            strongly_type_basic_check(E_ILLEGAL_VARIABLE_TYPE_FOR_FUNCTION_PARAMETER, symbol_x->name, function->name, symbol_x->secondary_type, basic_lit->value_type);
                        }
                        break;
                    }
                    default:
                        break;
                    }
                }
                break;
            }
            case BasicLit_kind: {
                BasicLit* basic_lit = expr->v.basic_lit;
                strongly_type_basic_check(E_ILLEGAL_ELEMENT_TYPE_FOR_TYPED_LIST, getValueTypeName(basic_lit->value_type), symbol_x->name, symbol_x->secondary_type, basic_lit->value_type);
                break;
            }
            default:
                break;
            }
        }
    }

    if (function != NULL && value_type != V_ANY && value_type != V_REF) {
        strongly_type_basic_check(E_ILLEGAL_VARIABLE_TYPE_FOR_FUNCTION_PARAMETER, symbol_x->name, function->name, symbol_x->type, value_type);
    }
}

void strongly_type_basic_check(unsigned short code, char *str1, char *str2, enum Type type, enum ValueType value_type)
{
    switch (type) {
    case K_BOOL:
        if (value_type != V_BOOL)
            throw_error(code, str1, str2);
        break;
    case K_NUMBER:
        if (value_type != V_INT && value_type != V_FLOAT)
            throw_error(code, str1, str2);
        break;
    case K_STRING:
        if (value_type != V_STRING)
            throw_error(code, str1, str2);
        break;
    case K_LIST:
        if (value_type != V_LIST)
            throw_error(code, str1, str2);
        break;
    case K_DICT:
        if (value_type != V_DICT)
            throw_error(code, str1, str2);
        break;
    default:
        break;
    }
}
