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

bool getVariableBoolByTypeCasting(char *name) {
    char *symbol_name = malloc(strlen(name) + 1);
    strcpy(symbol_name, name);
    Symbol* symbol = getSymbol(symbol_name);
    free(symbol_name);
    return symbolValueByTypeCastingToBool(symbol);
}

long long getVariableIntByTypeCasting(char *name) {
    char *symbol_name = malloc(strlen(name) + 1);
    strcpy(symbol_name, name);
    Symbol* symbol = getSymbol(symbol_name);
    free(symbol_name);
    return symbolValueByTypeCastingToInt(symbol);
}

long double getVariableFloatByTypeCasting(char *name) {
    char *symbol_name = malloc(strlen(name) + 1);
    strcpy(symbol_name, name);
    Symbol* symbol = getSymbol(symbol_name);
    free(symbol_name);
    return symbolValueByTypeCastingToFloat(symbol);
}

char* getVariableStringByTypeCasting(char *name) {
    char *symbol_name = malloc(strlen(name) + 1);
    strcpy(symbol_name, name);
    Symbol* symbol = getSymbol(symbol_name);
    free(symbol_name);
    return symbolValueByTypeCastingToString(symbol);
}

unsigned long getListLength(char *name) {
    char *symbol_name = malloc(strlen(name) + 1);
    strcpy(symbol_name, name);
    Symbol* symbol = getSymbol(symbol_name);
    free(symbol_name);
    return symbol->children_count;
}

bool getListElementBool(char *name, long long i) {
    char *symbol_name = malloc(strlen(name) + 1);
    strcpy(symbol_name, name);
    Symbol* symbol = getListElement(getSymbol(symbol_name), i);
    free(symbol_name);
    if (symbol->value_type != V_BOOL) {
        throw_error(E_UNEXPECTED_VALUE_TYPE, getValueTypeName(symbol->value_type), symbol->name);
    }
    return _getSymbolValueBool(symbol);
}

long long getListElementInt(char *name, long long i) {
    char *symbol_name = malloc(strlen(name) + 1);
    strcpy(symbol_name, name);
    Symbol* symbol = getListElement(getSymbol(symbol_name), i);
    free(symbol_name);
    if (symbol->value_type != V_INT) {
        throw_error(E_UNEXPECTED_VALUE_TYPE, getValueTypeName(symbol->value_type), symbol->name);
    }
    return _getSymbolValueInt(symbol);
}

long double getListElementFloat(char *name, long long i) {
    char *symbol_name = malloc(strlen(name) + 1);
    strcpy(symbol_name, name);
    Symbol* symbol = getListElement(getSymbol(symbol_name), i);
    free(symbol_name);
    if (symbol->value_type != V_FLOAT) {
        throw_error(E_UNEXPECTED_VALUE_TYPE, getValueTypeName(symbol->value_type), symbol->name);
    }
    return _getSymbolValueFloat(symbol);
}

char* getListElementString(char *name, long long i) {
    char *symbol_name = malloc(strlen(name) + 1);
    strcpy(symbol_name, name);
    Symbol* symbol = getListElement(getSymbol(symbol_name), i);
    free(symbol_name);
    if (symbol->value_type != V_STRING) {
        throw_error(E_UNEXPECTED_VALUE_TYPE, getValueTypeName(symbol->value_type), symbol->name);
    }
    return _getSymbolValueString(symbol);
}

bool getListElementBoolByTypeCasting(char *name, long long i) {
    char *symbol_name = malloc(strlen(name) + 1);
    strcpy(symbol_name, name);
    Symbol* symbol = getListElement(getSymbol(symbol_name), i);
    free(symbol_name);
    return symbolValueByTypeCastingToBool(symbol);
}

long long getListElementIntByTypeCasting(char *name, long long i) {
    char *symbol_name = malloc(strlen(name) + 1);
    strcpy(symbol_name, name);
    Symbol* symbol = getListElement(getSymbol(symbol_name), i);
    free(symbol_name);
    return symbolValueByTypeCastingToInt(symbol);
}

long double getListElementFloatByTypeCasting(char *name, long long i) {
    char *symbol_name = malloc(strlen(name) + 1);
    strcpy(symbol_name, name);
    Symbol* symbol = getListElement(getSymbol(symbol_name), i);
    free(symbol_name);
    return symbolValueByTypeCastingToFloat(symbol);
}

