#include <stdio.h>
#include <string.h>

#include "function.h"

function_parameters_counter = 0;

void startFunction(char *name) {
    function_mode = getFunction(name);
    if (function_mode != NULL) {
        memset(function_mode->body, 0, strlen(function_mode->body));
        return;
    }
    function_mode = (struct Function*)malloc(sizeof(Function));
    function_mode->name = name;

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
        sizeof(Symbol) * function_parameters_counter
    );

    memcpy(
        function_mode->parameters,
        function_parameters_mode->parameters,
        function_parameters_counter * sizeof(Symbol)
    );

    free(function_parameters_mode);
    function_parameters_mode = NULL;
    function_parameters_counter = 0;
}

void endFunction() {
    if (function_mode == NULL) return;
    function_mode = NULL;
}

void callFunction(char *name) {
    Function* function = getFunction(name);
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
}

void addFunctionParameter(char *secondary_name, enum Type type) {
    union Value value;
    Symbol* symbol = addSymbol(NULL, type, value);
    symbol->secondary_name = secondary_name;

    addSymbolToFunctionParameters(symbol);
}

void addSymbolToFunctionParameters(Symbol* symbol) {
    function_parameters_mode->parameters = realloc(
        function_parameters_mode->parameters,
        sizeof(Symbol) * ++function_parameters_counter
    );

    if (function_parameters_mode->parameters == NULL) {
        //throw_error(4, complex_mode->name);
    }

    function_parameters_mode->parameters[function_parameters_counter - 1] = symbol;
}
