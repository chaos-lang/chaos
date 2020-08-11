#include "function.h"

extern int yylineno;
bool interactive_shell_function_error_absorbed = false;
extern int yyparse();

int reset_line_no_to = 0;
bool decision_execution_mode = false;

void startFunction(char *name, enum Type type, enum Type secondary_type) {
    if (is_interactive) {
        phase = PREPARSE;
    }

    if (function_names_buffer.size > 0) {
        if (!isInFunctionNamesBuffer(name)) {
            //free(name);
            freeFunctionMode();
            return;
        }
    }

    function_mode = (struct _Function*)calloc(1, sizeof(_Function));
    function_mode->body = "";
    function_mode->name = malloc(1 + strlen(name));
    function_mode->line_no = yylineno;
    strcpy(function_mode->name, name);
    function_mode->type = type;
    function_mode->secondary_type = secondary_type;
    function_mode->parameter_count = 0;
    function_mode->optional_parameter_count = 0;

    function_mode->decision_expressions.capacity = 0;
    function_mode->decision_expressions.size = 0;

    function_mode->decision_functions.capacity = 0;
    function_mode->decision_functions.size = 0;

    unsigned short parent_context = 1;
    if (module_path_stack.size > 1) parent_context = 2;
    function_mode->context = malloc(1 + strlen(module_path_stack.arr[module_path_stack.size - parent_context]));
    strcpy(function_mode->context, module_path_stack.arr[module_path_stack.size - parent_context]);
    function_mode->module_context = malloc(1 + strlen(module_path_stack.arr[module_path_stack.size - 1]));
    strcpy(function_mode->module_context, module_path_stack.arr[module_path_stack.size - 1]);
    function_mode->module = malloc(1 + strlen(module_stack.arr[module_stack.size - 1]));
    strcpy(function_mode->module, module_stack.arr[module_stack.size - 1]);

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
        add_suggestion(suggestion);
        free(suggestion);
    }
    #endif

    //free(name);
    freeFunctionMode();

    if (is_interactive) {
        phase = PROGRAM;
    }
}

void endFunction() {
    if (function_mode == NULL) return;
    function_mode = NULL;
    freeFunctionMode();
}

void freeFunctionMode() {
    if (function_parameters_mode == NULL) return;

    if (function_parameters_mode->parameter_count > 0) {
        free(function_parameters_mode->parameters);
    }
    free(function_parameters_mode);
    function_parameters_mode = NULL;
}