char* getListElementStringByTypeCasting(char *name, long long i) {
    char *symbol_name = malloc(strlen(name) + 1);
    strcpy(symbol_name, name);
    Symbol* symbol = getListElement(getSymbol(symbol_name), i);
    free(symbol_name);
    return symbolValueByTypeCastingToString(symbol);
}

void copyListElement(char *name, long long i) {
    char *symbol_name = malloc(strlen(name) + 1);
    strcpy(symbol_name, name);
    Symbol* symbol = getListElement(getSymbol(symbol_name), i);
    free(symbol_name);
    createCloneFromSymbol(
        NULL,
        symbol->type,
        symbol,
        symbol->secondary_type
    );
}

enum Type getListElementType(char *name, long long i) {
    char *symbol_name = malloc(strlen(name) + 1);
    strcpy(symbol_name, name);
    Symbol* symbol = getListElement(getSymbol(symbol_name), i);
    free(symbol_name);
    return symbol->type;
}

enum ValueType getListElementValueType(char *name, long long i) {
    char *symbol_name = malloc(strlen(name) + 1);
    strcpy(symbol_name, name);
    Symbol* symbol = getListElement(getSymbol(symbol_name), i);
    free(symbol_name);
    return symbol->value_type;
}

unsigned long getDictLength(char *name) {
    char *symbol_name = malloc(strlen(name) + 1);
    strcpy(symbol_name, name);
    Symbol* symbol = getSymbol(symbol_name);
    free(symbol_name);
    return symbol->children_count;
}

char* getDictKeyByIndex(char *name, long long i) {
    char *symbol_name = malloc(strlen(name) + 1);
    strcpy(symbol_name, name);
    Symbol* symbol = getSymbol(symbol_name);
    free(symbol_name);
    Symbol* child = symbol->children[i];
    char* key;
    key = malloc(1 + strlen(child->key));
    strcpy(key, child->key);
    return key;
}

bool getDictElementBool(char *name, char *key) {
    char *symbol_name = malloc(strlen(name) + 1);
    strcpy(symbol_name, name);
    Symbol* symbol = getDictElement(getSymbol(symbol_name), key);
    free(symbol_name);
    if (symbol->value_type != V_BOOL) {
        throw_error(E_UNEXPECTED_VALUE_TYPE, getValueTypeName(symbol->value_type), symbol->name);
    }
    return _getSymbolValueBool(symbol);
}

long long getDictElementInt(char *name, char *key) {
    char *symbol_name = malloc(strlen(name) + 1);
    strcpy(symbol_name, name);
    Symbol* symbol = getDictElement(getSymbol(symbol_name), key);
    free(symbol_name);
    if (symbol->value_type != V_INT) {
        throw_error(E_UNEXPECTED_VALUE_TYPE, getValueTypeName(symbol->value_type), symbol->name);
    }
    return _getSymbolValueInt(symbol);
}

long double getDictElementFloat(char *name, char *key) {
    char *symbol_name = malloc(strlen(name) + 1);
    strcpy(symbol_name, name);
    Symbol* symbol = getDictElement(getSymbol(symbol_name), key);
    free(symbol_name);
    if (symbol->value_type != V_FLOAT) {
        throw_error(E_UNEXPECTED_VALUE_TYPE, getValueTypeName(symbol->value_type), symbol->name);
    }
    return _getSymbolValueFloat(symbol);
}

char* getDictElementString(char *name, char *key) {
    char *symbol_name = malloc(strlen(name) + 1);
    strcpy(symbol_name, name);
    Symbol* symbol = getDictElement(getSymbol(symbol_name), key);
    free(symbol_name);
    if (symbol->value_type != V_STRING) {
        throw_error(E_UNEXPECTED_VALUE_TYPE, getValueTypeName(symbol->value_type), symbol->name);
    }
    return _getSymbolValueString(symbol);
}

bool getDictElementBoolByTypeCasting(char *name, char *key) {
    char *symbol_name = malloc(strlen(name) + 1);
    strcpy(symbol_name, name);
    Symbol* symbol = getDictElement(getSymbol(symbol_name), key);
    free(symbol_name);
    return symbolValueByTypeCastingToBool(symbol);
}

long long getDictElementIntByTypeCasting(char *name, char *key) {
    char *symbol_name = malloc(strlen(name) + 1);
    strcpy(symbol_name, name);
    Symbol* symbol = getDictElement(getSymbol(symbol_name), key);
    free(symbol_name);
    return symbolValueByTypeCastingToInt(symbol);
}

