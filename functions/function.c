#include <stdio.h>
#include <string.h>

#include "function.h"
#include "../utilities/injector.h"

void startFunction(char *name, enum Type type) {
    function_mode = getFunction(name);
    if (function_mode != NULL) {
        memset(function_mode->body, 0, strlen(function_mode->body));
        free(name);
        for (int i = 0; i < function_parameters_mode->parameter_count; i++) {
            Symbol* parameter = function_parameters_mode->parameters[i];
            removeSymbol(parameter);
        }
        return;
    }
    function_mode = (struct _Function*)calloc(1, sizeof(_Function));
    function_mode->name = malloc(1 + strlen(name));
    strcpy(function_mode->name, name);
    function_mode->type = type;
    function_mode->parameter_count = 0;
    function_mode->decision_length = 0;

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

    memcpy(
        function_mode->parameters,
        function_parameters_mode->parameters,
        function_parameters_mode->parameter_count * sizeof(Symbol)
    );
    function_mode->parameter_count = function_parameters_mode->parameter_count;


    for (int i = 0; i < function_mode->parameter_count; i++) {
        Symbol* parameter = function_mode->parameters[i];
        parameter->param_of = function_mode;
    }

    #if !defined(_WIN32) && !defined(_WIN64) && !defined(__CYGWIN__)
    if (name != NULL) {
        char suggestion[80];
        strcpy(suggestion, name);
        strcat(suggestion, "()");
        add_suggestion(suggestion);
    }
    #endif

    free(name);
    freeFunctionMode();
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

void callFunction(char *name) {
    _Function* function = getFunction(name);
    scope_override = function;
    for (int i = 0; i < function->parameter_count; i++) {
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

    recursion_depth++;

    if (recursion_depth > __MAX_RECURSION_DEPTH__) {
        throw_error(17, NULL);
    }

    injectCode(function->body);

    executeDecision(function);

    for (int i = 0; i < function->parameter_count; i++) {
        Symbol* parameter = function->parameters[i];
        removeSymbolByName(parameter->secondary_name);
    }

    recursion_depth--;

    removeSymbolsByScope(function);

    executed_function = NULL;
}

_Function* getFunction(char *name) {
    function_cursor = start_function;
    while (function_cursor != NULL) {
        if (function_cursor->name != NULL && strcmp(function_cursor->name, name) == 0) {
            _Function* function = function_cursor;
            return function;
        }
        function_cursor = function_cursor->next;
    }
    if (phase == PROGRAM) {
        throw_error(15, name);
    }
    return NULL;
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
        throw_error(16, NULL);
    }

    function_parameters_mode->parameters[function_parameters_mode->parameter_count - 1] = symbol;
}

void addFunctionCallParameterBool(bool b) {
    union Value value;
    value.b = b;
    Symbol* symbol = addSymbol(NULL, BOOL, value, V_BOOL);
    addSymbolToFunctionParameters(symbol);
}

void addFunctionCallParameterInt(int i) {
    union Value value;
    value.i = i;
    Symbol* symbol = addSymbol(NULL, NUMBER, value, V_INT);
    addSymbolToFunctionParameters(symbol);
}

void addFunctionCallParameterFloat(float f) {
    union Value value;
    value.f = f;
    Symbol* symbol = addSymbol(NULL, NUMBER, value, V_FLOAT);
    addSymbolToFunctionParameters(symbol);
}

void addFunctionCallParameterString(char *s) {
    union Value value;
    value.s = malloc(1 + strlen(s));
    strcpy(value.s, s);
    Symbol* symbol = addSymbol(NULL, STRING, value, V_STRING);
    addSymbolToFunctionParameters(symbol);
}

void addFunctionCallParameterSymbol(char *name) {
    addSymbolToFunctionParameters(getSymbolFunctionParameter(name));
}

void returnSymbol(char *name) {
    Symbol* symbol = getSymbol(name);
    if (symbol->type != executed_function->type) {
        free(name);
        throw_error(14, executed_function->name);
    }
    scope_override = executed_function->parent_scope;
    executed_function->symbol = createCloneFromSymbol(
        NULL,
        symbol->type,
        symbol,
        symbol->secondary_type
    );
    scope_override = NULL;
    free(name);
}

void printFunctionReturn(char *name) {
    _Function* function = getFunction(name);
    printSymbolValueEndWithNewLine(function->symbol);
}

void initMainFunction() {
    main_function = (struct _Function*)malloc(sizeof(_Function));
    main_function->name = "main";
    main_function->type = ANY;
    main_function->parameter_count = 0;
    recursion_depth = 0;
    initScopeless();
}

void initScopeless() {
    scopeless = (struct _Function*)malloc(sizeof(_Function));
    scopeless->name = "N/A";
    scopeless->type = ANY;
    scopeless->parameter_count = 0;
}

void freeFunction(_Function* function) {
    free(function->name);
    free(function->parameters);
    for (int i = 0; i < function->decision_length; i++) {
        free(function->decision_expressions[i]);
        free(function->decision_functions[i]);
    }
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

void finishDecisionMode() {
    decision_mode->decision_functions[decision_mode->decision_length] = malloc(1 + strlen(trim_string(decision_buffer)));
    strcpy(decision_mode->decision_functions[decision_mode->decision_length], trim_string(decision_buffer));
    memset(decision_buffer, 0, strlen(trim_string(decision_buffer)));
    decision_mode->decision_length++;
    decision_mode = NULL;
    decision_expression_mode = NULL;
    decision_function_mode = NULL;
}

void executeDecision(_Function* function) {
    if (function->decision_length <= 0) {
        return;
    }
    _Function* executed_function_backup = executed_function;

    union Value value;
    value.b = false;
    char expression_buffer[1000] = "";
    char function_buffer[1000] = "";
    char *name = malloc(1 + strlen(__LANGUAGE_NAME__));
    strcpy(name, __LANGUAGE_NAME__);
    Symbol* symbol = addSymbol(name, BOOL, value, V_BOOL);

    for (int i = 0; i < function->decision_length; i++) {
        strcat(expression_buffer, __LANGUAGE_NAME__);
        strcat(expression_buffer, " = ");
        strcat(expression_buffer, function->decision_expressions[i]);
        strcat(expression_buffer, "\n");

        injectCode(expression_buffer);

        if (symbol->value.b) {
            strcat(function_buffer, function->decision_functions[i]);
            strcat(function_buffer, "\n");
            injectCode(function_buffer);
            break;
        }
    }

    removeSymbol(symbol);

    executed_function = executed_function_backup;
}
