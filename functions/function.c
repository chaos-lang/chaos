#include "function.h"

extern int yylineno;
bool interactive_shell_function_error_absorbed = false;
extern int yyparse();

int reset_line_no_to = 0;

void startFunction(char *name, enum Type type) {
    if (is_interactive) {
        phase = PREPARSE;
    }

    function_mode = getFunction(name, NULL);
    if (function_mode != NULL) {
        free(function_mode->body);
        function_mode->body = "";
        free(name);
        for (unsigned short i = 0; i < function_parameters_mode->parameter_count; i++) {
            Symbol* parameter = function_parameters_mode->parameters[i];
            removeSymbol(parameter);
        }

        if (is_interactive) {
            phase = PROGRAM;
        }
        return;
    }

    if (function_names_buffer.size > 0) {
        if (!isInFunctionNamesBuffer(name)) {
            free(name);
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
    function_mode->parameter_count = 0;

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

    recordToken(strdup("\n"), 1);

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

    free(name);
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

    scope_override = function;
    for (unsigned short i = 0; i < function->parameter_count; i++) {
        Symbol* parameter = function->parameters[i];
        Symbol* parameter_call = function_parameters_mode->parameters[i];

        parameter_call->name = malloc(1 + strlen(parameter->secondary_name));
        strcpy(parameter_call->name, parameter->secondary_name);
        parameter_call->scope = function;
        parameter_call->recursion_depth = recursion_depth + 1;
        parameter_call->param_of = function;
    }
    scope_override = NULL;

    freeFunctionMode();

    _Function* parent_scope = getCurrentScope();
    executed_function = function;
    executed_function->parent_scope = parent_scope;

    pushModuleStack(executed_function->module_context, "");

    recursion_depth++;

    if (recursion_depth > __MAX_RECURSION_DEPTH__) {
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
        __DYNAMIC_LIBRARY_EXTENSION__
        ) == 0
        ) {
            callFunctionFromDynamicLibrary(function);
        } else {
            injectCode(function->body, INIT_PROGRAM);
        }
    }

    reset_line_no_to = 0;

    if (!interactive_shell_function_error_absorbed)
        executeDecision(function);

    if (function->type != K_VOID &&
        function->symbol == NULL &&
        !interactive_shell_function_error_absorbed
    ) {
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
            "\t{name: %s, type: %u, parameter_count: %hu, decision_length: %u, context: %s, module_context: %s, module: %s} =>\n",
            function->name,
            function->type,
            function->parameter_count,
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
}

void addFunctionParameter(char *secondary_name, enum Type type) {
    union Value value;
    Symbol* symbol = addSymbol(NULL, type, value, V_VOID);
    symbol->secondary_name = malloc(1 + strlen(secondary_name));
    strcpy(symbol->secondary_name, secondary_name);

    addSymbolToFunctionParameters(symbol);
    free(secondary_name);
}

void addSymbolToFunctionParameters(Symbol* symbol) {
    if (phase == PREPARSE) {
        symbol->role = PARAM;
    } else if (phase == PROGRAM) {
        symbol->role = CALL_PARAM;
    }
    symbol->scope = scopeless;

    if (function_parameters_mode == NULL) {
        startFunctionParameters();
    }

    function_parameters_mode->parameters = realloc(
        function_parameters_mode->parameters,
        sizeof(Symbol) * ++function_parameters_mode->parameter_count
    );

    if (function_parameters_mode->parameters == NULL) {
        throw_error(E_MEMORY_ALLOCATION_FOR_FUNCTION_FAILED, NULL);
    }

    function_parameters_mode->parameters[function_parameters_mode->parameter_count - 1] = symbol;
}

void addFunctionCallParameterBool(bool b) {
    union Value value;
    value.b = b;
    Symbol* symbol = addSymbol(NULL, K_BOOL, value, V_BOOL);
    addSymbolToFunctionParameters(symbol);
}

void addFunctionCallParameterInt(long long i) {
    union Value value;
    value.i = i;
    Symbol* symbol = addSymbol(NULL, K_NUMBER, value, V_INT);
    addSymbolToFunctionParameters(symbol);
}

void addFunctionCallParameterFloat(float f) {
    union Value value;
    value.f = f;
    Symbol* symbol = addSymbol(NULL, K_NUMBER, value, V_FLOAT);
    addSymbolToFunctionParameters(symbol);
}

void addFunctionCallParameterString(char *s) {
    union Value value;
    value.s = malloc(1 + strlen(s));
    strcpy(value.s, s);
    Symbol* symbol = addSymbol(NULL, K_STRING, value, V_STRING);
    addSymbolToFunctionParameters(symbol);
}

void addFunctionCallParameterSymbol(char *name) {
    addSymbolToFunctionParameters(getSymbolFunctionParameter(name));
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
    free(name);
}

void printFunctionReturn(char *name, char *module) {
    _Function* function = getFunction(name, module);
    if (function->symbol == NULL) {
        throw_error(E_FUNCTION_DID_NOT_RETURN_ANYTHING, name);
        return;
    }
    printSymbolValueEndWithNewLine(function->symbol);
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
}

void initScopeless() {
    scopeless = (struct _Function*)malloc(sizeof(_Function));
    scopeless->name = "N/A";
    scopeless->type = K_ANY;
    scopeless->parameter_count = 0;
}

void freeFunction(_Function* function) {
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
    if (function->decision_functions.size <= 0) {
        return;
    }
    _Function* executed_function_backup = executed_function;

    union Value value;
    value.b = false;
    bool is_decision_made = false;
    char *expression_buffer = "";
    char *function_buffer = "";
    char *name = malloc(1 + strlen(__LANGUAGE_NAME__));
    strcpy(name, __LANGUAGE_NAME__);
    Symbol* symbol = addSymbol(name, K_BOOL, value, V_BOOL);

    for (unsigned i = 0; i < function->decision_functions.size; i++) {
        expression_buffer = strcat_ext(expression_buffer, __LANGUAGE_NAME__);
        expression_buffer = strcat_ext(expression_buffer, " = ");
        expression_buffer = strcat_ext(expression_buffer, function->decision_expressions.arr[i]);
        expression_buffer = strcat_ext(expression_buffer, "\n");

        injectCode(expression_buffer, INIT_PROGRAM);

        if (symbol->value.b) {
            function_buffer = strcat_ext(function_buffer, function->decision_functions.arr[i]);
            function_buffer = strcat_ext(function_buffer, "\n");
            injectCode(function_buffer, INIT_PROGRAM);
            is_decision_made = true;
            break;
        }
    }

    removeSymbol(symbol);

    if (!is_decision_made && function->decision_default != NULL) {
        function_buffer = strcat_ext(function_buffer, function->decision_default);
        function_buffer = strcat_ext(function_buffer, "\n");
        injectCode(function_buffer, INIT_PROGRAM);
    }

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

    free(expression_buffer);
    free(function_buffer);
}

void addFunctionNameToFunctionNamesBuffer(char *name) {
    append_to_array(&function_names_buffer, name);
    free(name);
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
