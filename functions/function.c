#include <stdio.h>
#include <string.h>

#include "function.h"
#include "../utilities/injector.h"

void startFunction(char *name, enum Type type) {
    function_mode = getFunction(name);
    if (function_mode != NULL) {
        memset(function_mode->body, 0, strlen(function_mode->body));
        free(name);
        return;
    }
    function_mode = (struct _Function*)calloc(1, sizeof(_Function));
    function_mode->name = malloc(1 + strlen(name));
    strcpy(function_mode->name, name);
    function_mode->type = type;
    function_mode->parameter_count = 0;

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

    if (name != NULL) {
        char suggestion[80];
        strcpy(suggestion, name);
        strcat(suggestion, "()");
        add_suggestion(suggestion);
    }

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

        createCloneFromSymbol(parameter->secondary_name, parameter->type, parameter_call, parameter_call->secondary_type);
    }
    scope_override = NULL;

    freeFunctionMode();

    executed_function = function;

    injectCode(function->body);

    for (int i = 0; i < function->parameter_count; i++) {
        Symbol* parameter = function->parameters[i];
        removeSymbolByName(parameter->secondary_name);
    }

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
    return NULL;
    //throw_error(3, name);
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
    if (function_parameters_mode == NULL) {
        startFunctionParameters();
    }

    function_parameters_mode->parameters = realloc(
        function_parameters_mode->parameters,
        sizeof(Symbol) * ++function_parameters_mode->parameter_count
    );

    if (function_parameters_mode->parameters == NULL) {
        //throw_error(4, complex_mode->name);
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
        throw_error(13, executed_function->name);
    }
    executed_function->symbol = createCloneFromSymbol(
        NULL,
        symbol->type,
        symbol,
        symbol->secondary_type
    );
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
}

void freeFunction(_Function* function) {
    free(function->name);
    free(function->parameters);
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
