/*
 * Description: Function module of the Chaos Programming Language's source
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

#include "function.h"

extern int kaos_lineno;
extern int yyparse();

bool decision_execution_mode = false;
FunctionCall* function_call_start = NULL;

#ifdef CHAOS_COMPILER
extern jmp_buf LoopBreak;
extern jmp_buf LoopContinue;
#endif

#ifdef CHAOS_COMPILER
void startFunction(char *name, enum Type type, enum Type secondary_type, char* context, char* module_context, char* module, bool is_dynamic) {
#else
void startFunction(char *name, enum Type type, enum Type secondary_type) {
#endif
#ifndef CHAOS_COMPILER
    if (is_interactive)
        phase = PREPARSE;
#endif

    bool context_is_module_context = false;
    if (function_names_buffer.size > 0 && !isInFunctionNamesBuffer(name))
        context_is_module_context = true;

    removeFunctionIfDefined(name);
    function_mode = (struct _Function*)calloc(1, sizeof(_Function));
    function_mode->name = malloc(1 + strlen(name));
    function_mode->line_no = kaos_lineno;

    strcpy(function_mode->name, name);
    function_mode->type = type;
    function_mode->secondary_type = secondary_type;
    function_mode->parameter_count = 0;
    function_mode->optional_parameter_count = 0;

    function_mode->decision_expressions.capacity = 0;
    function_mode->decision_expressions.size = 0;

    function_mode->decision_functions.capacity = 0;
    function_mode->decision_functions.size = 0;

#ifdef CHAOS_COMPILER
    if (!is_dynamic) {
        function_mode->context = malloc(1 + strlen(context_is_module_context ? module_context : context));
        strcpy(function_mode->context, context_is_module_context ? module_context : context);
        function_mode->module_context = malloc(1 + strlen(module_context));
        strcpy(function_mode->module_context, module_context);
        function_mode->module = malloc(1 + strlen(module));
        strcpy(function_mode->module, module);
    } else {
#endif
    unsigned short parent_context = 1;
    if (module_path_stack.size > 1) parent_context = 2;
    char *context = module_path_stack.arr[module_path_stack.size - parent_context];
    char *module_context = module_path_stack.arr[module_path_stack.size - 1];
    char *module = module_stack.arr[module_stack.size - 1];
    function_mode->context = malloc(1 + strlen(context_is_module_context ? module_context : context));
    strcpy(function_mode->context, context_is_module_context ? module_context : context);
    function_mode->module_context = malloc(1 + strlen(module_context));
    strcpy(function_mode->module_context, module_context);
    function_mode->module = malloc(1 + strlen(module));
    strcpy(function_mode->module, module);
#ifdef CHAOS_COMPILER
    }
#endif

    function_mode->is_dynamic = strcmp(
        get_filename_ext(function_mode->module_context),
        __KAOS_DYNAMIC_LIBRARY_EXTENSION__
    ) == 0;

    if (function_mode->is_dynamic) {
        ASTNode* fake_ast_end;
        fake_ast_end = (struct ASTNode*)calloc(1, sizeof(ASTNode) + 0 * sizeof *fake_ast_end->strings);
        fake_ast_end->id = 0;
        fake_ast_end->node_type = AST_END;
        fake_ast_end->lineno = 0;
        fake_ast_end->module = function_mode->module_context;
        fake_ast_end->child = fake_ast_end;
        function_mode->node = fake_ast_end;
    }

    if (start_function == NULL) {
        start_function = function_mode;
        end_function = function_mode;
    } else {
        end_function->next = function_mode;
        function_mode->previous = end_function;
        end_function = function_mode;
        end_function->next = NULL;
    }

    function_mode->parameters = realloc(
        function_mode->parameters,
        sizeof(Symbol) * function_parameters_mode->parameter_count
    );

    if (function_parameters_mode->parameters != NULL) {
        memcpy(
            function_mode->parameters,
            function_parameters_mode->parameters,
            function_parameters_mode->parameter_count * sizeof(Symbol)
        );
    }
    function_mode->parameter_count = function_parameters_mode->parameter_count;
    function_mode->optional_parameter_count = function_parameters_mode->optional_parameter_count;


    for (unsigned short i = 0; i < function_mode->parameter_count; i++) {
        Symbol* parameter = function_mode->parameters[i];
        parameter->param_of = function_mode;
    }

    #if !defined(_WIN32) && !defined(_WIN64) && !defined(__CYGWIN__)
    if (name != NULL) {
        char *suggestion = malloc(1 + strlen(name));
        strcpy(suggestion, name);
        suggestion = strcat_ext(suggestion, "()");
#ifndef CHAOS_COMPILER
        add_suggestion(suggestion);
#endif
        free(suggestion);
    }
    #endif

    freeFunctionParametersMode();

#ifndef CHAOS_COMPILER
    if (is_interactive)
        phase = PROGRAM;
#endif
}

_Function* declareFunction(char *name, char *module, char *module_path, char *context, enum Type type, enum Type secondary_type) {
    _Function* function = (struct _Function*)calloc(1, sizeof(_Function));
    function->name = malloc(1 + strlen(name));
    function->line_no = kaos_lineno;
    function->is_compiled = false;
    function->ref = NULL;

    strcpy(function->name, name);
    function->type = type;
    function->secondary_type = secondary_type;
    function->parameters = NULL;
    function->parameter_count = 0;
    function->optional_parameter_count = 0;

    function->decision_expressions.capacity = 0;
    function->decision_expressions.size = 0;

    function->decision_functions.capacity = 0;
    function->decision_functions.size = 0;

    function->context = context;
    function->module_context = module_path;
    function->module = malloc(1 + strlen(module));
    strcpy(function->module, module);

    if (start_function == NULL) {
        start_function = function;
        end_function = function;
    } else {
        end_function->next = function;
        function->previous = end_function;
        end_function = function;
        end_function->next = NULL;
    }

    return function;
}

void startFunctionScope(_Function* function) {
    FunctionCall* function_call;
    if (function_call_start == NULL) {
        function_call = (struct FunctionCall*)malloc(sizeof(FunctionCall));
        function_call->start_symbol = NULL;
        function_call->end_symbol = NULL;
    } else {
        function_call = function_call_start;
    }
    function_call_start = NULL;
    function_call->function = function;
#ifndef CHAOS_COMPILER
    function_call->dont_pop_module_stack = false;
#endif

    scope_override = function_call;
    pushExecutedFunctionStack(function_call);
}

_Function* startFunctionNew(char *name) {
    function_mode = getFunctionByModuleContext(name, _ast_root->files[current_file_index]->module_path);
    if (function_mode->is_compiled)
        return function_mode;

    startFunctionScope(function_mode);
    for (unsigned short i = 0; i < function_mode->parameter_count; i++) {
        updateSymbolScope(function_mode->parameters[i]);
    }

    return function_mode;
}

void addFunctionParameterNew(_Function* function, Symbol* parameter) {
    function->parameter_count++;
    function->parameters = realloc(
        function->parameters,
        sizeof(Symbol) * function->parameter_count
    );
    function->parameters[function->parameter_count - 1] = parameter;
    parameter->param_of = function;
}

void endFunction() {
    popExecutedFunctionStack();
    scope_override = NULL;
    if (function_mode == NULL) return;
    function_mode = NULL;
    freeFunctionParametersMode();
}

void freeFunctionParametersMode() {
    if (function_parameters_mode == NULL) return;

    if (function_parameters_mode->parameter_count > 0) {
        free(function_parameters_mode->parameters);
    }
    free(function_parameters_mode);
    function_parameters_mode = NULL;
}

void resetFunctionParametersMode() {
    if (function_parameters_mode == NULL) return;

    for (unsigned short i = 0; i < function_parameters_mode->parameter_count; i++) {
        removeSymbol(function_parameters_mode->parameters[i]);
    }
    freeFunctionParametersMode();
}

FunctionCall* callFunction(char *name, char *module) {
    _Function* function = getFunction(name, module);
    FunctionCall* function_call;
    if (function_call_start == NULL) {
        function_call = (struct FunctionCall*)malloc(sizeof(FunctionCall));
        function_call->start_symbol = NULL;
        function_call->end_symbol = NULL;
    } else {
        function_call = function_call_start;
    }
    function_call_start = NULL;
    function_call->function = function;
#ifndef CHAOS_COMPILER
    function_call->dont_pop_module_stack = false;
#endif

    if (function_parameters_mode != NULL &&
        function_parameters_mode->parameter_count < (function->parameter_count - function->optional_parameter_count)) {
            resetFunctionParametersMode();
            free(function_call);
            throw_error(E_INCORRECT_FUNCTION_ARGUMENT_COUNT, name);
    }

    if (function->parameter_count > 0 && function_parameters_mode == NULL) {
        resetFunctionParametersMode();
        free(function_call);
        throw_error(E_INCORRECT_FUNCTION_ARGUMENT_COUNT, name);
    }

    if (function_parameters_mode != NULL && function_parameters_mode->parameter_count > function->parameter_count) {
        resetFunctionParametersMode();
        free(function_call);
        throw_error(E_INCORRECT_FUNCTION_ARGUMENT_COUNT, name);
    }

    scope_override = function_call;
    for (unsigned short i = 0; i < function->parameter_count; i++) {
        Symbol* parameter = function->parameters[i];

        if ((i + 1) > function_parameters_mode->parameter_count) {
            function_parameters_mode->parameters = realloc(
                function_parameters_mode->parameters,
                sizeof(Symbol) * ++function_parameters_mode->parameter_count
            );

            if (function_parameters_mode->parameters == NULL) {
                resetFunctionParametersMode();
                free(function_call);
                throw_error(E_MEMORY_ALLOCATION_FOR_FUNCTION_FAILED, NULL);
            }

            Symbol* parameter_call = createCloneFromSymbol(
                parameter->secondary_name,
                parameter->type,
                parameter,
                parameter->secondary_type
            );
            parameter_call->scope = function_call;
            parameter_call->param_of = function;

            function_parameters_mode->parameters[function_parameters_mode->parameter_count - 1] = parameter_call;
        } else {
            Symbol* parameter_call = function_parameters_mode->parameters[i];

            if (parameter->type != K_ANY && parameter->type != parameter_call->type) {
                resetFunctionParametersMode();
                free(function_call);
                throw_error(E_ILLEGAL_VARIABLE_TYPE_FOR_FUNCTION_PARAMETER, parameter->secondary_name, function->name);
            }

            if ((parameter->type == K_LIST || parameter->type == K_DICT) && parameter->secondary_type != K_ANY) {
                for (unsigned long i = 0; i < parameter_call->children_count; i++) {
                    Symbol* child = parameter_call->children[i];
                    if (child->type != parameter->secondary_type) {
                        resetFunctionParametersMode();
                        free(function_call);
                        throw_error(E_ILLEGAL_VARIABLE_TYPE_FOR_FUNCTION_PARAMETER, parameter->secondary_name, function->name);
                    }
                }
            }

            parameter_call->name = malloc(1 + strlen(parameter->secondary_name));
            strcpy(parameter_call->name, parameter->secondary_name);
            parameter_call->scope = function_call;
            parameter_call->param_of = function;
            parameter_call->secondary_type = parameter->secondary_type;
        }
    }
    scope_override = NULL;

    freeFunctionParametersMode();

    FunctionCall* parent_scope = getCurrentScope();
    pushExecutedFunctionStack(function_call);
    function_call_stack.arr[function_call_stack.size - 1]->parent_scope = parent_scope;

#ifndef CHAOS_COMPILER
    if (
        strcmp(
            module_path_stack.arr[module_path_stack.size - 1],
            function_call_stack.arr[function_call_stack.size - 1]->function->module_context
        ) == 0
        &&
        strcmp(
            module_stack.arr[module_stack.size - 1],
            ""
        ) == 0
    ) {
        function_call->dont_pop_module_stack = true;
    } else {
#endif
        pushModuleStack(function_call_stack.arr[function_call_stack.size - 1]->function->module_context, "");
#ifndef CHAOS_COMPILER
    }
#endif

    // if (function->is_dynamic) {
    //     callFunctionFromDynamicLibrary(function);
    // }

    return function_call;
}

#ifndef CHAOS_COMPILER
void callFunctionCleanUp(FunctionCall* function_call) {
#else
void callFunctionCleanUp(FunctionCall* function_call, bool has_decision) {
#endif

    bool is_loop_breaked = false;
    bool is_loop_continued = false;

#ifndef CHAOS_COMPILER
    if (setjmp(LoopBreakDecision))
        is_loop_breaked = true;

    if (setjmp(LoopContinueDecision))
        is_loop_continued = true;
#endif

#ifdef CHAOS_COMPILER
    if (has_decision) {
#endif
    if (!is_loop_breaked &&
        !is_loop_continued
    ) {
        executeDecision(function_call);
    } else {
        callFunctionCleanUpSymbols(function_call);
    }
#ifdef CHAOS_COMPILER
    } else {
        if (function_call_stack.size < 2 && decision_symbol_chain != NULL) {
            removeSymbol(decision_symbol_chain);
            decision_symbol_chain = NULL;
        }
        callFunctionCleanUpSymbols(function_call);
    }
#endif

    if (function_call->function->type != K_VOID &&
        function_call->function->symbol == NULL &&
        !is_loop_breaked &&
        !is_loop_continued
    ) {
        throw_error(E_FUNCTION_DID_NOT_RETURN_ANYTHING, function_call->function->name);
        return;
    }

#ifndef CHAOS_COMPILER
    if (function_call->dont_pop_module_stack) {
        popExecutedFunctionStack();
    } else {
#endif
        callFunctionCleanUpCommon();
#ifndef CHAOS_COMPILER
    }
#endif

    if (is_loop_breaked) {
        free(function_call);
        breakLoop();
    }

    if (is_loop_continued) {
        free(function_call);
        continueLoop();
    }
}

void callFunctionCleanUpSymbols(FunctionCall* function_call) {
    removeSymbolsByScope(function_call);
}

void callFunctionCleanUpCommon() {
    popModuleStack();
    popExecutedFunctionStack();
}

_Function* getFunction(char *name, char *module) {
    function_cursor = start_function;
    while (function_cursor != NULL) {
        if (module == NULL && strcmp(function_cursor->module, "") != 0) {
            function_cursor = function_cursor->next;
            continue;
        }
        bool criteria = function_cursor->name != NULL && strcmp(function_cursor->name, name) == 0;
        criteria = criteria && (
            strcmp(function_cursor->context, module_path_stack.arr[module_path_stack.size - 1]) == 0
        );
        if (module != NULL) criteria = criteria && strcmp(function_cursor->module, module) == 0;
        if (criteria) {
            _Function* function = function_cursor;
            return function;
        }
        function_cursor = function_cursor->next;
    }
    if (phase == PROGRAM) {
        if (scope_override != NULL)
            free(scope_override);
        throw_error(
            E_UNDEFINED_FUNCTION,
            name,
            (module == NULL || strcmp(module, "") == 0) ? "<module>" : module
        );
    }
    return NULL;
}

_Function* getFunctionByModuleContext(char *name, char *module_context) {
    function_cursor = start_function;
    while (function_cursor != NULL) {
        bool criteria = function_cursor->name != NULL && strcmp(function_cursor->name, name) == 0;
        if (criteria && module_context != NULL && (
            strcmp(function_cursor->context, module_context) == 0
            ||
            strcmp(function_cursor->module_context, module_context) == 0
        )) {
            _Function* function = function_cursor;
            return function;
        }
        function_cursor = function_cursor->next;
    }
    if (phase == PROGRAM) {
        if (scope_override != NULL)
            free(scope_override);
        throw_error(
            E_UNDEFINED_FUNCTION,
            name,
            (module_context == NULL || strcmp(module_context, "") == 0) ? "<module>" : module_context
        );
    }
    return NULL;
}

_Function* checkDuplicateFunction(char *name, char *module_path) {
    function_cursor = start_function;
    while (function_cursor != NULL) {
        if (
            strcmp(function_cursor->name, name) == 0
            &&
            strcmp(function_cursor->module_context, module_path) == 0
        ) {
            _Function* function = function_cursor;
            return function;
        }

        function_cursor = function_cursor->next;
    }

    return NULL;
}

void removeFunctionIfDefined(char *name) {
    unsigned short parent_context = 1;
    if (module_path_stack.size > 1) parent_context = 2;

    function_cursor = start_function;
    while (function_cursor != NULL) {
        if (strcmp(function_cursor->name, name) == 0 &&
            strcmp(function_cursor->context, module_path_stack.arr[module_path_stack.size - parent_context]) == 0 &&
            strcmp(function_cursor->module_context, module_path_stack.arr[module_path_stack.size - 1]) == 0 &&
            strcmp(function_cursor->module, module_stack.arr[module_stack.size - 1]) == 0
        ) {
            _Function* function = function_cursor;
#ifndef CHAOS_COMPILER
            function->node->dont_transpile = true;
#endif
            removeFunction(function);
            return;
        }
        function_cursor = function_cursor->next;
    }
}

void printFunctionTable() {
    _Function* function = start_function;
    printf("[start] =>\n");
    while (function != NULL) {
        char *context_temp = malloc(1 + strlen(function->context));
        strcpy(context_temp, function->context);
        printf(
            "\t{name: %s, type: %u, parameter_count: %hu, optional_parameter_count: %hu, decision_length: %u, context: %s, module_context: %s, module: %s} =>\n",
            function->name,
            function->type,
            function->parameter_count,
            function->optional_parameter_count,
            function->decision_functions.size,
            context_temp,
            function->module_context,
            function->module
        );
        function = function->next;
        free(context_temp);
    }
    printf("[end]\n");
}

void startFunctionParameters() {
    function_parameters_mode = (struct _Function*)calloc(1, sizeof(_Function));
    function_parameters_mode->parameter_count = 0;
    function_parameters_mode->optional_parameter_count = 0;
}

void addFunctionParameter(char *secondary_name, enum Type type, enum Type secondary_type) {
    union Value value;
    value.i = 0;
    Symbol* symbol = addSymbol(NULL, type, value, V_VOID);
    symbol->secondary_name = malloc(1 + strlen(secondary_name));
    strcpy(symbol->secondary_name, secondary_name);
    symbol->secondary_type = secondary_type;

    addSymbolToFunctionParameters(symbol, false);
}

void addFunctionOptionalParameterBool(char *secondary_name, bool b) {
    Symbol* symbol = addSymbolBool(NULL, b);
    symbol->secondary_name = malloc(1 + strlen(secondary_name));
    strcpy(symbol->secondary_name, secondary_name);

    addSymbolToFunctionParameters(symbol, true);
}

void addFunctionOptionalParameterInt(char *secondary_name, long long i) {
    Symbol* symbol = addSymbolInt(NULL, i);
    symbol->secondary_name = malloc(1 + strlen(secondary_name));
    strcpy(symbol->secondary_name, secondary_name);

    addSymbolToFunctionParameters(symbol, true);
}

void addFunctionOptionalParameterFloat(char *secondary_name, double f) {
    Symbol* symbol = addSymbolFloat(NULL, f);
    symbol->secondary_name = malloc(1 + strlen(secondary_name));
    strcpy(symbol->secondary_name, secondary_name);

    addSymbolToFunctionParameters(symbol, true);
}

void addFunctionOptionalParameterString(char *secondary_name, char *s) {
    Symbol* symbol = addSymbolString(NULL, s);
    symbol->secondary_name = malloc(1 + strlen(secondary_name));
    strcpy(symbol->secondary_name, secondary_name);

    addSymbolToFunctionParameters(symbol, true);
}

void addFunctionOptionalParameterComplex(char *secondary_name, enum Type type) {
    Symbol* symbol = finishComplexMode(NULL, type);
    symbol->secondary_name = malloc(1 + strlen(secondary_name));
    strcpy(symbol->secondary_name, secondary_name);

    addSymbolToFunctionParameters(symbol, true);
}

void addSymbolToFunctionParameters(Symbol* symbol, bool is_optional) {
    if (phase == PREPARSE) {
        symbol->role = PARAM;
    } else if (phase == PROGRAM) {
        symbol->role = CALL_PARAM;
    }

    if (function_parameters_mode == NULL) {
        startFunctionParameters();
    }

    function_parameters_mode->parameters = realloc(
        function_parameters_mode->parameters,
        sizeof(Symbol) * ++function_parameters_mode->parameter_count
    );

    if (is_optional)
        ++function_parameters_mode->optional_parameter_count;

    if (function_parameters_mode->parameters == NULL) {
        throw_error(E_MEMORY_ALLOCATION_FOR_FUNCTION_FAILED, NULL);
    }

    for (unsigned short i = function_parameters_mode->parameter_count - 1; i > 0; i--) {
        function_parameters_mode->parameters[i] = function_parameters_mode->parameters[i - 1];
    }

    function_parameters_mode->parameters[0] = symbol;
}

void initFunctionCall() {
    if (function_call_start == NULL) {
        function_call_start = (struct FunctionCall*)malloc(sizeof(FunctionCall));
        function_call_start->start_symbol = NULL;
        function_call_start->end_symbol = NULL;
    }
    scope_override = function_call_start;
}

void addFunctionCallParameterBool(bool b) {
    initFunctionCall();
    union Value value;
    value.b = b;
    Symbol* symbol = addSymbol(NULL, K_BOOL, value, V_BOOL);
    addSymbolToFunctionParameters(symbol, false);
}

void addFunctionCallParameterInt(long long i) {
    initFunctionCall();
    union Value value;
    value.i = i;
    Symbol* symbol = addSymbol(NULL, K_NUMBER, value, V_INT);
    addSymbolToFunctionParameters(symbol, false);
}

void addFunctionCallParameterFloat(double f) {
    initFunctionCall();
    union Value value;
    value.f = f;
    Symbol* symbol = addSymbol(NULL, K_NUMBER, value, V_FLOAT);
    addSymbolToFunctionParameters(symbol, false);
}

void addFunctionCallParameterString(char *s) {
    initFunctionCall();
    union Value value;
    value.s = malloc(1 + strlen(s));
    strcpy(value.s, s);
    Symbol* symbol = addSymbol(NULL, K_STRING, value, V_STRING);
    addSymbolToFunctionParameters(symbol, false);
}

void addFunctionCallParameterSymbol(char *name) {
    initFunctionCall();
    addSymbolToFunctionParameters(getSymbolFunctionParameter(name), false);
}

void addFunctionCallParameterList(enum Type type) {
    initFunctionCall();
    Symbol* symbol = finishComplexMode(NULL, type);
    changeSymbolScope(symbol, scope_override);
    addSymbolToFunctionParameters(symbol, false);
}

void returnSymbol(char *name) {
    Symbol* symbol = getSymbol(name);
    if (symbol->type != K_ANY &&
        function_call_stack.arr[function_call_stack.size - 1]->function->type != K_ANY &&
        symbol->type != function_call_stack.arr[function_call_stack.size - 1]->function->type
    ) {
        throw_error(E_ILLEGAL_VARIABLE_TYPE_FOR_FUNCTION, getTypeName(symbol->type), function_call_stack.arr[function_call_stack.size - 1]->function->name);
    }
    if (symbol->secondary_type != K_ANY &&
        function_call_stack.arr[function_call_stack.size - 1]->function->secondary_type != K_ANY &&
        symbol->secondary_type != function_call_stack.arr[function_call_stack.size - 1]->function->secondary_type
    ) {
        throw_error(E_ILLEGAL_VARIABLE_TYPE_FOR_FUNCTION, getTypeName(symbol->secondary_type), function_call_stack.arr[function_call_stack.size - 1]->function->name);
    }

    scope_override = function_call_stack.arr[function_call_stack.size - 1]->parent_scope;
    function_call_stack.arr[function_call_stack.size - 1]->function->symbol = createCloneFromSymbol(
        NULL,
        symbol->type,
        symbol,
        symbol->secondary_type
    );

    decision_symbol_chain = createCloneFromSymbol(
        NULL,
        symbol->type,
        symbol,
        symbol->secondary_type
    );

    scope_override = NULL;
}

void printFunctionReturn(FunctionCall* function_call, char *end, bool pretty, bool escaped) {
    if (function_call->function->symbol == NULL) {
        throw_error(E_FUNCTION_DID_NOT_RETURN_ANYTHING, function_call->function->name);
        return;
    }
    printSymbolValueEndWith(function_call->function->symbol, end, pretty, escaped);
    freeFunctionReturn(function_call);
}

void createCloneFromFunctionReturn(char *clone_name, enum Type type, FunctionCall* function_call, enum Type extra_type) {
    if (function_call->function->symbol == NULL) {
        throw_error(E_FUNCTION_DID_NOT_RETURN_ANYTHING, function_call->function->name);
        return;
    }
    createCloneFromSymbol(clone_name, type, function_call->function->symbol, extra_type);
    freeFunctionReturn(function_call);
}

void updateSymbolByClonningFunctionReturn(char *clone_name, FunctionCall* function_call) {
    if (function_call->function->symbol == NULL) {
        throw_error(E_FUNCTION_DID_NOT_RETURN_ANYTHING, function_call->function->name);
        return;
    }
    updateSymbolByClonning(clone_name, function_call->function->symbol);
    freeFunctionReturn(function_call);
}

void updateComplexSymbolByClonningFunctionReturn(FunctionCall* function_call) {
    if (function_call->function->symbol == NULL) {
        throw_error(E_FUNCTION_DID_NOT_RETURN_ANYTHING, function_call->function->name);
        return;
    }
    updateComplexElementSymbol(function_call->function->symbol);
    freeFunctionReturn(function_call);
}

void initMainFunction() {
    function_names_buffer.capacity = 0;
    function_names_buffer.size = 0;
    decision_buffer = "";
    dummy_scope = (struct FunctionCall*)malloc(sizeof(FunctionCall));
    dummy_scope->start_symbol = NULL;
    dummy_scope->end_symbol = NULL;
    initScopeless();
    initMainContext();
    initKaosApi();
#if !defined(_WIN32) && !defined(_WIN64) && !defined(__CYGWIN__)
    increaseStackSize();
#endif
}

void initScopeless() {
    _Function* scopeless_function = (struct _Function*)malloc(sizeof(_Function));
    scopeless_function->name = "N/A";
    scopeless_function->type = K_ANY;
    scopeless_function->parameter_count = 0;
    scopeless = (struct FunctionCall*)malloc(sizeof(FunctionCall));
    scopeless->start_symbol = NULL;
    scopeless->end_symbol = NULL;
    scopeless->function = scopeless_function;
}

#if !defined(_WIN32) && !defined(_WIN64) && !defined(__CYGWIN__)
void increaseStackSize() {
    const rlim_t stack_size = ULONG_MAX; // Maximum possible stack size
    struct rlimit rl;
    int result;

    result = getrlimit(RLIMIT_STACK, &rl);
    if (result == 0) {
        if (rl.rlim_cur < stack_size) {
            rl.rlim_cur = stack_size;
            result = setrlimit(RLIMIT_STACK, &rl);
            // if (result != 0) {
            //     fprintf(stderr, "setrlimit returned result = %d\n", result);
            // }
        }
    }
}
#endif

void removeFunction(_Function* function) {
    _Function* previous_function = function->previous;
    _Function* next_function = function->next;

    if (previous_function == NULL && next_function == NULL) {
        start_function = NULL;
        end_function = NULL;
        freeFunction(function);
        return;
    } else if (previous_function == NULL) {
        start_function = next_function;
        start_function->previous = NULL;
    } else if (next_function == NULL) {
        end_function = previous_function;
        end_function->next = NULL;
    } else {
        previous_function->next = next_function;
        next_function->previous = previous_function;
    }

    freeFunction(function);
}

void freeFunction(_Function* function) {
    free(function->name);
    if (function->ref == NULL)
        free(function->parameters);
    for (unsigned i = 0; i < function->decision_functions.size; i++) {
        free(function->decision_expressions.arr[i]);
        free(function->decision_functions.arr[i]);
    }
    free(function->decision_expressions.arr);
    free(function->decision_functions.arr);
    free(function->decision_default);
    free(function->module);
    if (function->is_dynamic)
        free(function->node);
    free(function);
}

void freeAllFunctions() {
    function_cursor = start_function;
    while (function_cursor != NULL) {
        _Function* function = function_cursor;
        function_cursor = function_cursor->next;
        freeFunction(function);
    }
}

bool block(enum BlockType type) {
    switch (type) {
    case B_EXPRESSION:
        if (decision_mode != NULL) {
            decision_expression_mode = decision_mode;
            decision_function_mode = NULL;
            return true;
        }
        break;
    case B_FUNCTION:
        if (decision_mode != NULL) {
            decision_function_mode = decision_mode;
            decision_expression_mode = NULL;
            return true;
        }
        break;
    default:
        return false;
        break;
    }
    return false;
}

void addBooleanDecision() {
    append_to_array(
        &decision_mode->decision_functions,
        trim_string(decision_buffer)
    );
    free(decision_buffer);
    decision_buffer = "";
    decision_expression_mode = NULL;
    decision_function_mode = NULL;
}

void addDefaultDecision() {
    decision_mode->decision_default = realloc(decision_mode->decision_default, 1 + strlen(trim_string(decision_buffer)));
    strcpy(decision_mode->decision_default, trim_string(decision_buffer));
    free(decision_buffer);
    decision_buffer = "";
    decision_expression_mode = NULL;
    decision_function_mode = NULL;
}

void executeDecision(FunctionCall* function_call) {
#ifndef CHAOS_COMPILER
    if (function_call->function->decision_node == NULL) {
        if (function_call_stack.size < 2 && decision_symbol_chain != NULL) {
            removeSymbol(decision_symbol_chain);
            decision_symbol_chain = NULL;
        }
        callFunctionCleanUpSymbols(function_call);
        return;
    }

    eval_node(function_call->function->decision_node, function_call->function->module_context);
    stop_ast_evaluation = false;
#endif

    if (decision_symbol_chain == NULL)
        return;

    if (function_call_stack.arr[function_call_stack.size - 1]->function->type != K_VOID && function_call_stack.arr[function_call_stack.size - 1]->function->symbol == NULL) {
        scope_override = function_call_stack.arr[function_call_stack.size - 1]->parent_scope;
        function_call_stack.arr[function_call_stack.size - 1]->function->symbol = createCloneFromSymbol(
            NULL,
            decision_symbol_chain->type,
            decision_symbol_chain,
            decision_symbol_chain->secondary_type
        );
        scope_override = NULL;
    }
    if (function_call_stack.size < 2 && decision_symbol_chain != NULL) {
        removeSymbol(decision_symbol_chain);
        decision_symbol_chain = NULL;
    }
}

void addFunctionNameToFunctionNamesBuffer(char *name) {
    append_to_array(&function_names_buffer, name);
}

void freeFunctionNamesBuffer() {
    for (unsigned i = 0; i < function_names_buffer.size; i++) {
        free(function_names_buffer.arr[i]);
    }
    function_names_buffer.size = 0;
}

bool isInFunctionNamesBuffer(char *name) {
    for (unsigned i = 0; i < function_names_buffer.size; i++) {
        if (strcmp(function_names_buffer.arr[i], name) == 0) return true;
    }
    return false;
}

bool isFunctionType(char *name, char *module, enum Type type) {
    _Function* function = getFunction(name, module);
    return function->type == type;
}

void setScopeless(Symbol* symbol) {
    symbol->scope = scopeless;
    for (unsigned long i = 0; i < symbol->children_count; i++) {
        setScopeless(symbol->children[i]);
    }
}

void pushExecutedFunctionStack(FunctionCall* function_call) {
    if (function_call_stack.capacity == 0) {
        function_call_stack.arr = (FunctionCall**)malloc((function_call_stack.capacity = 2) * sizeof(FunctionCall));
    } else if (function_call_stack.capacity == function_call_stack.size) {
        function_call_stack.arr = (FunctionCall**)realloc(function_call_stack.arr, (function_call_stack.capacity *= 2) * sizeof(FunctionCall));
    }

    function_call_stack.arr[function_call_stack.size] = function_call;
    function_call_stack.size++;
}

void popExecutedFunctionStack() {
    function_call_stack.arr[function_call_stack.size - 1] = NULL;
    function_call_stack.size--;
}

void freeFunctionReturn(FunctionCall* function_call) {
    if (function_call->function->symbol != NULL) {
        removeSymbol(function_call->function->symbol);
        function_call->function->symbol = NULL;
    }
}

void decisionBreakLoop() {
#ifdef CHAOS_COMPILER
    callFunctionCleanUpSymbols(function_call_stack.arr[function_call_stack.size - 1]);
    free(function_call_stack.arr[function_call_stack.size - 1]);
    callFunctionCleanUpCommon();
    longjmp(LoopBreak, 1);
#else
    longjmp(LoopBreakDecision, 1);
#endif
}

void decisionContinueLoop() {
#ifdef CHAOS_COMPILER
    callFunctionCleanUpSymbols(function_call_stack.arr[function_call_stack.size - 1]);
    free(function_call_stack.arr[function_call_stack.size - 1]);
    callFunctionCleanUpCommon();
    longjmp(LoopContinue, 1);
#else
    longjmp(LoopContinueDecision, 1);
#endif
}

void updateDecisionSymbolChainScope() {
    if (function_call_stack.size > 0 && decision_symbol_chain != NULL) {
        changeSymbolScope(decision_symbol_chain, function_call_stack.arr[function_call_stack.size - 1]);
    }
}
