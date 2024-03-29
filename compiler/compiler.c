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

AST* ast_ref = NULL;
i64 label_counter = 0;
i64 op_counter = 0;
int stack_counter = 0;
i64 register_offset = 0;

KaosIR* compile(ASTRoot* ast_root)
{
    KaosIR* program = initProgram();
    initCallJumps();

    // Compile imports
    compileImports(ast_root, program);

    // Declare functions in all parsed files
    declare_functions(ast_root, program);

    // Determine whether the functions should be inlined or not
    determine_inline_functions(ast_root);

    // Compile functions in all parsed files
    compile_functions(ast_root, program);

    StmtList* stmt_list = ast_root->files[0]->stmt_list;
    current_file_index = 0;

    startFunctionNew(__KAOS_MAIN_FUNCTION__);

    // Compile other statements in the first parsed file
    if (stmt_list->stmt_count > 0)
        ast_ref = stmt_list->stmts[stmt_list->stmt_count - 1]->ast;
    push_inst_(program, MAIN_PROLOG);
    for (unsigned long j = stmt_list->stmt_count; 0 < j; j--) {
        Stmt* stmt = stmt_list->stmts[j - 1];
        if (
            (stmt->kind == DeclStmt_kind && stmt->v.decl_stmt->decl->kind != FuncDecl_kind)
            ||
            (stmt->kind != DeclStmt_kind)
        )
            compileStmt(program, stmt);
    }
    push_inst_i(program, RETI, 0);

    function_mode->is_compiled = true;
    endFunction();

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
            push_inst_(program, DYN_PRETTY_ECHO);
        } else {
            push_inst_(program, DYN_ECHO);
        }
        break;
    case PrintStmt_kind:
        compileExpr(program, stmt->v.print_stmt->x);
        if (stmt->v.print_stmt->mod != NULL && stmt->v.print_stmt->mod->kind == PrettySpec_kind) {
            push_inst_(program, DYN_PRETTY_PRNT);
        } else {
            push_inst_(program, DYN_PRNT);
        }
        break;
    case ExprStmt_kind:
        compileExpr(program, stmt->v.expr_stmt->x);
        break;
    case DeclStmt_kind:
        compileDecl(program, stmt->v.decl_stmt->decl);
        break;
    case AssignStmt_kind: {
        compileExpr(program, stmt->v.assign_stmt->x);
        push_inst_r_r(program, MOVR, R12, R5);
        push_inst_r_r(program, MOVR, R13, R11);
        push_inst_r_r(program, MOVR, R9, R2);
        // shift_registers(program);
        compileExpr(program, stmt->v.assign_stmt->y);
        switch (stmt->v.assign_stmt->x->kind) {
        case Ident_kind: {
            Symbol* symbol_x = getSymbol(stmt->v.assign_stmt->x->v.ident->name);
            Symbol* symbol_y = NULL;

            enum ValueType target_value_type = 0;
            bool set_to_target_value_type = false;
            if (stmt->v.assign_stmt->y->kind == Ident_kind) {
                symbol_y = getSymbol(stmt->v.assign_stmt->y->v.ident->name);
                target_value_type = symbol_y->value_type;
                set_to_target_value_type = true;
            } else if (stmt->v.assign_stmt->y->kind == BasicLit_kind) {
                BasicLit* basic_lit = stmt->v.assign_stmt->y->v.basic_lit;
                target_value_type = basic_lit->value_type;
                set_to_target_value_type = true;
            }

            // i64 addr = symbol_x->addr;
            if (symbol_x->type == K_ANY)
                symbol_x->value_type = V_ANY;
            else if (symbol_x->type == K_NUMBER && symbol_y != NULL && (
                symbol_y->value_type == V_INT || symbol_y->value_type == V_FLOAT
            )) {
                if (symbol_y->value_type == V_FLOAT) {
                    push_inst_r_i(program, MOVI, R0, V_FLOAT);
                    push_inst_r_r_i(program, STR, R9, R0, sizeof(long long));
                } else if (symbol_y->value_type == V_INT) {
                    push_inst_r_i(program, MOVI, R0, V_INT);
                    push_inst_r_r_i(program, STR, R9, R0, sizeof(long long));
                }
                symbol_x->value_type = symbol_y->value_type;
            }

            // strongly_type(symbol_x, symbol_y, NULL, stmt->v.assign_stmt->y, symbol_x->value_type);

            if (symbol_x->type == K_ANY && set_to_target_value_type) {
                push_inst_r_i(program, MOVI, R0, target_value_type);
                push_inst_r_r_i(program, STR, R9, R0, sizeof(long long));
            }

            // printf("target_value_type: %d\n", target_value_type);
            // printf("symbol_x->value_type: %d\n", symbol_x->value_type);
            switch (symbol_x->value_type) {
            case V_BOOL:
                if (symbol_y != NULL && symbol_y->type == K_ANY) {
                    if (target_value_type == V_FLOAT)
                        push_inst_r_r(program, TRUNCR, R1, R1);
                    else if (target_value_type == V_STRING)
                        push_inst_(program, DYN_STR_TO_BOOL);
                }
                push_inst_r_i(program, MOVI, R3, sizeof(long long));
                push_inst_r_r_r_i(program, STXR, R9, R3, R1, sizeof(long long));
                break;
            case V_INT:
                push_inst_r_i(program, MOVI, R3, sizeof(long long));
                push_inst_r_r_r_i(program, STXR, R9, R3, R1, sizeof(long long));
                break;
            case V_FLOAT:
                if (symbol_y != NULL && symbol_y->type == K_ANY && target_value_type != V_FLOAT)
                    push_inst_r_r(program, EXTR, R1, R1);
                push_inst_r_i(program, MOVI, R3, sizeof(double));
                push_inst_r_r_r_i(program, FSTXR, R9, R3, R1, sizeof(double));
                break;
            case V_STRING:
                if (symbol_y != NULL && symbol_y->type == K_ANY && target_value_type == V_BOOL)
                    push_inst_(program, DYN_BOOL_TO_STR);
                push_inst_r_i(program, MOVI, R3, sizeof(long long));
                push_inst_r_r_r_i(program, STXR, R9, R3, R1, sizeof(long long));
                break;
            case V_ANY: {
                if (!set_to_target_value_type)
                    push_inst_r_r_i(program, STR, R9, R0, sizeof(long long));
                push_inst_r_i(program, MOVI, R3, sizeof(long long));
                push_inst_r_r_r_i(program, STXR, R9, R3, R1, sizeof(long long));
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
                // At first load, turn the argument into a variable in the stack
                symbol_x->addr = stack_counter++;
                push_inst_i_i(program, ALLOCAI, symbol_x->addr, 2 * sizeof(long long));
                push_inst_r_i(program, REF_ALLOCAI, R2, symbol_x->addr);
                push_inst_r_r_i(program, STR, R2, R0, sizeof(long long));
                push_inst_r_i(program, MOVI, R3, sizeof(long long));
                push_inst_r_r_r_i(program, STXR, R2, R3, R1, sizeof(long long));
                symbol_x->value_type = V_INT;

                push_inst_r_i(program, MOVI, R3, sizeof(long long));
                push_inst_r_r_r_i(program, STXR, R2, R3, R1, sizeof(long long));
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

            // strongly_type(symbol, NULL, NULL, stmt->v.assign_stmt->y, symbol->value_type);

            switch (symbol->type) {
            case K_STRING:
                push_inst_r_i(program, MOVI, R2, sizeof(size_t));
                push_inst_r_r_r_i(program, LDXR, R3, R1, R2, sizeof(char));
                push_inst_r_r_r_i(program, STXR, R5, R4, R3, sizeof(char));
                break;
            case K_LIST:
                push_inst_(program, DYN_LIST_INDEX_UPDATE);
                break;
            case K_DICT:
                push_inst_(program, DYN_DICT_KEY_UPDATE);
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
            compileExpr(program, stmt->v.del_stmt->ident->v.index_expr->x);
            push_inst_r_r(program, MOVR, R11, R1);
            compileExpr(program, stmt->v.del_stmt->ident->v.index_expr->index);
            Symbol* symbol = getSymbol(stmt->v.del_stmt->ident->v.index_expr->x->v.ident->name);

            if (symbol->type != K_LIST && symbol->type != K_DICT && symbol->type != K_STRING)
                throw_error(E_UNRECOGNIZED_COMPLEX_DATA_TYPE, getTypeName(symbol->type), symbol->name);

            switch (symbol->type) {
            case K_STRING:
                push_inst_(program, DYN_STR_INDEX_DELETE);
                break;
            case K_LIST:
                push_inst_(program, DYN_LIST_INDEX_DELETE);
                break;
            case K_DICT:
                push_inst_(program, DYN_DICT_KEY_DELETE);
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
    case BlockStmt_kind:
        compileStmtList(program, stmt->v.block_stmt->stmt_list);
        break;
    case ReturnStmt_kind: {
        ReturnStmt* return_stmt = stmt->v.return_stmt;
        enum ValueType value_type = compileExpr(program, return_stmt->x) - 1;
        function_mode->value_type = value_type;

        if (!return_stmt->dont_push_callx)
            push_inst_r(program, RETR, R1);

        break;
    }
    case ExitStmt_kind: {
        if (stmt->v.exit_stmt->x == NULL) {
            push_inst_r_i(program, MOVI, R1, 0);
        } else {
            compileExpr(program, stmt->v.exit_stmt->x);
        }
        push_inst_(program, DYN_EXIT);
        break;
    }
    case BreakStmt_kind: {
        push_inst_(program, DYN_BREAK);
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
            /*
              0      8                     size+8       size+9
              +------+ +-----------------+ +-----------------+
              | size | |     string      | | null-terminator |
              +------+ +-----------------+ +-----------------+
               size_t     size * char             char
            */
            size_t len = strlen(expr->v.basic_lit->value.s);
            i64 addr = stack_counter++;
            push_inst_i_i(program, ALLOCAI, addr, (len + 1) * sizeof(char) + sizeof(size_t));
            push_inst_r_i(program, REF_ALLOCAI, R1, addr);

            push_inst_r_i(program, MOVI, R3, len);
            push_inst_r_r_i(program, STR, R1, R3, sizeof(size_t));

            for (size_t i = 0; i < len; i++) {
                push_inst_r_i(program, MOVI, R2, i * sizeof(char) + sizeof(size_t));
                push_inst_r_i(program, MOVI, R3, expr->v.basic_lit->value.s[i]);
                push_inst_r_r_r_i(program, STXR, R1, R2, R3, sizeof(char));
            }

            push_inst_r_i(program, MOVI, R2, len * sizeof(char) + sizeof(size_t));
            push_inst_r_i(program, MOVI, R3, '\0');
            push_inst_r_r_r_i(program, STXR, R1, R2, R3, sizeof(char));

            push_inst_r_i(program, MOVI, R0, V_STRING);
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
        case V_REF:
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
        shift_registers(program);
        i64 addr = stack_counter++;
        if (expr->v.binary_expr->x->kind == ParenExpr_kind || expr->v.binary_expr->x->kind == BinaryExpr_kind) {
            push_inst_i_i(program, ALLOCAI, addr, sizeof(long long));
            push_inst_r_i(program, REF_ALLOCAI, R2, addr);
            push_inst_r_r_i(program, STR, R2, R1, sizeof(long long));
        }
        compileExpr(program, expr->v.binary_expr->x);
        if (expr->v.binary_expr->x->kind == ParenExpr_kind || expr->v.binary_expr->x->kind == BinaryExpr_kind) {
            shift_registers(program);
            push_inst_r_i(program, REF_ALLOCAI, R2, addr);
            push_inst_r_r_i(program, LDR, R5, R2, sizeof(long long));
        }
        switch (expr->v.binary_expr->op) {
        case ADD_tok:
            push_inst_(program, DYN_ADD);
            break;
        case SUB_tok:
            push_inst_(program, DYN_SUB);
            break;
        case MUL_tok:
            push_inst_(program, DYN_MUL);
            break;
        case QUO_tok:
            push_inst_(program, DYN_DIV);
            break;
        case REM_tok:
            push_inst_r_r_r(program, MODR, R1, R1, R5);
            break;
        case AND_tok:
            push_inst_r_r_r(program, ANDR, R1, R1, R5);
            break;
        case OR_tok:
            push_inst_r_r_r(program, ORR, R1, R1, R5);
            break;
        case XOR_tok:
            push_inst_r_r_r(program, XORR, R1, R1, R5);
            break;
        case SHL_tok:
            push_inst_r_r_r(program, LSHR, R1, R1, R5);
            break;
        case SHR_tok:
            push_inst_r_r_r(program, RSHR, R1, R1, R5);
            break;
        case EQL_tok:
            push_inst_(program, DYN_EQR);
            push_inst_r_i(program, MOVI, R0, V_BOOL);
            break;
        case NEQ_tok:
            push_inst_(program, DYN_NER);
            push_inst_r_i(program, MOVI, R0, V_BOOL);
            break;
        case GTR_tok:
            push_inst_(program, DYN_GTR);
            push_inst_r_i(program, MOVI, R0, V_BOOL);
            break;
        case LSS_tok:
            push_inst_(program, DYN_LTR);
            push_inst_r_i(program, MOVI, R0, V_BOOL);
            break;
        case GEQ_tok:
            push_inst_(program, DYN_GER);
            push_inst_r_i(program, MOVI, R0, V_BOOL);
            break;
        case LEQ_tok:
            push_inst_(program, DYN_LER);
            push_inst_r_i(program, MOVI, R0, V_BOOL);
            break;
        case LAND_tok:
            push_inst_(program, DYN_LAND);
            push_inst_r_i(program, MOVI, R0, V_BOOL);
            break;
        case LOR_tok:
            push_inst_(program, DYN_LOR);
            push_inst_r_i(program, MOVI, R0, V_BOOL);
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
            push_inst_(program, DYN_NEG);
            break;
        case NOT_tok:
            push_inst_(program, DYN_LNOT);
            push_inst_r_i(program, MOVI, R0, V_BOOL);
            break;
        case TILDE_tok:
            push_inst_r_r(program, NOTR, R1, R1);
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
        shift_registers(program);

        compileExpr(program, expr->v.index_expr->index);
        push_inst_r_r(program, MOVR, R11, R1);
        switch (type1) {
        case V_STRING: {
            push_inst_(program, DYN_STR_INDEX_ACCESS);
            push_inst_r_i(program, MOVI, R0, V_STRING);

            i64 len = 1;
            i64 addr = stack_counter++;
            push_inst_i_i(program, ALLOCAI, addr, (len + 1) * sizeof(char) + sizeof(size_t));
            push_inst_r_i(program, REF_ALLOCAI, R1, addr);

            push_inst_r_i(program, MOVI, R2, sizeof(size_t));
            push_inst_r_i(program, MOVI, R3, len);
            push_inst_r_r_r_i(program, STXR, R1, R2, R3, sizeof(size_t));

            push_inst_r_i(program, MOVI, R2, 0 * sizeof(char) + sizeof(size_t));
            push_inst_r_r_r_i(program, LDXR, R3, R5, R4, sizeof(char));
            push_inst_r_r_r_i(program, STXR, R1, R2, R3, sizeof(char));

            push_inst_r_i(program, MOVI, R2, len * sizeof(char) + sizeof(size_t));
            push_inst_r_i(program, MOVI, R3, '\0');
            push_inst_r_r_r_i(program, STXR, R1, R2, R3, sizeof(char));
            break;
        }
        case V_LIST:
        case V_DICT: {
            push_inst_r_r_r(program, DYN_COMP_ACCESS, R5, R4, R1);
            push_inst_r_r_i(program, LDR, R0, R2, sizeof(long long));
            push_inst_r_i(program, MOVI, R3, sizeof(long long));
            push_inst_r_r_r_i(program, LDXR, R1, R2, R3, sizeof(long long));
            push_inst_r_r_r_i(program, FLDXR, R1, R2, R3, sizeof(double));
            break;
        }
        default:
            break;
        }

        // R5 holds to pointer to size_t + string + '\n'
        // R4 holds size to the place size_t + N number of chars
        return type1 + 1;
        break;
    }
    case IncDecExpr_kind: {
        enum ValueType type = compileExpr(program, expr->v.incdec_expr->x);
        switch (expr->v.incdec_expr->op) {
        case INC_tok:
            push_inst_r_r_i(program, ADDI, R1, R1, 1);
            break;
        case DEC_tok:
            push_inst_r_r_i(program, SUBI, R1, R1, 1);
            break;
        default:
            break;
        }
        if (expr->v.incdec_expr->x->kind == Ident_kind) {
            Symbol* symbol = getSymbol(expr->v.incdec_expr->x->v.ident->name);
            // i64 addr = symbol->addr;
            if (symbol->value_type == V_REF) {
            } else {
                push_inst_r_i(program, MOVI, R3, sizeof(long long));
                push_inst_r_r_r_i(program, STXR, R2, R3, R1, sizeof(long long));
            }
        }
        if (!expr->v.incdec_expr->first) {
            switch (expr->v.incdec_expr->op) {
            case INC_tok:
                push_inst_r_r_i(program, SUBI, R1, R1, 1);
                break;
            case DEC_tok:
                push_inst_r_r_i(program, ADDI, R1, R1, 1);
                break;
            default:
                break;
            }
        }
        return type;
        break;
    }
    case CompositeLit_kind: {
        /*
          0      8              size+8
          +------+ +-----------------+
          | size | |    elements     |
          +------+ +-----------------+
           size_t      size * i64
                        ref (list)
                    key-value (dict)
        */
        ExprList* expr_list = expr->v.composite_lit->elts;
        enum ValueType value_type = expr->v.composite_lit->type->kind == ListType_kind ? V_LIST : V_DICT;
        i64 list_addr = stack_counter++;
        push_inst_i_i(program, ALLOCAI, list_addr, sizeof(size_t) + expr_list->expr_count * sizeof(long long));
        push_inst_r_i(program, REF_ALLOCAI, R10, list_addr);
        push_inst_r_i(program, MOVI, R3, expr_list->expr_count);
        push_inst_r_r_i(program, STR, R10, R3, sizeof(size_t));
        size_t j = 0;
        for (size_t i = expr_list->expr_count; 0 < i; i--) {
            compileExpr(program, expr_list->exprs[i - 1]);
            Expr* expr = expr_list->exprs[i - 1];
            i64 elt_addr = stack_counter++;

            switch (expr->kind) {
            case BasicLit_kind: {
                BasicLit* basic_lit = expr->v.basic_lit;
                switch (basic_lit->value_type) {
                case V_BOOL:
                case V_INT:
                case V_STRING:
                    push_inst_i_i(program, ALLOCAI, elt_addr, 2 * sizeof(long long));
                    push_inst_r_i(program, REF_ALLOCAI, R2, elt_addr);
                    push_inst_r_r_i(program, STR, R2, R0, sizeof(long long));
                    push_inst_r_i(program, MOVI, R3, sizeof(long long));
                    push_inst_r_r_r_i(program, STXR, R2, R3, R1, sizeof(long long));
                    break;
                case V_FLOAT:
                    push_inst_i_i(program, ALLOCAI, elt_addr, 2 * sizeof(double));
                    push_inst_r_i(program, REF_ALLOCAI, R2, elt_addr);
                    push_inst_r_r_i(program, STR, R2, R0, sizeof(double));
                    push_inst_r_i(program, MOVI, R3, sizeof(double));
                    push_inst_r_r_r_i(program, FSTXR, R2, R3, R1, sizeof(double));
                    break;
                default:
                    break;
                }
                break;
            }
            case CompositeLit_kind:
            case Ident_kind:
            case KeyValueExpr_kind: {
                push_inst_i_i(program, ALLOCAI, elt_addr, 2 * sizeof(long long));
                push_inst_r_i(program, REF_ALLOCAI, R2, elt_addr);
                push_inst_r_r_i(program, STR, R2, R0, sizeof(long long));
                push_inst_r_i(program, MOVI, R3, sizeof(long long));
                push_inst_r_r_r_i(program, STXR, R2, R3, R1, sizeof(long long));
                break;
            }
            default:
                break;
            }
            // if (expr_list->exprs[i - 1]->kind != KeyValueExpr_kind) {
            // } else {
            //     value_type = V_DICT;
            // }
            push_inst_r_i(program, REF_ALLOCAI, R10, list_addr);
            push_inst_r_i(program, MOVI, R3, sizeof(size_t) + (j++) * sizeof(long long));
            push_inst_r_r_r_i(program, STXR, R10, R3, R2, sizeof(long long));
        }
        // compileSpec(program, expr->v.composite_lit->type);
        push_inst_r_r(program, MOVR, R1, R10);
        push_inst_r_i(program, MOVI, R0, value_type);
        return value_type + 1;
        break;
    }
    case KeyValueExpr_kind: {
        /*
          0      8     16       24      32
          +------+-----+  +------+-------+
          | type | ref |  | type | value |
          +------+-----+  +------+-------+
            i64    i64      i64   i64/f64
               key             value
          ________________________________

          0      8      16
          +------+------+
          | key | value |
          +------+------+
            i64    i64
        */
        i64 key_addr = stack_counter++;
        compileExpr(program, expr->v.key_value_expr->key);
        push_inst_i_i(program, ALLOCAI, key_addr, 2 * sizeof(long long));
        push_inst_r_i(program, REF_ALLOCAI, R2, key_addr);
        push_inst_r_r_i(program, STR, R2, R0, sizeof(long long));
        push_inst_r_i(program, MOVI, R3, sizeof(long long));
        push_inst_r_r_r_i(program, STXR, R2, R3, R1, sizeof(long long));

        i64 value_addr = stack_counter++;
        enum ValueType value_type = compileExpr(program, expr->v.key_value_expr->value) - 1;
        push_inst_i_i(program, ALLOCAI, value_addr, 2 * sizeof(long long));
        push_inst_r_i(program, REF_ALLOCAI, R2, value_addr);
        push_inst_r_r_i(program, STR, R2, R0, sizeof(long long));
        push_inst_r_i(program, MOVI, R3, sizeof(long long));
        if (value_type == V_FLOAT)
            push_inst_r_r_r_i(program, FSTXR, R2, R3, R1, sizeof(double));
        else
            push_inst_r_r_r_i(program, STXR, R2, R3, R1, sizeof(long long));

        push_inst_r_i(program, REF_ALLOCAI, R0, key_addr);
        push_inst_r_i(program, REF_ALLOCAI, R1, value_addr);
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

        i64* putargr_stack = (i64*)malloc(USHRT_MAX * 256 * sizeof(i64));
        i64 putargr_stack_p = 0;

        // Scope override generics for the function inlining
        FunctionCall* scope_override_backup = NULL;
        FunctionCall* function_inline_scope = NULL;
        if (function->should_inline) {
            scope_override_backup = scope_override;
            startFunctionScope(function);
            function_inline_scope = scope_override;
            popExecutedFunctionStack();
            scope_override = scope_override_backup;
        }

        for (unsigned long i = 0; i < expr_list->expr_count; i++) {
            register_offset = i * 2;

            Expr* expr = expr_list->exprs[i];
            enum ValueType value_type = compileExpr(program, expr) - 1;
            Symbol* parameter = function->parameters[i];

            if (function->should_inline) {
                // Make the parameters available the inlined function's scope
                Symbol* symbol_upper = getSymbol(expr_list->exprs[i]->v.ident->name);
                scope_override = function_inline_scope;
                pushExecutedFunctionStack(function_inline_scope);
                Symbol* symbol_new = createCloneFromSymbol(parameter->name, symbol_upper->type, symbol_upper, symbol_upper->type);
                popExecutedFunctionStack();
                scope_override = scope_override_backup;
                symbol_new->addr = symbol_upper->addr;
            }

            // strongly_type(parameter, NULL, function, expr, value_type);

            enum Type type = parameter->type;
            // i64 addr = parameter->addr;

            if (function->is_dynamic) {
                continue;
            }

            putargr_stack[putargr_stack_p++] = R0 + register_offset;
            putargr_stack[putargr_stack_p++] = R1 + register_offset;

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

            register_offset = 0;
        }

        if (function->is_dynamic) {
        } else {
        }

        if (function->should_inline) {
            // Inline the function's body and decision block
            Decl* decl = function->ast;
            scope_override = function_inline_scope;
            pushExecutedFunctionStack(function_inline_scope);
            compileStmt(program, decl->v.func_decl->body);
            if (decl->v.func_decl->decision != NULL)
                compileSpec(program, decl->v.func_decl->decision);
            popExecutedFunctionStack();
            scope_override = scope_override_backup;
        } else {
            push_inst_(program, PREPARE);
            for (size_t i = 0; i < putargr_stack_p; i++)
                push_inst_r(program, PUTARGR, putargr_stack[i]);

            push_inst_i_i(program, CALL, function->addr, op_counter++);

            function->call_patches[function->call_patches_size++] = op_counter - 1;

            push_inst_r(program, RETVAL, R1);
            push_inst_r_i(program, MOVI, R0, 1); // TODO: temp, set it according to function return type
        }

        return function->value_type + 1;
        break;
    }
    case DecisionExpr_kind: {
        compileExpr(program, expr->v.decision_expr->bool_expr);

        i64 _op = op_counter++;

        push_inst_r_i_i(program, BEQI, R1, 0, _op);

        // This check is here to mitigate two CALLX in ReturnStmt and FuncDecl
        if (expr->v.decision_expr->outcome->kind == ReturnStmt_kind)
            expr->v.decision_expr->outcome->v.return_stmt->dont_push_callx = true;

        compileStmt(program, expr->v.decision_expr->outcome);

        push_inst_r(program, RETR, R1);

        push_inst_i(program, PATCH, _op);
        break;
    }
    case DefaultExpr_kind: {
        // This check is here to mitigate two CALLX in ReturnStmt and FuncDecl
        if (expr->v.default_expr->outcome->kind == ReturnStmt_kind)
            expr->v.default_expr->outcome->v.return_stmt->dont_push_callx = true;

        compileStmt(program, expr->v.default_expr->outcome);

        push_inst_r(program, RETR, R1);
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
        Symbol* symbol = NULL;

        switch (type) {
        case K_BOOL:
            if ((decl->v.var_decl->expr->kind != BinaryExpr_kind && decl->v.var_decl->expr->kind != UnaryExpr_kind) && value_type == V_FLOAT)
                push_inst_r_r(program, TRUNCR, R1, R1);
            else if (value_type == V_STRING)
                push_inst_(program, DYN_STR_TO_BOOL);

            if ((decl->v.var_decl->expr->kind != BinaryExpr_kind && decl->v.var_decl->expr->kind != UnaryExpr_kind))
                push_inst_r_i(program, MOVI, R0, V_BOOL);
            store_bool(
                program,
                decl->v.var_decl->ident->v.ident->name,
                false
            );
            break;
        case K_NUMBER:
            if (value_type == V_FLOAT) {
                if ((decl->v.var_decl->expr->kind != BinaryExpr_kind && decl->v.var_decl->expr->kind != UnaryExpr_kind))
                    push_inst_r_i(program, MOVI, R0, V_FLOAT);
                store_float(
                    program,
                    decl->v.var_decl->ident->v.ident->name,
                    false
                );
            } else {
                if ((decl->v.var_decl->expr->kind != BinaryExpr_kind && decl->v.var_decl->expr->kind != UnaryExpr_kind))
                    push_inst_r_i(program, MOVI, R0, V_INT);
                store_int(
                    program,
                    decl->v.var_decl->ident->v.ident->name,
                    false
                );
            }
            break;
        case K_STRING:
            if (value_type == V_BOOL)
                push_inst_(program, DYN_BOOL_TO_STR);

            if ((decl->v.var_decl->expr->kind != BinaryExpr_kind && decl->v.var_decl->expr->kind != UnaryExpr_kind))
                push_inst_r_i(program, MOVI, R0, V_STRING);
            store_string(
                program,
                decl->v.var_decl->ident->v.ident->name,
                false
            );
            break;
        case K_VOID:
            break;
        case K_ANY: {
            switch (value_type) {
            case V_BOOL:
                symbol = store_bool(
                    program,
                    decl->v.var_decl->ident->v.ident->name,
                    true
                );
                break;
            case V_INT:
                symbol = store_int(
                    program,
                    decl->v.var_decl->ident->v.ident->name,
                    true
                );
                break;
            case V_FLOAT:
                symbol = store_float(
                    program,
                    decl->v.var_decl->ident->v.ident->name,
                    true
                );
                break;
            case V_STRING:
                symbol = store_string(
                    program,
                    decl->v.var_decl->ident->v.ident->name,
                    false
                );
                break;
            case V_LIST:
                symbol = store_any(
                    program,
                    decl->v.var_decl->ident->v.ident->name
                );
                break;
            case V_DICT:
                symbol = store_any(
                    program,
                    decl->v.var_decl->ident->v.ident->name
                );
                break;
            case V_REF:
                break;
            default:
                break;
            }
            symbol->type = K_ANY;
            break;
        }
        case K_LIST: {
            size_t len = 0;
            bool is_dynamic = false;

            switch (decl->v.var_decl->expr->kind) {
            case CompositeLit_kind:
                len = decl->v.var_decl->expr->v.composite_lit->elts->expr_count;
                break;
            case Ident_kind:
                is_dynamic = true;
                break;
            default:
                break;
            }

            symbol = store_list(
                program,
                decl->v.var_decl->ident->v.ident->name,
                len,
                is_dynamic
            );
            break;
        }
        case K_DICT: {
            size_t len = 0;
            bool is_dynamic = false;

            switch (decl->v.var_decl->expr->kind) {
            case CompositeLit_kind:
                len = decl->v.var_decl->expr->v.composite_lit->elts->expr_count;
                break;
            case Ident_kind:
                is_dynamic = true;
                break;
            default:
                break;
            }

            symbol = store_dict(
                program,
                decl->v.var_decl->ident->v.ident->name,
                len,
                is_dynamic
            );
            break;
        }
        }
        break;
    }
    case TimesDo_kind: {
        Symbol* index_symbol = NULL;
        i64 len_addr = 0;

        compileExpr(program, decl->v.times_do->x);

        i64 addr = stack_counter++;
        push_inst_i_i(program, ALLOCAI, addr, 1 * sizeof(i64));
        push_inst_r_i(program, REF_ALLOCAI, R2, addr);
        push_inst_r_r_i(program, STR, R2, R1, sizeof(i64));

        if (decl->v.times_do->index != NULL) {
            len_addr = stack_counter++;
            push_inst_i_i(program, ALLOCAI, len_addr, 1 * sizeof(i64));
            push_inst_r_i(program, REF_ALLOCAI, R2, len_addr);
            push_inst_r_r_i(program, STR, R2, R1, sizeof(i64));
        }

        i64 loop_start = label_counter++;
        push_inst_i(program, DECLARE_LABEL, loop_start);

        push_inst_r_i(program, REF_ALLOCAI, R2, addr);
        push_inst_r_r_i(program, LDR, R1, R2, sizeof(i64));

        i64 loop_end = op_counter++;
        push_inst_r_i_i(program, BEQI, R1, 0, loop_end);

        push_inst_r_r_i(program, SUBI, R1, R1, 1);
        push_inst_r_r_i(program, STR, R2, R1, sizeof(i64));

        if (decl->v.times_do->index != NULL) {
            push_inst_r_i(program, REF_ALLOCAI, R2, len_addr);
            push_inst_r_r_i(program, LDR, R3, R2, sizeof(i64));
            push_inst_r_r_i(program, ADDI, R1, R1, 1);
            push_inst_r_r_r(program, SUBR, R1, R3, R1);
            push_inst_r_i(program, MOVI, R0, V_INT);

            index_symbol = store_any(
                program,
                decl->v.times_do->index->v.ident->name
            );
        }

        compileExpr(program, decl->v.times_do->call_expr);

        if (decl->v.times_do->index != NULL) {
            removeSymbol(index_symbol);
        }

        push_inst_(program, DYN_BREAK_HANDLE);
        i64 loop_end_break = op_counter++;
        push_inst_r_i_i(program, BEQI, R1, 1, loop_end_break);

        push_inst_i(program, JMPI, loop_start);
        push_inst_i(program, PATCH, loop_end);
        push_inst_i(program, PATCH, loop_end_break);
        break;
    }
    case ForeachAsList_kind: {
        Symbol* index_symbol = NULL;

        compileExpr(program, decl->v.foreach_as_list->x);

        switch (decl->v.foreach_as_list->x->kind) {
        case Ident_kind: {
            Symbol* symbol_x = getSymbol(decl->v.foreach_as_list->x->v.ident->name);
            if (symbol_x->type != K_LIST && symbol_x->type != K_ANY)
                throw_error(E_NOT_A_LIST, symbol_x->name);
            break;
        }
        default:
            break;
        }

        i64 addr = stack_counter++;
        push_inst_i_i(program, ALLOCAI, addr, 1 * sizeof(i64));
        push_inst_r_i(program, REF_ALLOCAI, R2, addr);
        push_inst_r_r_i(program, STR, R2, R1, sizeof(i64));

        push_inst_r_r(program, DYN_GET_COMP_SIZE, R1, R1);

        i64 len_addr = stack_counter++;
        push_inst_i_i(program, ALLOCAI, len_addr, 1 * sizeof(i64));
        push_inst_r_i(program, REF_ALLOCAI, R2, len_addr);
        push_inst_r_r_i(program, STR, R2, R1, sizeof(i64));

        i64 len_bak_addr = stack_counter++;
        push_inst_i_i(program, ALLOCAI, len_bak_addr, 1 * sizeof(i64));
        push_inst_r_i(program, REF_ALLOCAI, R3, len_bak_addr);
        push_inst_r_r_i(program, STR, R3, R1, sizeof(i64));

        i64 loop_start = label_counter++;
        push_inst_i(program, DECLARE_LABEL, loop_start);

        push_inst_r_i(program, REF_ALLOCAI, R2, len_addr);
        push_inst_r_r_i(program, LDR, R1, R2, sizeof(i64));

        push_inst_r_i(program, REF_ALLOCAI, R3, len_bak_addr);
        push_inst_r_r_i(program, LDR, R11, R3, sizeof(i64));
        push_inst_r_r_r(program, SUBR, R11, R11, R1);

        i64 loop_end = op_counter++;
        push_inst_r_i_i(program, BEQI, R1, 0, loop_end);

        push_inst_r_r_i(program, SUBI, R1, R1, 1);
        push_inst_r_r_i(program, STR, R2, R1, sizeof(i64));

        if (decl->v.foreach_as_list->index != NULL) {
            push_inst_r_r_i(program, LDR, R3, R3, sizeof(i64));
            push_inst_r_r_i(program, ADDI, R1, R1, 1);
            push_inst_r_r_r(program, SUBR, R1, R3, R1);
            push_inst_r_i(program, MOVI, R0, V_INT);

            index_symbol = store_any(
                program,
                decl->v.foreach_as_list->index->v.ident->name
            );
        }

        push_inst_r_i(program, MOVI, R0, V_LIST);
        push_inst_r_i(program, REF_ALLOCAI, R2, addr);
        push_inst_r_r_i(program, LDR, R1, R2, sizeof(i64));

        push_inst_r_r_r(program, DYN_COMP_ACCESS, R1, R0, R11);
        push_inst_r_r_i(program, LDR, R0, R2, sizeof(long long));
        push_inst_r_i(program, MOVI, R3, sizeof(long long));
        push_inst_r_r_r_i(program, LDXR, R1, R2, R3, sizeof(long long));
        push_inst_r_r_r_i(program, FLDXR, R1, R2, R3, sizeof(double));

        Symbol* el_symbol = store_any(
            program,
            decl->v.foreach_as_list->el->v.ident->name
        );

        compileExpr(program, decl->v.foreach_as_list->call_expr);

        if (decl->v.foreach_as_list->index != NULL) {
            removeSymbol(index_symbol);
        }

        removeSymbol(el_symbol);

        push_inst_(program, DYN_BREAK_HANDLE);
        i64 loop_end_break = op_counter++;
        push_inst_r_i_i(program, BEQI, R1, 1, loop_end_break);

        push_inst_i(program, JMPI, loop_start);
        push_inst_i(program, PATCH, loop_end);
        push_inst_i(program, PATCH, loop_end_break);
        break;
    }
    case ForeachAsDict_kind: {
        Symbol* index_symbol = NULL;

        compileExpr(program, decl->v.foreach_as_dict->x);

        switch (decl->v.foreach_as_dict->x->kind) {
        case Ident_kind: {
            Symbol* symbol_x = getSymbol(decl->v.foreach_as_dict->x->v.ident->name);
            if (symbol_x->type != K_DICT && symbol_x->type != K_ANY)
                throw_error(E_NOT_A_DICT, symbol_x->name);
            break;
        }
        default:
            break;
        }

        i64 addr = stack_counter++;
        push_inst_i_i(program, ALLOCAI, addr, 1 * sizeof(i64));
        push_inst_r_i(program, REF_ALLOCAI, R2, addr);
        push_inst_r_r_i(program, STR, R2, R1, sizeof(i64));

        push_inst_r_r(program, DYN_GET_COMP_SIZE, R1, R1);

        i64 len_addr = stack_counter++;
        push_inst_i_i(program, ALLOCAI, len_addr, 1 * sizeof(i64));
        push_inst_r_i(program, REF_ALLOCAI, R2, len_addr);
        push_inst_r_r_i(program, STR, R2, R1, sizeof(i64));

        i64 len_bak_addr = stack_counter++;
        push_inst_i_i(program, ALLOCAI, len_bak_addr, 1 * sizeof(i64));
        push_inst_r_i(program, REF_ALLOCAI, R3, len_bak_addr);
        push_inst_r_r_i(program, STR, R3, R1, sizeof(i64));

        i64 loop_start = label_counter++;
        push_inst_i(program, DECLARE_LABEL, loop_start);

        push_inst_r_i(program, REF_ALLOCAI, R2, len_addr);
        push_inst_r_r_i(program, LDR, R1, R2, sizeof(i64));

        push_inst_r_i(program, REF_ALLOCAI, R3, len_bak_addr);
        push_inst_r_r_i(program, LDR, R11, R3, sizeof(i64));
        push_inst_r_r_r(program, SUBR, R11, R11, R1);

        i64 loop_end = op_counter++;
        push_inst_r_i_i(program, BEQI, R1, 0, loop_end);

        push_inst_r_r_i(program, SUBI, R1, R1, 1);
        push_inst_r_r_i(program, STR, R2, R1, sizeof(i64));

        if (decl->v.foreach_as_dict->index != NULL) {
            push_inst_r_r_i(program, LDR, R3, R3, sizeof(i64));
            push_inst_r_r_i(program, ADDI, R1, R1, 1);
            push_inst_r_r_r(program, SUBR, R1, R3, R1);
            push_inst_r_i(program, MOVI, R0, V_INT);

            index_symbol = store_any(
                program,
                decl->v.foreach_as_dict->index->v.ident->name
            );
        }

        // Don't worry `V_LIST` was intentional
        push_inst_r_i(program, MOVI, R0, V_LIST);
        push_inst_r_i(program, REF_ALLOCAI, R2, addr);
        push_inst_r_r_i(program, LDR, R1, R2, sizeof(i64));

        push_inst_r_r_r(program, DYN_COMP_ACCESS, R1, R0, R11);
        push_inst_r_r_i(program, LDR, R11, R2, sizeof(long long));
        push_inst_r_i(program, MOVI, R3, sizeof(long long));
        push_inst_r_r_r_i(program, LDXR, R12, R2, R3, sizeof(long long));

        push_inst_r_r_i(program, LDR, R0, R11, sizeof(long long));
        push_inst_r_i(program, MOVI, R3, sizeof(long long));
        push_inst_r_r_r_i(program, LDXR, R1, R11, R3, sizeof(long long));

        Symbol* key_symbol = store_any(
            program,
            decl->v.foreach_as_dict->key->v.ident->name
        );

        push_inst_r_r_i(program, LDR, R0, R12, sizeof(long long));
        push_inst_r_i(program, MOVI, R3, sizeof(long long));
        push_inst_r_r_r_i(program, LDXR, R1, R12, R3, sizeof(long long));
        push_inst_r_r_r_i(program, FLDXR, R1, R12, R3, sizeof(double));

        Symbol* value_symbol = store_any(
            program,
            decl->v.foreach_as_dict->value->v.ident->name
        );

        compileExpr(program, decl->v.foreach_as_dict->call_expr);

        if (decl->v.foreach_as_dict->index != NULL) {
            removeSymbol(index_symbol);
        }

        removeSymbol(key_symbol);
        removeSymbol(value_symbol);

        push_inst_(program, DYN_BREAK_HANDLE);
        i64 loop_end_break = op_counter++;
        push_inst_r_i_i(program, BEQI, R1, 1, loop_end_break);

        push_inst_i(program, JMPI, loop_start);
        push_inst_i(program, PATCH, loop_end);
        push_inst_i(program, PATCH, loop_end_break);
        break;
    }
    case FuncDecl_kind: {
        _Function* function = startFunctionNew(decl->v.func_decl->name->v.ident->name);
        if (function->should_inline) {
            function->ast = decl;
            function_mode->is_compiled = true;
            endFunction();
            break;
        }

        function->ast = decl;

        if (function->is_compiled)
            break;

        for (int i = 0; i < function->call_patches_size; i++)
            push_inst_i(program, PATCH, function->call_patches[i]);

        push_inst_i(program, PROLOG, function->addr);

        compileSpec(program, decl->v.func_decl->type->v.func_type->params);

        for (int i = 0; i < function->parameter_count; i++) {
            Symbol* parameter = function->parameters[i];
            push_inst_r_i(program, GETARG, R0, (i * 2));
            push_inst_r_i(program, GETARG, R1, (i * 2) + 1);

            parameter->addr = stack_counter++;
            push_inst_i_i(program, ALLOCAI, parameter->addr, 2 * sizeof(long long));
            push_inst_r_i(program, REF_ALLOCAI, R2, parameter->addr);
            push_inst_r_r_i(program, STR, R2, R0, sizeof(long long));
            push_inst_r_i(program, MOVI, R3, sizeof(long long));
            push_inst_r_r_r_i(program, STXR, R2, R3, R1, sizeof(long long));
            parameter->value_type = V_INT;  // TODO: temp, set it according to parameter type
        }

        compileStmt(program, decl->v.func_decl->body);
        if (decl->v.func_decl->decision != NULL)
            compileSpec(program, decl->v.func_decl->decision);

        function_mode->is_compiled = true;
        endFunction();

        push_inst_i(program, RETI, 0);  // TODO: should we remove it?
        break;
    }
    default:
        break;
    }
}

void declareSpecList(KaosIR* program, SpecList* spec_list)
{
    for (unsigned long i = 0; i < spec_list->spec_count; i++) {
        declareSpec(program, spec_list->specs[i]);
    }
}

void compileSpecList(KaosIR* program, SpecList* spec_list)
{
    for (unsigned long i = 0; i < spec_list->spec_count; i++) {
        compileSpec(program, spec_list->specs[i]);
    }
}

unsigned short declareSpec(KaosIR* program, Spec* spec)
{
    switch (spec->kind) {
        case FieldListSpec_kind:
            declareSpecList(program, spec->v.field_list_spec->list);
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
        default:
            break;
    }
    return 0;
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

        push_inst_i_i(program, DECLARE_ARG, JIT_UNSIGNED_NUM, sizeof(i64));

        switch (type) {
        case K_BOOL:
            push_inst_i_i(program, DECLARE_ARG, JIT_UNSIGNED_NUM, sizeof(i64));
            break;
        case K_NUMBER:
            push_inst_i_i(program, DECLARE_ARG, JIT_UNSIGNED_NUM, sizeof(i64));
            break;
        case K_STRING:
            push_inst_i_i(program, DECLARE_ARG, JIT_UNSIGNED_NUM, sizeof(i64));
            break;
        case K_LIST:
            push_inst_i_i(program, DECLARE_ARG, JIT_UNSIGNED_NUM, sizeof(i64));
            break;
        case K_DICT:
            push_inst_i_i(program, DECLARE_ARG, JIT_UNSIGNED_NUM, sizeof(i64));
            break;
        case K_ANY:
            break;
        default:
            break;
        }
        break;
    }
    case OptionalFieldSpec_kind:
        break;
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

void push_inst_r(KaosIR* program, enum IROpCode op_code, enum IRRegister reg)
{
    reg += register_offset;

    KaosOp* op1 = malloc(sizeof *op1);
    op1->type = IR_REG;
    op1->reg = reg;

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
    reg += register_offset;

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
    reg += register_offset;

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
    reg1 += register_offset;
    reg2 += register_offset;

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
    reg1 += register_offset;
    reg2 += register_offset;

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

void push_inst_r_i_i(KaosIR* program, enum IROpCode op_code, enum IRRegister reg1, i64 i1, i64 i2)
{
    reg1 += register_offset;

    KaosOp* op1 = malloc(sizeof *op1);
    op1->type = IR_REG;
    op1->reg = reg1;

    KaosOp* op2 = malloc(sizeof *op2);
    op2->type = IR_VAL;
    union IRValue value2;
    value2.i = i1;
    op2->value = value2;

    KaosOp* op3 = malloc(sizeof *op3);
    op3->type = IR_VAL;
    union IRValue value3;
    value3.i = i2;
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
    reg1 += register_offset;
    reg2 += register_offset;

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
    reg1 += register_offset;
    reg2 += register_offset;
    reg3 += register_offset;

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
    reg1 += register_offset;
    reg2 += register_offset;
    reg3 += register_offset;

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
    reg1 += register_offset;
    reg2 += register_offset;
    reg3 += register_offset;

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

void shift_registers(KaosIR* program)
{
    i64 shift = 4;
    for (size_t i = 0; i < shift; i++) {
        push_inst_r_r(program, MOVR, shift + i, i);
    }

    push_inst_r_r(program, FMOVR, R2, R1);
}

Symbol* store_bool(KaosIR* program, char *name, bool is_any)
{
    /*
      0      8       16
      +------+-------+
      | type | value |
      +------+-------+
        i64     i64
    */
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
    /*
      0      8       16
      +------+-------+
      | type | value |
      +------+-------+
        i64     i64
    */
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
    /*
      0      8       16
      +------+-------+
      | type | value |
      +------+-------+
        i64     f64
    */
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

Symbol* store_string(KaosIR* program, char *name, bool is_any)
{
    /*
      0      8     16
      +------+-----+
      | type | ref |
      +------+-----+
        i64    i64
    */
    union Value value;
    value.i = 0;
    Symbol* symbol;
    if (is_any)
        symbol = addSymbol(name, K_ANY, value, V_STRING);
    else {
        symbol = addSymbol(name, K_STRING, value, V_STRING);
    }
    symbol->addr = stack_counter++;
    push_inst_i_i(program, ALLOCAI, symbol->addr, 2 * sizeof(long long));
    push_inst_r_i(program, REF_ALLOCAI, R2, symbol->addr);
    push_inst_r_r_i(program, STR, R2, R0, sizeof(long long));
    push_inst_r_i(program, MOVI, R3, sizeof(long long));
    push_inst_r_r_r_i(program, STXR, R2, R3, R1, sizeof(long long));

    return symbol;
}

Symbol* store_list(KaosIR* program, char *name, size_t len, bool is_dynamic)
{
    /*
      0      8     16
      +------+-----+
      | type | ref |
      +------+-----+
        i64    i64
    */
    union Value value;
    value.i = 0;
    Symbol* symbol = addSymbol(name, K_LIST, value, V_LIST);

    symbol->len = len;
    symbol->is_dynamic = is_dynamic;

    symbol->addr = stack_counter++;
    push_inst_i_i(program, ALLOCAI, symbol->addr, 2 * sizeof(long long));
    push_inst_r_i(program, REF_ALLOCAI, R2, symbol->addr);
    if (is_dynamic)
        push_inst_(program, DYN_NEW_LIST);
    else {
        push_inst_r_r_i(program, STR, R2, R0, sizeof(long long));
        push_inst_r_i(program, MOVI, R3, sizeof(long long));
        push_inst_r_r_r_i(program, STXR, R2, R3, R1, sizeof(long long));
    }

    return symbol;
}

Symbol* store_dict(KaosIR* program, char *name, size_t len, bool is_dynamic)
{
    /*
      0      8     16
      +------+-----+
      | type | ref |
      +------+-----+
        i64    i64
    */
    union Value value;
    value.i = 0;
    Symbol* symbol = addSymbol(name, K_DICT, value, V_DICT);

    symbol->len = len;
    symbol->is_dynamic = is_dynamic;

    symbol->addr = stack_counter++;
    push_inst_i_i(program, ALLOCAI, symbol->addr, 2 * sizeof(long long));
    push_inst_r_i(program, REF_ALLOCAI, R2, symbol->addr);
    if (is_dynamic)
        push_inst_(program, DYN_NEW_DICT);
    else {
        push_inst_r_r_i(program, STR, R2, R0, sizeof(long long));
        push_inst_r_i(program, MOVI, R3, sizeof(long long));
        push_inst_r_r_r_i(program, STXR, R2, R3, R1, sizeof(long long));
    }

    return symbol;
}

Symbol* store_any(KaosIR* program, char *name)
{
    /*
      0      8     16
      +------+-----+
      | type | ref |
      +------+-----+
        i64    i64
    */
    union Value value;
    value.i = 0;
    Symbol* symbol = addSymbol(name, K_ANY, value, V_ANY);
    symbol->is_dynamic = true;
    symbol->addr = stack_counter++;
    push_inst_i_i(program, ALLOCAI, symbol->addr, 2 * sizeof(long long));
    push_inst_r_i(program, REF_ALLOCAI, R2, symbol->addr);
    push_inst_r_r_i(program, STR, R2, R0, sizeof(long long));
    push_inst_r_i(program, MOVI, R3, sizeof(long long));
    push_inst_r_r_r_i(program, STXR, R2, R3, R1, sizeof(long long));

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
    push_inst_r_i(program, REF_ALLOCAI, R2, addr);
    push_inst_r_r_i(program, LDR, R0, R2, sizeof(long long));
    push_inst_r_i(program, MOVI, R3, sizeof(long long));
    push_inst_r_r_r_i(program, LDXR, R1, R2, R3, sizeof(long long));
}

void load_list(KaosIR* program, Symbol* symbol)
{
    i64 addr = symbol->addr;
    push_inst_r_i(program, REF_ALLOCAI, R2, addr);
    push_inst_r_r_i(program, LDR, R0, R2, sizeof(long long));
    push_inst_r_i(program, MOVI, R3, sizeof(long long));
    push_inst_r_r_r_i(program, LDXR, R1, R2, R3, sizeof(long long));
}

void load_dict(KaosIR* program, Symbol* symbol)
{
    i64 addr = symbol->addr;
    push_inst_r_i(program, REF_ALLOCAI, R2, addr);
    push_inst_r_r_i(program, LDR, R0, R2, sizeof(long long));
    push_inst_r_i(program, MOVI, R3, sizeof(long long));
    push_inst_r_r_r_i(program, LDXR, R1, R2, R3, sizeof(long long));
}

void load_any(KaosIR* program, Symbol* symbol)
{
    i64 addr = symbol->addr;
    push_inst_r_i(program, REF_ALLOCAI, R2, addr);
    push_inst_r_r_i(program, LDR, R0, R2, sizeof(long long));
    push_inst_r_i(program, MOVI, R3, sizeof(long long));
    push_inst_r_r_r_i(program, LDXR, R1, R2, R3, sizeof(long long));
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
            function_mode->addr = label_counter++;
            push_inst_i(program, DECLARE_LABEL, function_mode->addr);
            declareSpec(program, decl->v.func_decl->type->v.func_type->params);
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
    function_mode->addr = label_counter++;
    push_inst_i(program, DECLARE_LABEL, function_mode->addr);
    declareSpec(program, decl->v.func_decl->type->v.func_type->params);

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

        if (i == 0) {
            function_mode = declareFunction(
                __KAOS_MAIN_FUNCTION__,
                file->module,
                file->module_path,
                file->context,
                K_VOID,
                K_VOID
            );
        }

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

void determine_inline_functions(ASTRoot* ast_root)
{
    for (unsigned long i = 0; i < ast_root->file_count; i++) {
        File* file = ast_root->files[i];
        current_file_index = i;
        StmtList* stmt_list = file->stmt_list;
        pushModuleStack(file->module_path, file->module);

        // Foreach function look for other functions' decision blocks for calls
        for (unsigned long j = stmt_list->stmt_count; 0 < j; j--) {
            Stmt* stmt = stmt_list->stmts[j - 1];
            if (stmt->kind == DeclStmt_kind && stmt->v.decl_stmt->decl->kind == FuncDecl_kind) {
                Decl* decl = stmt->v.decl_stmt->decl;
                _Function* function = startFunctionNew(decl->v.func_decl->name->v.ident->name);
                function->should_inline = determine_inline_function(ast_root, function);
                endFunction();
            }
        }

        popModuleStack();
    }
}

bool determine_inline_function(ASTRoot* ast_root, _Function* function)
{
    unsigned long call_counter = 0;
    // Look for all functions' decision blocks for all the files compiled
    for (unsigned long i = 0; i < ast_root->file_count; i++) {
        File* file = ast_root->files[i];
        current_file_index = i;
        StmtList* stmt_list = file->stmt_list;
        pushModuleStack(file->module_path, file->module);

        for (unsigned long j = stmt_list->stmt_count; 0 < j; j--) {
            Stmt* stmt = stmt_list->stmts[j - 1];
            if (stmt->kind == DeclStmt_kind && stmt->v.decl_stmt->decl->kind == FuncDecl_kind) {
                Decl* decl = stmt->v.decl_stmt->decl;
                _Function* _function = startFunctionNew(decl->v.func_decl->name->v.ident->name);
                if (function == _function)
                    continue;
                if (decl->v.func_decl->decision != NULL) {
                    ExprList* expr_list = decl->v.func_decl->decision->v.decision_block->decisions;
                    for (unsigned long i = expr_list->expr_count; 0 < i; i--) {
                        Expr* expr = expr_list->exprs[i - 1];
                        if (does_decision_have_a_call(expr, function)) {
                            call_counter++;
                        }
                    }
                }
                endFunction();
            }
        }

        popModuleStack();
    }

    if (call_counter == 1)
        return true;
    else
        return false;
}

bool does_decision_have_a_call(Expr* expr, _Function* function)
{
    Stmt* stmt = NULL;
    switch (expr->kind) {
    case DecisionExpr_kind:
        stmt = expr->v.decision_expr->outcome;
        break;
    case DefaultExpr_kind:
        stmt = expr->v.default_expr->outcome;
        break;
    default:
        break;
    }

    return does_stmt_have_a_call(stmt, function);
}

bool does_stmt_have_a_call(Stmt* stmt, _Function* function)
{
    if (stmt->kind == ExprStmt_kind && stmt->v.expr_stmt->x->kind == CallExpr_kind) {
        Expr* expr = stmt->v.expr_stmt->x;
        _Function* _function = NULL;
        switch (expr->v.call_expr->fun->kind) {
        case Ident_kind:
            _function = getFunction(expr->v.call_expr->fun->v.ident->name, NULL);
            break;
        case SelectorExpr_kind:
            _function = getFunction(
                expr->v.call_expr->fun->v.selector_expr->sel->v.ident->name,
                expr->v.call_expr->fun->v.selector_expr->x->v.ident->name
            );
            break;
        default:
            break;
        }

        if (function == _function)
            return true;
    }

    return false;
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