void callFunction(char *name, char *module) {
    _Function* function = getFunction(name, module);

    if (function_parameters_mode != NULL &&
        function_parameters_mode->parameter_count < (function->parameter_count - function->optional_parameter_count))
            throw_error(E_INCORRECT_FUNCTION_ARGUMENT_COUNT, name);

    if (function->parameter_count > 0 && function_parameters_mode == NULL)
        throw_error(E_INCORRECT_FUNCTION_ARGUMENT_COUNT, name);

    if (function_parameters_mode != NULL && function_parameters_mode->parameter_count > function->parameter_count) {
        freeFunctionMode();
        throw_error(E_INCORRECT_FUNCTION_ARGUMENT_COUNT, name);
    }

    scope_override = function;
    for (unsigned short i = 0; i < function->parameter_count; i++) {
        Symbol* parameter = function->parameters[i];

        if ((i + 1) > function_parameters_mode->parameter_count) {
            function_parameters_mode->parameters = realloc(
                function_parameters_mode->parameters,
                sizeof(Symbol) * ++function_parameters_mode->parameter_count
            );

            if (function_parameters_mode->parameters == NULL) {
                freeFunctionMode();
                throw_error(E_MEMORY_ALLOCATION_FOR_FUNCTION_FAILED, NULL);
            }

            Symbol* parameter_call = createCloneFromSymbol(
                parameter->secondary_name,
                parameter->type,
                parameter,
                parameter->secondary_type
            );
            parameter_call->scope = function;
            parameter_call->recursion_depth = recursion_depth + 1;
            parameter_call->param_of = function;

            function_parameters_mode->parameters[function_parameters_mode->parameter_count - 1] = parameter_call;
        } else {
            Symbol* parameter_call = function_parameters_mode->parameters[i];

            if (parameter->type != K_ANY && parameter->type != parameter_call->type) {
                freeFunctionMode();
                throw_error(E_ILLEGAL_VARIABLE_TYPE_FOR_FUNCTION_PARAMETER, parameter->secondary_name, function->name);
            }

            if ((parameter->type == K_LIST || parameter->type == K_DICT) && parameter->secondary_type != K_ANY) {
                for (unsigned long i = 0; i < parameter_call->children_count; i++) {
                    Symbol* child = parameter_call->children[i];
                    if (child->type != parameter->secondary_type) {
                        freeFunctionMode();
                        throw_error(E_ILLEGAL_VARIABLE_TYPE_FOR_FUNCTION_PARAMETER, parameter->secondary_name, function->name);
                    }
                }
            }

            parameter_call->name = malloc(1 + strlen(parameter->secondary_name));
            strcpy(parameter_call->name, parameter->secondary_name);
            parameter_call->scope = function;
            parameter_call->recursion_depth = recursion_depth + 1;
            parameter_call->param_of = function;
        }
    }
    scope_override = NULL;

    freeFunctionMode();

    _Function* parent_scope = getCurrentScope();
    executed_function = function;
    executed_function->parent_scope = parent_scope;

    pushModuleStack(executed_function->module_context, "");

    recursion_depth++;

    if (recursion_depth > __KAOS_MAX_RECURSION_DEPTH__) {
        throw_error(E_MAXIMUM_RECURSION_DEPTH_EXCEEDED, NULL);
    }

    reset_line_no_to = function->line_no;

    if (is_interactive) {
        if (setjmp(InteractiveShellFunctionErrorAbsorber)) {
            interactive_shell_function_error_absorbed = true;
        }
    }

    if (!interactive_shell_function_error_absorbed) {
        if (strcmp(
        get_filename_ext(function->module_context),
        __KAOS_DYNAMIC_LIBRARY_EXTENSION__
        ) == 0
        ) {
            callFunctionFromDynamicLibrary(function);
        } else {
            eval_node(function->node->child, function->module_context);
        }
    }

    reset_line_no_to = 0;

    if (!interactive_shell_function_error_absorbed)
        executeDecision(function);

    if (function->type != K_VOID &&
        function->symbol == NULL &&
        !interactive_shell_function_error_absorbed
    ) {
        append_to_array_without_malloc(&free_string_stack, name);
        throw_error(E_FUNCTION_DID_NOT_RETURN_ANYTHING, name);
        return;
    }

    for (unsigned short i = 0; i < function->parameter_count; i++) {
        Symbol* parameter = function->parameters[i];
        removeSymbolByName(parameter->secondary_name);
    }

    recursion_depth--;

    removeSymbolsByScope(function);

    popModuleStack();

    executed_function = NULL;

    if (is_interactive && interactive_shell_function_error_absorbed) {
        interactive_shell_function_error_absorbed = false;
        yyrestart_interactive();
        yyparse();
    }
}

_Function* getFunction(char *name, char *module) {
    function_cursor = start_function;
    while (function_cursor != NULL) {
        if (module == NULL && strcmp(function_cursor->module, "") != 0) {
            function_cursor = function_cursor->next;
            continue;
        }
        bool criteria = function_cursor->name != NULL && strcmp(function_cursor->name, name) == 0;
        criteria = criteria && strcmp(function_cursor->context, module_path_stack.arr[module_path_stack.size - 1]) == 0;
        if (module != NULL) criteria = criteria && strcmp(function_cursor->module, module) == 0;
        if (criteria) {
            _Function* function = function_cursor;
            return function;
        }
        function_cursor = function_cursor->next;
    }
    if (phase == PROGRAM) {
        //append_to_array_without_malloc(&free_string_stack, name);
        throw_error(E_UNDEFINED_FUNCTION, name);
    }
    return NULL;
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
    Symbol* symbol = addSymbol(NULL, type, value, V_VOID);
    symbol->secondary_name = malloc(1 + strlen(secondary_name));
    strcpy(symbol->secondary_name, secondary_name);
    symbol->secondary_type = secondary_type;

    addSymbolToFunctionParameters(symbol, false);
    //free(secondary_name);
}

