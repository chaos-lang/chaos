#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "function.h"
#include "chaos.h"

int defineFunction(char *name, enum Type type, char *params_name[], int params_type[], int params_length) {
    char *function_name = malloc(strlen(name) + 1);
    strcpy(function_name, name);
    startFunctionParameters();

    for (int i = 0; i < params_length; i++) {
        char *param_name = malloc(strlen(params_name[i]) + 1);
        strcpy(param_name, params_name[i]);
        addFunctionParameter(param_name, params_type[i]);
    }

    startFunction(function_name, type);
    endFunction();
    return 0;
}

bool getVariableBool(char *name) {
    char *symbol_name = malloc(strlen(name) + 1);
    strcpy(symbol_name, name);
    return getSymbolValueBool(symbol_name);
}

int getVariableInt(char *name) {
    char *symbol_name = malloc(strlen(name) + 1);
    strcpy(symbol_name, name);
    return getSymbolValueInt(symbol_name);
}

float getVariableFloat(char *name) {
    char *symbol_name = malloc(strlen(name) + 1);
    strcpy(symbol_name, name);
    return getSymbolValueFloat(symbol_name);
}

char* getVariableString(char *name) {
    char *symbol_name = malloc(strlen(name) + 1);
    strcpy(symbol_name, name);
    return getSymbolValueString(symbol_name);
}
