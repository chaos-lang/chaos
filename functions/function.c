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