void addFunctionOptionalParameterBool(char *secondary_name, bool b) {
    Symbol* symbol = addSymbolBool(NULL, b);
    symbol->secondary_name = malloc(1 + strlen(secondary_name));
    strcpy(symbol->secondary_name, secondary_name);

    addSymbolToFunctionParameters(symbol, true);
    //free(secondary_name);
}

void addFunctionOptionalParameterInt(char *secondary_name, long long i) {
    Symbol* symbol = addSymbolInt(NULL, i);
    symbol->secondary_name = malloc(1 + strlen(secondary_name));
    strcpy(symbol->secondary_name, secondary_name);

    addSymbolToFunctionParameters(symbol, true);
    //free(secondary_name);
}

void addFunctionOptionalParameterFloat(char *secondary_name, long double f) {
    Symbol* symbol = addSymbolFloat(NULL, f);
    symbol->secondary_name = malloc(1 + strlen(secondary_name));
    strcpy(symbol->secondary_name, secondary_name);

    addSymbolToFunctionParameters(symbol, true);
    //free(secondary_name);
}

void addFunctionOptionalParameterString(char *secondary_name, char *s) {
    Symbol* symbol = addSymbolString(NULL, s);
    symbol->secondary_name = malloc(1 + strlen(secondary_name));
    strcpy(symbol->secondary_name, secondary_name);

    addSymbolToFunctionParameters(symbol, true);
    //free(secondary_name);
}

void addFunctionOptionalParameterComplex(char *secondary_name, enum Type type) {
    Symbol* symbol = finishComplexMode(NULL, type);
    symbol->secondary_name = malloc(1 + strlen(secondary_name));
    strcpy(symbol->secondary_name, secondary_name);

    addSymbolToFunctionParameters(symbol, true);
    //free(secondary_name);
}