long double getDictElementFloatByTypeCasting(char *name, char *key) {
    char *symbol_name = malloc(strlen(name) + 1);
    strcpy(symbol_name, name);
    Symbol* symbol = getDictElement(getSymbol(symbol_name), key);
    free(symbol_name);
    return symbolValueByTypeCastingToFloat(symbol);
}

char* getDictElementStringByTypeCasting(char *name, char *key) {
    char *symbol_name = malloc(strlen(name) + 1);
    strcpy(symbol_name, name);
    Symbol* symbol = getDictElement(getSymbol(symbol_name), key);
    free(symbol_name);
    return symbolValueByTypeCastingToString(symbol);
}

void copyDictElement(char *name, char *key) {
    char *symbol_name = malloc(strlen(name) + 1);
    strcpy(symbol_name, name);
    Symbol* symbol = getDictElement(getSymbol(symbol_name), key);
    free(symbol_name);
    createCloneFromSymbol(
        key,
        symbol->type,
        symbol,
        symbol->secondary_type
    );
}

enum Type getDictElementType(char *name, char *key) {
    char *symbol_name = malloc(strlen(name) + 1);
    strcpy(symbol_name, name);
    Symbol* symbol = getDictElement(getSymbol(symbol_name), key);
    free(symbol_name);
    return symbol->type;
}

enum ValueType getDictElementValueType(char *name, char *key) {
    char *symbol_name = malloc(strlen(name) + 1);
    strcpy(symbol_name, name);
    Symbol* symbol = getDictElement(getSymbol(symbol_name), key);
    free(symbol_name);
    return symbol->value_type;
}

char* dumpVariableToString(char *name, bool pretty, bool escaped, bool double_quotes) {
    Symbol* symbol = getSymbol(name);
    bool is_complex = false;
    if (symbol->type == K_LIST || symbol->type == K_DICT)
        is_complex = true;
    char *encoded = malloc(2);
    strcpy(encoded, " ");
    encoded = encodeSymbolValueToString(symbol, is_complex, pretty, escaped, 0, encoded, double_quotes);
    memmove(encoded, encoded + 1, strlen(encoded));
    return encoded;
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
    char *symbol_name = NULL;
    if (name != NULL) {
        symbol_name = malloc(strlen(name) + 1);
        strcpy(symbol_name, name);
    }
    addSymbolBool(symbol_name, b);
}

void createVariableInt(char *name, long long i) {
    char *symbol_name = NULL;
    if (name != NULL) {
        symbol_name = malloc(strlen(name) + 1);
        strcpy(symbol_name, name);
    }
    addSymbolInt(symbol_name, i);
}

void createVariableFloat(char *name, long double f) {
    char *symbol_name = NULL;
    if (name != NULL) {
        symbol_name = malloc(strlen(name) + 1);
        strcpy(symbol_name, name);
    }
    addSymbolFloat(symbol_name, f);
}

void createVariableString(char *name, char *s) {
    char *symbol_name = NULL;
    if (name != NULL) {
        symbol_name = malloc(strlen(name) + 1);
        strcpy(symbol_name, name);
    }
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

void finishList(enum Type type) {
    finishComplex(type);
}

void finishDict(enum Type type) {
    finishComplex(type);
}

void finishComplex(enum Type type) {
    complex_mode_stack.arr[complex_mode_stack.size - 1]->children_count = complex_mode_stack.child_counter[complex_mode_stack.size - 1];
    complex_mode_stack.arr[complex_mode_stack.size - 1]->secondary_type = type;
    Symbol* symbol = complex_mode_stack.arr[complex_mode_stack.size - 1];
    popComplexModeStack();
}

enum Type getListType(char *name) {
    Symbol* symbol = getSymbol(name);
    if (symbol->type != K_LIST)
        throw_error(E_NOT_A_LIST, name);
    return symbol->secondary_type;
}

enum Type getDictType(char *name) {
    Symbol* symbol = getSymbol(name);
    if (symbol->type != K_DICT)
        throw_error(E_NOT_A_DICT, name);
    return symbol->secondary_type;
}

enum ValueType getValueType(char *name) {
    Symbol* symbol = getSymbol(name);
    return symbol->value_type;
}

enum Role getRole(char *name) {
    Symbol* symbol = getSymbol(name);
    return symbol->role;
}

void raiseError(char *msg) {
    throw_error(E_RAISED_FROM_AN_EXTENSION, msg, NULL, 0, 0);
}

void parseJson(char *json) {
    injectCode(json, INIT_JSON_PARSE);
    phase = PROGRAM;
}
