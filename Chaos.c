#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "function.h"
#include "Chaos.h"

int defineFunction(char *name, enum Type type, char *params_name[], unsigned params_type[], unsigned short params_length) {
    char *function_name = malloc(strlen(name) + 1);
    strcpy(function_name, name);
    startFunctionParameters();

    for (unsigned short i = 0; i < params_length; i++) {
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

long long getVariableInt(char *name) {
    char *symbol_name = malloc(strlen(name) + 1);
    strcpy(symbol_name, name);
    return getSymbolValueInt(symbol_name);
}

long double getVariableFloat(char *name) {
    char *symbol_name = malloc(strlen(name) + 1);
    strcpy(symbol_name, name);
    return getSymbolValueFloat(symbol_name);
}

char* getVariableString(char *name) {
    char *symbol_name = malloc(strlen(name) + 1);
    strcpy(symbol_name, name);
    return getSymbolValueString(symbol_name);
}

bool getListElementBool(char *name, long long i) {
    char *symbol_name = malloc(strlen(name) + 1);
    strcpy(symbol_name, name);
    Symbol* symbol = getListElement(getSymbol(symbol_name), i);
    if (symbol->value_type != V_BOOL) {
        throw_error(E_UNEXPECTED_VALUE_TYPE, getValueTypeName(symbol->value_type), symbol->name);
    }
    return symbol->value.b;
}

long long getListElementInt(char *name, long long i) {
    char *symbol_name = malloc(strlen(name) + 1);
    strcpy(symbol_name, name);
    Symbol* symbol = getListElement(getSymbol(symbol_name), i);
    if (symbol->value_type != V_INT) {
        throw_error(E_UNEXPECTED_VALUE_TYPE, getValueTypeName(symbol->value_type), symbol->name);
    }
    return symbol->value.i;
}

long double getListElementFloat(char *name, long long i) {
    char *symbol_name = malloc(strlen(name) + 1);
    strcpy(symbol_name, name);
    Symbol* symbol = getListElement(getSymbol(symbol_name), i);
    free(symbol_name);
    if (symbol->value_type != V_FLOAT) {
        throw_error(E_UNEXPECTED_VALUE_TYPE, getValueTypeName(symbol->value_type), symbol->name);
    }
    return symbol->value.f;
}

char* getListElementString(char *name, long long i) {
    char *symbol_name = malloc(strlen(name) + 1);
    strcpy(symbol_name, name);
    Symbol* symbol = getListElement(getSymbol(symbol_name), i);
    if (symbol->value_type != V_STRING) {
        throw_error(E_UNEXPECTED_VALUE_TYPE, getValueTypeName(symbol->value_type), symbol->name);
    }
    char* value;
    value = malloc(1 + strlen(symbol->value.s));
    strcpy(value, symbol->value.s);
    return value;
}

bool getDictElementBool(char *name, char *key) {
    char *symbol_name = malloc(strlen(name) + 1);
    strcpy(symbol_name, name);
    Symbol* symbol = getDictElement(getSymbol(symbol_name), key);
    if (symbol->value_type != V_BOOL) {
        throw_error(E_UNEXPECTED_VALUE_TYPE, getValueTypeName(symbol->value_type), symbol->name);
    }
    return symbol->value.b;
}

long long getDictElementInt(char *name, char *key) {
    char *symbol_name = malloc(strlen(name) + 1);
    strcpy(symbol_name, name);
    Symbol* symbol = getDictElement(getSymbol(symbol_name), key);
    if (symbol->value_type != V_INT) {
        throw_error(E_UNEXPECTED_VALUE_TYPE, getValueTypeName(symbol->value_type), symbol->name);
    }
    return symbol->value.i;
}

long double getDictElementFloat(char *name, char *key) {
    char *symbol_name = malloc(strlen(name) + 1);
    strcpy(symbol_name, name);
    Symbol* symbol = getDictElement(getSymbol(symbol_name), key);
    if (symbol->value_type != V_FLOAT) {
        throw_error(E_UNEXPECTED_VALUE_TYPE, getValueTypeName(symbol->value_type), symbol->name);
    }
    return symbol->value.f;
}

char* getDictElementString(char *name, char *key) {
    char *symbol_name = malloc(strlen(name) + 1);
    strcpy(symbol_name, name);
    Symbol* symbol = getDictElement(getSymbol(symbol_name), key);
    free(symbol_name);
    if (symbol->value_type != V_STRING) {
        throw_error(E_UNEXPECTED_VALUE_TYPE, getValueTypeName(symbol->value_type), symbol->name);
    }
    char* value;
    value = malloc(1 + strlen(symbol->value.s));
    strcpy(value, symbol->value.s);
    return value;
}

void returnVariableBool(bool b) {
    Symbol* symbol = addSymbolBool(NULL, b);
    returnVariable(symbol);
}

void returnVariableInt(long long i) {
    Symbol* symbol = addSymbolInt(NULL, i);
    returnVariable(symbol);
}

void returnVariableFloat(long double f) {
    Symbol* symbol = addSymbolFloat(NULL, f);
    returnVariable(symbol);
}

void returnVariableString(char *s) {
    Symbol* symbol = addSymbolString(NULL, s);
    returnVariable(symbol);
}

void createVariableBool(char *name, bool b) {
    char *symbol_name = malloc(strlen(name) + 1);
    strcpy(symbol_name, name);
    addSymbolBool(symbol_name, b);
}

void createVariableInt(char *name, long long i) {
    char *symbol_name = malloc(strlen(name) + 1);
    strcpy(symbol_name, name);
    addSymbolInt(symbol_name, i);
}

void createVariableFloat(char *name, long double f) {
    char *symbol_name = malloc(strlen(name) + 1);
    strcpy(symbol_name, name);
    addSymbolFloat(symbol_name, f);
}

void createVariableString(char *name, char *s) {
    char *symbol_name = malloc(strlen(name) + 1);
    strcpy(symbol_name, name);
    char *_s = malloc(strlen(s) + 1);
    strcpy(_s, s);
    addSymbolString(symbol_name, _s);
}

void startBuildingList() {
    addSymbolList(NULL);
}

void returnList(enum Type type) {
    returnComplex(type);
}

void startBuildingDict() {
    addSymbolDict(NULL);
}

void returnDict(enum Type type) {
    returnComplex(type);
}

void returnComplex(enum Type type) {
    complex_mode_stack.arr[complex_mode_stack.size - 1]->children_count = complex_mode_stack.child_counter[complex_mode_stack.size - 1];
    complex_mode_stack.arr[complex_mode_stack.size - 1]->secondary_type = type;
    Symbol* symbol = complex_mode_stack.arr[complex_mode_stack.size - 1];
    popComplexModeStack();
    returnVariable(symbol);
}