void addSymbolToFunctionParameters(Symbol* symbol, bool is_optional) {
    if (phase == PREPARSE) {
        symbol->role = PARAM;
    } else if (phase == PROGRAM) {
        symbol->role = CALL_PARAM;
    }
    setScopeless(symbol);

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

void addFunctionCallParameterBool(bool b) {
    union Value value;
    value.b = b;
    Symbol* symbol = addSymbol(NULL, K_BOOL, value, V_BOOL);
    addSymbolToFunctionParameters(symbol, false);
}

void addFunctionCallParameterInt(long long i) {
    union Value value;
    value.i = i;
    Symbol* symbol = addSymbol(NULL, K_NUMBER, value, V_INT);
    addSymbolToFunctionParameters(symbol, false);
}

void addFunctionCallParameterFloat(long double f) {
    union Value value;
    value.f = f;
    Symbol* symbol = addSymbol(NULL, K_NUMBER, value, V_FLOAT);
    addSymbolToFunctionParameters(symbol, false);
}

void addFunctionCallParameterString(char *s) {
    union Value value;
    value.s = malloc(1 + strlen(s));
    strcpy(value.s, s);
    Symbol* symbol = addSymbol(NULL, K_STRING, value, V_STRING);
    addSymbolToFunctionParameters(symbol, false);
}

void addFunctionCallParameterSymbol(char *name) {
    addSymbolToFunctionParameters(getSymbolFunctionParameter(name), false);
}

void addFunctionCallParameterList(enum Type type) {
    Symbol* symbol = finishComplexMode(NULL, type);
    addSymbolToFunctionParameters(symbol, false);
}

void returnSymbol(char *name) {
    Symbol* symbol = getSymbol(name);
    if (symbol->type != K_ANY &&
        executed_function->type != K_ANY &&
        symbol->type != executed_function->type
    ) {
        free(name);
        throw_error(E_ILLEGAL_VARIABLE_TYPE_FOR_FUNCTION, getTypeName(symbol->type), executed_function->name);
    }
    if (symbol->secondary_type != K_ANY &&
        executed_function->secondary_type != K_ANY &&
        symbol->secondary_type != executed_function->secondary_type
    ) {
        free(name);
        throw_error(E_ILLEGAL_VARIABLE_TYPE_FOR_FUNCTION, getTypeName(symbol->secondary_type), executed_function->name);
    }

    scope_override = executed_function->parent_scope;
    executed_function->symbol = createCloneFromSymbol(
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
    //free(name);
}

void printFunctionReturn(char *name, char *module, char *end, bool pretty, bool escaped) {
    _Function* function = getFunction(name, module);
    if (function->symbol == NULL) {
        append_to_array_without_malloc(&free_string_stack, name);
        throw_error(E_FUNCTION_DID_NOT_RETURN_ANYTHING, name);
        return;
    }
    printSymbolValueEndWith(function->symbol, end, pretty, escaped);
}

void createCloneFromFunctionReturn(char *clone_name, enum Type type, char *name, char *module, enum Type extra_type) {
    _Function* function = getFunction(name, module);
    if (function->symbol == NULL) {
        append_to_array_without_malloc(&free_string_stack, name);
        throw_error(E_FUNCTION_DID_NOT_RETURN_ANYTHING, name);
        return;
    }
    Symbol* clone_symbol = createCloneFromSymbol(clone_name, type, function->symbol, extra_type);
    //free(name);
    //free(clone_name);
    //free(module);
}

void updateSymbolByClonningFunctionReturn(char *clone_name, char *name, char*module) {
    _Function* function = getFunction(name, module);
    if (function->symbol == NULL) {
        append_to_array_without_malloc(&free_string_stack, name);
        throw_error(E_FUNCTION_DID_NOT_RETURN_ANYTHING, name);
        return;
    }
    updateSymbolByClonning(clone_name, function->symbol);
    //free(clone_name);
    //free(name);
    //free(module);
}

void updateComplexSymbolByClonningFunctionReturn(char *name, char*module) {
    _Function* function = getFunction(name, module);
    if (function->symbol == NULL) {
        append_to_array_without_malloc(&free_string_stack, name);
        throw_error(E_FUNCTION_DID_NOT_RETURN_ANYTHING, name);
        return;
    }
    updateComplexElementSymbol(function->symbol);
    //free(name);
    //free(module);
}

void initMainFunction() {
    main_function = (struct _Function*)malloc(sizeof(_Function));
    main_function->name = "main";
    main_function->type = K_ANY;
    main_function->parameter_count = 0;
    recursion_depth = 0;
    function_names_buffer.capacity = 0;
    function_names_buffer.size = 0;
    decision_buffer = "";
    initScopeless();
    initMainContext();
    initKaosApi();
}

void initScopeless() {
    scopeless = (struct _Function*)malloc(sizeof(_Function));
    scopeless->name = "N/A";
    scopeless->type = K_ANY;
    scopeless->parameter_count = 0;
}

void freeFunction(_Function* function) {
    if (strlen(function->body) > 0)
        free(function->body);
    free(function->name);
    free(function->parameters);
    for (unsigned i = 0; i < function->decision_functions.size; i++) {
        free(function->decision_expressions.arr[i]);
        free(function->decision_functions.arr[i]);
    }
    free(function->decision_expressions.arr);
    free(function->decision_functions.arr);
    free(function->decision_default);
    free(function->context);
    free(function->module_context);
    free(function->module);
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
    switch (type)
    {
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

void executeDecision(_Function* function) {
    if (function->decision_node == NULL) {
        return;
    }
    _Function* executed_function_backup = executed_function;

    eval_node(function->decision_node, function->module_context);
    stop_ast_evaluation = false;

    executed_function = executed_function_backup;

    if (executed_function_backup->type != K_VOID && executed_function_backup->symbol == NULL) {
        executed_function_backup->symbol = createCloneFromSymbol(
            NULL,
            decision_symbol_chain->type,
            decision_symbol_chain,
            decision_symbol_chain->secondary_type
        );
        executed_function_backup->symbol->scope = executed_function->parent_scope;
    }
    if (decision_symbol_chain != NULL) {
        removeSymbol(decision_symbol_chain);
    }
}

void addFunctionNameToFunctionNamesBuffer(char *name) {
    append_to_array(&function_names_buffer, name);
    //free(name);
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
