#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "function.h"
#include "Chaos.h"

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

bool getArrayElementBool(char *name, int i) {
    char *symbol_name = malloc(strlen(name) + 1);
    strcpy(symbol_name, name);
    Symbol* symbol = getArrayElement(symbol_name, i);
    if (symbol->value_type != V_BOOL) {
        throw_error(E_UNEXPECTED_VALUE_TYPE, getValueTypeName(symbol->value_type), symbol->name);
    }
    return symbol->value.b;
}

int getArrayElementInt(char *name, int i) {
    char *symbol_name = malloc(strlen(name) + 1);
    strcpy(symbol_name, name);
    Symbol* symbol = getArrayElement(symbol_name, i);
    if (symbol->value_type != V_INT) {
        throw_error(E_UNEXPECTED_VALUE_TYPE, getValueTypeName(symbol->value_type), symbol->name);
    }
    return symbol->value.i;
}

float getArrayElementFloat(char *name, int i) {
    char *symbol_name = malloc(strlen(name) + 1);
    strcpy(symbol_name, name);
    Symbol* symbol = getArrayElement(symbol_name, i);
    if (symbol->value_type != V_FLOAT) {
        throw_error(E_UNEXPECTED_VALUE_TYPE, getValueTypeName(symbol->value_type), symbol->name);
    }
    return symbol->value.f;
}

char* getArrayElementString(char *name, int i) {
    char *symbol_name = malloc(strlen(name) + 1);
    strcpy(symbol_name, name);
    Symbol* symbol = getArrayElement(symbol_name, i);
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
    Symbol* symbol = getDictElement(symbol_name, key);
    if (symbol->value_type != V_BOOL) {
        throw_error(E_UNEXPECTED_VALUE_TYPE, getValueTypeName(symbol->value_type), symbol->name);
    }
    return symbol->value.b;
}

int getDictElementInt(char *name, char *key) {
    char *symbol_name = malloc(strlen(name) + 1);
    strcpy(symbol_name, name);
    Symbol* symbol = getDictElement(symbol_name, key);
    if (symbol->value_type != V_INT) {
        throw_error(E_UNEXPECTED_VALUE_TYPE, getValueTypeName(symbol->value_type), symbol->name);
    }
    return symbol->value.i;
}

float getDictElementFloat(char *name, char *key) {
    char *symbol_name = malloc(strlen(name) + 1);
    strcpy(symbol_name, name);
    Symbol* symbol = getDictElement(symbol_name, key);
    if (symbol->value_type != V_FLOAT) {
        throw_error(E_UNEXPECTED_VALUE_TYPE, getValueTypeName(symbol->value_type), symbol->name);
    }
    return symbol->value.f;
}

char* getDictElementString(char *name, char *key) {
    char *symbol_name = malloc(strlen(name) + 1);
    strcpy(symbol_name, name);
    Symbol* symbol = getDictElement(symbol_name, key);
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

void returnVariableInt(int i) {
    Symbol* symbol = addSymbolInt(NULL, i);
    returnVariable(symbol);
}

void returnVariableFloat(float f) {
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

void createVariableInt(char *name, int i) {
    char *symbol_name = malloc(strlen(name) + 1);
    strcpy(symbol_name, name);
    addSymbolInt(symbol_name, i);
}

void createVariableFloat(char *name, float f) {
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

void startBuildingArray() {
    addSymbolArray(NULL);
}

void returnArray(enum Type type) {
    returnComplex(type);
}

void startBuildingDict() {
    addSymbolDict(NULL);
}

void returnDict(enum Type type) {
    returnComplex(type);
}

void returnComplex(enum Type type) {
    complex_mode->children_count = symbol_counter;
    complex_mode->secondary_type = type;
    Symbol* symbol = complex_mode;
    complex_mode = NULL;
    symbol_counter = 0;
    returnVariable(symbol);
}
