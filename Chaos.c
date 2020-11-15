/*
 * Description: Chaos C extension interface
 *
 * Copyright (c) 2019-2020 Chaos Language Development Authority <info@chaos-lang.org>
 *
 * License: GNU General Public License v3.0
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>
 *
 * Authors: M. Mert Yildiran <me@mertyildiran.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "interpreter/function.h"

#ifdef CHAOS_COMPILER
    #include "../Chaos.h"
#else
    #include "Chaos.h"
#endif

int defineFunction(
    char *name,
    enum Type type,
    enum Type secondary_type,
    char *params_name[],
    unsigned params_type[],
    unsigned params_secondary_type[],
    unsigned short params_length,
    KaosValue optional_params[],
    unsigned short optional_params_length
) {
    startFunctionParameters();

    unsigned short optional_params_starts_from = params_length - optional_params_length;

    unsigned short j = optional_params_length - 1;
    if (params_length > 0) {
        for (int i = (int) (params_length - 1); i >= optional_params_starts_from; i--) {
            enum Type optional_param_type = params_type[i];

            struct KaosValue optional_param_value = optional_params[j];

            switch (optional_param_type)
            {
                case K_BOOL:
                    addFunctionOptionalParameterBool(params_name[i], optional_param_value.b);
                    break;
                case K_NUMBER:
                    addFunctionOptionalParameterFloat(params_name[i], optional_param_value.f);
                    break;
                case K_STRING:
                    addFunctionOptionalParameterString(params_name[i], optional_param_value.s);
                    break;
                default:
                    throw_error(E_ILLEGAL_VARIABLE_TYPE_FOR_VARIABLE, name);
                    break;
            }
            j--;
        }

        for (int i = (int) optional_params_starts_from - 1; i >= 0; i--) {
            addFunctionParameter(params_name[i], params_type[i], params_secondary_type[i]);
        }
    }

#ifdef CHAOS_COMPILER
    startFunction(name, type, secondary_type, "", "", "", true);
#else
    startFunction(name, type, secondary_type);
#endif
    endFunction();
    return 0;
}

bool getVariableBool(char *name) {
    return getSymbolValueBool(name);
}

long long getVariableInt(char *name) {
    return getSymbolValueInt(name);
}

long double getVariableFloat(char *name) {
    return getSymbolValueFloat(name);
}

char* getVariableString(char *name) {
    return getSymbolValueString(name);
}

bool getVariableBoolByTypeCasting(char *name) {
    Symbol* symbol = getSymbol(name);
    return symbolValueByTypeCastingToBool(symbol);
}

long long getVariableIntByTypeCasting(char *name) {
    Symbol* symbol = getSymbol(name);
    return symbolValueByTypeCastingToInt(symbol);
}

long double getVariableFloatByTypeCasting(char *name) {
    Symbol* symbol = getSymbol(name);
    return symbolValueByTypeCastingToFloat(symbol);
}

char* getVariableStringByTypeCasting(char *name) {
    Symbol* symbol = getSymbol(name);
    return symbolValueByTypeCastingToString(symbol);
}

unsigned long getListLength(char *name) {
    Symbol* symbol = getSymbol(name);
    return symbol->children_count;
}

bool getListElementBool(char *name, long long i) {
    Symbol* symbol = getListElement(getSymbol(name), i);
    if (symbol->value_type != V_BOOL) {
        throw_error(E_UNEXPECTED_VALUE_TYPE, getValueTypeName(symbol->value_type), symbol->name);
    }
    return _getSymbolValueBool(symbol);
}

long long getListElementInt(char *name, long long i) {
    Symbol* symbol = getListElement(getSymbol(name), i);
    if (symbol->value_type != V_INT && symbol->value_type != V_FLOAT) {
        throw_error(E_UNEXPECTED_VALUE_TYPE, getValueTypeName(symbol->value_type), symbol->name);
    }
    return _getSymbolValueInt(symbol);
}

long double getListElementFloat(char *name, long long i) {
    Symbol* symbol = getListElement(getSymbol(name), i);
    if (symbol->value_type != V_INT && symbol->value_type != V_FLOAT) {
        throw_error(E_UNEXPECTED_VALUE_TYPE, getValueTypeName(symbol->value_type), symbol->name);
    }
    return _getSymbolValueFloat(symbol);
}

char* getListElementString(char *name, long long i) {
    Symbol* symbol = getListElement(getSymbol(name), i);
    if (symbol->value_type != V_STRING) {
        throw_error(E_UNEXPECTED_VALUE_TYPE, getValueTypeName(symbol->value_type), symbol->name);
    }
    return _getSymbolValueString(symbol);
}

bool getListElementBoolByTypeCasting(char *name, long long i) {
    Symbol* symbol = getListElement(getSymbol(name), i);
    return symbolValueByTypeCastingToBool(symbol);
}

long long getListElementIntByTypeCasting(char *name, long long i) {
    Symbol* symbol = getListElement(getSymbol(name), i);
    return symbolValueByTypeCastingToInt(symbol);
}

long double getListElementFloatByTypeCasting(char *name, long long i) {
    Symbol* symbol = getListElement(getSymbol(name), i);
    return symbolValueByTypeCastingToFloat(symbol);
}

char* getListElementStringByTypeCasting(char *name, long long i) {
    Symbol* symbol = getListElement(getSymbol(name), i);
    return symbolValueByTypeCastingToString(symbol);
}

void copyListElement(char *name, long long i) {
    Symbol* symbol = getListElement(getSymbol(name), i);
    createCloneFromSymbol(
        NULL,
        symbol->type,
        symbol,
        symbol->secondary_type
    );
}

enum Type getListElementType(char *name, long long i) {
    Symbol* symbol = getListElement(getSymbol(name), i);
    return symbol->type;
}

enum ValueType getListElementValueType(char *name, long long i) {
    Symbol* symbol = getListElement(getSymbol(name), i);
    return symbol->value_type;
}

unsigned long getDictLength(char *name) {
    Symbol* symbol = getSymbol(name);
    return symbol->children_count;
}

char* getDictKeyByIndex(char *name, long long i) {
    Symbol* symbol = getSymbol(name);
    Symbol* child = symbol->children[i];
    char* key;
    key = malloc(1 + strlen(child->key));
    strcpy(key, child->key);
    return key;
}

bool getDictElementBool(char *name, char *key) {
    Symbol* symbol = getDictElement(getSymbol(name), key);
    if (symbol->value_type != V_BOOL) {
        throw_error(E_UNEXPECTED_VALUE_TYPE, getValueTypeName(symbol->value_type), symbol->name);
    }
    return _getSymbolValueBool(symbol);
}

long long getDictElementInt(char *name, char *key) {
    Symbol* symbol = getDictElement(getSymbol(name), key);
    if (symbol->value_type != V_INT && symbol->value_type != V_FLOAT) {
        throw_error(E_UNEXPECTED_VALUE_TYPE, getValueTypeName(symbol->value_type), symbol->name);
    }
    return _getSymbolValueInt(symbol);
}

long double getDictElementFloat(char *name, char *key) {
    Symbol* symbol = getDictElement(getSymbol(name), key);
    if (symbol->value_type != V_INT && symbol->value_type != V_FLOAT) {
        throw_error(E_UNEXPECTED_VALUE_TYPE, getValueTypeName(symbol->value_type), symbol->name);
    }
    return _getSymbolValueFloat(symbol);
}

char* getDictElementString(char *name, char *key) {
    Symbol* symbol = getDictElement(getSymbol(name), key);
    if (symbol->value_type != V_STRING) {
        throw_error(E_UNEXPECTED_VALUE_TYPE, getValueTypeName(symbol->value_type), symbol->name);
    }
    return _getSymbolValueString(symbol);
}

bool getDictElementBoolByTypeCasting(char *name, char *key) {
    Symbol* symbol = getDictElement(getSymbol(name), key);
    return symbolValueByTypeCastingToBool(symbol);
}

long long getDictElementIntByTypeCasting(char *name, char *key) {
    Symbol* symbol = getDictElement(getSymbol(name), key);
    return symbolValueByTypeCastingToInt(symbol);
}

long double getDictElementFloatByTypeCasting(char *name, char *key) {
    Symbol* symbol = getDictElement(getSymbol(name), key);
    return symbolValueByTypeCastingToFloat(symbol);
}

char* getDictElementStringByTypeCasting(char *name, char *key) {
    Symbol* symbol = getDictElement(getSymbol(name), key);
    return symbolValueByTypeCastingToString(symbol);
}

void copyDictElement(char *name, char *key) {
    Symbol* symbol = getDictElement(getSymbol(name), key);
    createCloneFromSymbol(
        key,
        symbol->type,
        symbol,
        symbol->secondary_type
    );
}

enum Type getDictElementType(char *name, char *key) {
    Symbol* symbol = getDictElement(getSymbol(name), key);
    return symbol->type;
}

enum ValueType getDictElementValueType(char *name, char *key) {
    Symbol* symbol = getDictElement(getSymbol(name), key);
    return symbol->value_type;
}

char* dumpVariableToString(char *name, bool pretty, bool escaped, bool double_quotes) {
    Symbol* symbol = getSymbol(name);
    bool is_complex = false;
    if (symbol->type == K_LIST || symbol->type == K_DICT)
        is_complex = true;
    char *encoded = NULL;
    encoded = encodeSymbolValueToString(symbol, is_complex, pretty, escaped, 0, encoded, double_quotes);
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
    addSymbolBool(name, b);
}

void createVariableInt(char *name, long long i) {
    addSymbolInt(name, i);
}

void createVariableFloat(char *name, long double f) {
    addSymbolFloat(name, f);
}

void createVariableString(char *name, char *s) {
    addSymbolString(name, s);
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
