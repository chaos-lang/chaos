#include <stdio.h>
#include <string.h>

#include "function.h"

void startFunction(char *name) {
    function_mode = getFunction(name);
    if (function_mode != NULL) {
        memset(function_mode->body, 0, strlen(function_mode->body));
        return;
    }
    function_mode = (struct Function*)malloc(sizeof(Function));
    function_mode->name = name;
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

    freeFunctionMode();
}

void endFunction() {
    if (function_mode == NULL) return;
    function_mode = NULL;
    freeFunctionMode();
}

void freeFunctionMode() {
    free(function_parameters_mode);
    function_parameters_mode = NULL;
}

void callFunction(char *name) {
    Function* function = getFunction(name);
    for (int i = 0; i < function->parameter_count; i++) {
        Symbol* parameter = function->parameters[i];
        Symbol* parameter_call = function_parameters_mode->parameters[i];

        parameter->type = parameter_call->type;

        parameter->value.b = parameter_call->value.b;
        parameter->value.i = parameter_call->value.i;
        parameter->value.c = parameter_call->value.c;
        parameter->value.s = parameter_call->value.s;
        parameter->value.f = parameter_call->value.f;

        parameter->name = parameter->secondary_name;
    }

    freeFunctionMode();

    injectCode(function->body);
}

Function* getFunction(char *name) {
    function_cursor = start_function;
    while (function_cursor != NULL) {
        if (function_cursor->name != NULL && strcmp(function_cursor->name, name) == 0) {
            Function* function = function_cursor;
            return function;
        }
        function_cursor = function_cursor->next;
    }
    return NULL;
    //throw_error(3, name);
}

void startFunctionParameters() {
    function_parameters_mode = (struct Function*)malloc(sizeof(Function));
    function_parameters_mode->parameter_count = 0;
}

void addFunctionParameter(char *secondary_name, enum Type type) {
    union Value value;
    Symbol* symbol = addSymbol(NULL, type, value);
    symbol->secondary_name = secondary_name;

    addSymbolToFunctionParameters(symbol);
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
    Symbol* symbol = addSymbol(NULL, BOOL, value);
    addSymbolToFunctionParameters(symbol);
}

void addFunctionCallParameterInt(int i) {
    union Value value;
    value.i = i;
    Symbol* symbol = addSymbol(NULL, INT, value);
    addSymbolToFunctionParameters(symbol);
}

void addFunctionCallParameterFloat(float f) {
    union Value value;
    value.f = f;
    Symbol* symbol = addSymbol(NULL, FLOAT, value);
    addSymbolToFunctionParameters(symbol);
}

void addFunctionCallParameterString(char *s) {
    union Value value;
    value.s = s;
    Symbol* symbol = addSymbol(NULL, STRING, value);
    addSymbolToFunctionParameters(symbol);
}
