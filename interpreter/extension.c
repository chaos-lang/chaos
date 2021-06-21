/*
 * Description: Extension module of the Chaos Programming Language's source
 *
 * Copyright (c) 2019-2021 Chaos Language Development Authority <info@chaos-lang.org>
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

#include "extension.h"

void initKaosApi() {
    kaos.defineFunction = defineFunction;
    kaos.getVariableBool = getVariableBool;
    kaos.getVariableInt = getVariableInt;
    kaos.getVariableFloat = getVariableFloat;
    kaos.getVariableString = getVariableString;
    kaos.getVariableBoolByTypeCasting = getVariableBoolByTypeCasting;
    kaos.getVariableIntByTypeCasting = getVariableIntByTypeCasting;
    kaos.getVariableFloatByTypeCasting = getVariableFloatByTypeCasting;
    kaos.getVariableStringByTypeCasting = getVariableStringByTypeCasting;
    kaos.getListLength = getListLength;
    kaos.getListElementBool = getListElementBool;
    kaos.getListElementInt = getListElementInt;
    kaos.getListElementFloat = getListElementFloat;
    kaos.getListElementString = getListElementString;
    kaos.getListElementBoolByTypeCasting = getListElementBoolByTypeCasting;
    kaos.getListElementIntByTypeCasting = getListElementIntByTypeCasting;
    kaos.getListElementFloatByTypeCasting = getListElementFloatByTypeCasting;
    kaos.getListElementStringByTypeCasting = getListElementStringByTypeCasting;
    kaos.copyListElement = copyListElement;
    kaos.getListElementType = getListElementType;
    kaos.getListElementValueType = getListElementValueType;
    kaos.getDictLength = getDictLength;
    kaos.getDictKeyByIndex = getDictKeyByIndex;
    kaos.getDictElementBool = getDictElementBool;
    kaos.getDictElementInt = getDictElementInt;
    kaos.getDictElementFloat = getDictElementFloat;
    kaos.getDictElementString = getDictElementString;
    kaos.getDictElementBoolByTypeCasting = getDictElementBoolByTypeCasting;
    kaos.getDictElementIntByTypeCasting = getDictElementIntByTypeCasting;
    kaos.getDictElementFloatByTypeCasting = getDictElementFloatByTypeCasting;
    kaos.getDictElementStringByTypeCasting = getDictElementStringByTypeCasting;
    kaos.copyDictElement = copyDictElement;
    kaos.getDictElementType = getDictElementType;
    kaos.getDictElementValueType = getDictElementValueType;
    kaos.dumpVariableToString = dumpVariableToString;
    kaos.returnVariableBool = returnVariableBool;
    kaos.returnVariableInt = returnVariableInt;
    kaos.returnVariableFloat = returnVariableFloat;
    kaos.returnVariableString = returnVariableString;
    kaos.createVariableBool = createVariableBool;
    kaos.createVariableInt = createVariableInt;
    kaos.createVariableFloat = createVariableFloat;
    kaos.createVariableString = createVariableString;
    kaos.startBuildingList = startBuildingList;
    kaos.returnList = returnList;
    kaos.startBuildingDict = startBuildingDict;
    kaos.returnDict = returnDict;
    kaos.returnComplex = returnComplex;
    kaos.finishList = finishList;
    kaos.finishDict = finishDict;
    kaos.finishComplex = finishComplex;
    kaos.getListType = getListType;
    kaos.getDictType = getDictType;
    kaos.getValueType = getValueType;
    kaos.getRole = getRole;
    kaos.raiseError = raiseError;
    kaos.parseJson = parseJson;
}

void callRegisterInDynamicLibrary(char* dynamic_library_path) {
    dynamic_library dylib = getFunctionFromDynamicLibrary(dynamic_library_path, __KAOS_EXTENSION_REGISTER_FUNCTION__);
    dylib.func(kaos);
#ifndef CHAOS_COMPILER
    if (is_interactive)
        phase = PROGRAM;
#endif
}

void callFunctionFromDynamicLibrary(_Function* function, cpu* c) {
    char* function_name = "";
    function_name = strcat_ext(function_name, __KAOS_EXTENSION_FUNCTION_PREFIX__);
    function_name = strcat_ext(function_name, function->name);
    dynamic_library dylib = getFunctionFromDynamicLibrary(function->module_context, function_name);
    startFunctionScope(function);
    populateCallParametersDynamicLibrary(function, c);
    dylib.func();
    handleFunctionReturnDynamicLibrary(function, c);
    removeSymbolsByScope(getCurrentScope());
    endFunction();
    free(function_name);
}

void populateCallParametersDynamicLibrary(_Function* function, cpu* c) {
    i64 arg_count = c->r[R5A];
    for (unsigned short i = 0; i < function->parameter_count; i++) {
        Symbol* parameter = function->parameters[i];

        arg_count--;
        if (arg_count < 0) {
            switch (parameter->type) {
            case K_BOOL:
                addSymbolBool(parameter->secondary_name, parameter->value.b);
                break;
            case K_NUMBER:
                addSymbolFloat(parameter->secondary_name, parameter->value.f);
                break;
            case K_STRING:
                addSymbolString(parameter->secondary_name, parameter->value.s);
                break;
            default:
                break;
            }
            continue;
        }

        switch (c->stack[c->sp++]) {
        case V_BOOL: {
            i64 i = c->stack[c->sp++];
            bool b;
            if (i == 0)
                b = false;
            else
                b = true;
            addSymbolBool(parameter->secondary_name, b);
            break;
        }
        case V_INT: {
            i64 i = c->stack[c->sp++];
            addSymbolInt(parameter->secondary_name, i);
            break;
        }
        case V_FLOAT: {
            f64 f;
            i64 i = c->stack[c->sp++];
            memcpy(&f, &i, sizeof f);;
            addSymbolFloat(parameter->secondary_name, f);
            break;
        }
        case V_STRING: {
            i64 len = c->stack[c->sp++];
            char* value = build_string(c, len);
            addSymbolString(parameter->secondary_name, value);
            break;
        }
        case V_LIST: {
            i64 len = c->stack[c->sp++];
            addSymbolList(parameter->secondary_name);
            for (unsigned long j = 0; j < len; j++) {
                i64 r0 = c->stack[c->sp++];
                switch (r0) {
                case V_BOOL: {
                    i64 i = c->stack[c->sp++];
                    bool b;
                    if (i == 0)
                        b = false;
                    else
                        b = true;
                    addSymbolBool(NULL, b);
                    break;
                }
                case V_INT: {
                    i64 i = c->stack[c->sp++];
                    addSymbolInt(NULL, i);
                    break;
                }
                case V_FLOAT: {
                    f64 f;
                    i64 i = c->stack[c->sp++];
                    memcpy(&f, &i, sizeof f);;
                    addSymbolFloat(NULL, f);
                    break;
                }
                case V_STRING: {
                    i64 len = c->stack[c->sp++];
                    char* value = build_string(c, len);
                    addSymbolString(NULL, value);
                    break;
                }
                default:
                    break;
                }
            }
            reverseComplexMode();
            finishComplexMode(NULL, K_ANY);
            break;
        }
        case V_DICT: {
            i64 len = c->stack[c->sp++];
            addSymbolDict(parameter->secondary_name);
            for (unsigned long j = 0; j < len; j++) {
                c->sp++;
                i64 key_len = c->stack[c->sp++];
                char* key = build_string(c, key_len);

                i64 r0 = c->stack[c->sp++];
                switch (r0) {
                case V_BOOL: {
                    i64 i = c->stack[c->sp++];
                    bool b;
                    if (i == 0)
                        b = false;
                    else
                        b = true;
                    addSymbolBool(key, b);
                    break;
                }
                case V_INT: {
                    i64 i = c->stack[c->sp++];
                    addSymbolInt(key, i);
                    break;
                }
                case V_FLOAT: {
                    f64 f;
                    i64 i = c->stack[c->sp++];
                    memcpy(&f, &i, sizeof f);;
                    addSymbolFloat(key, f);
                    break;
                }
                case V_STRING: {
                    i64 str_len = c->stack[c->sp++];
                    char* value = build_string(c, str_len);
                    addSymbolString(key, value);
                    break;
                }
                default:
                    break;
                }
            }
            reverseComplexMode();
            finishComplexMode(NULL, K_ANY);
            break;
        }
        default:
            break;
        }
    }
}

void handleFunctionReturnDynamicLibrary(_Function* function, cpu* c) {
    if (function->symbol == NULL) {
        return;
    }

    switch (function->symbol->type) {
    case K_BOOL:
        c->r[R0A] = V_BOOL;
        c->r[R1A] = function->symbol->value.b ? 1 : 0;
        break;
    case K_NUMBER:
        if (function->symbol->value_type == V_INT) {
            c->r[R0A] = V_INT;
            c->r[R1A] = function->symbol->value.i;
        } else {
            c->r[R0A] = V_FLOAT;
            i64 i;
            f64 f = function->symbol->value.f;
            memcpy(&i, &f, sizeof f);
            c->r[R1A] = i;
        }
        break;
    case K_STRING: {
        c->r[R0A] = V_STRING;
        i64 len = strlen(function->symbol->value.s);
        c->r[R1A] = len;
        for (size_t i = 0; i < len; i++) {
            c->stack[--c->sp] = function->symbol->value.s[i];
        }
        break;
    }
    case K_LIST: {
        c->r[R0A] = V_LIST;
        i64 len = function->symbol->children_count;
        c->r[R1A] = len;
        for (unsigned long i = len; i > 0; i--) {
            Symbol* child = function->symbol->children[i - 1];
            switch (child->value_type) {
            case V_BOOL:
                c->stack[--c->sp] = child->value.b ? 1 : 0;
                c->stack[--c->sp] = V_BOOL;
                break;
            case V_INT:
                c->stack[--c->sp] = child->value.i;
                c->stack[--c->sp] = V_INT;
                break;
            case V_FLOAT: {
                i64 i;
                f64 f = child->value.f;
                memcpy(&i, &f, sizeof f);
                c->stack[--c->sp] = i;
                c->stack[--c->sp] = V_FLOAT;
                break;
            }
            case V_STRING: {
                i64 _len = strlen(child->value.s);
                for (size_t i = _len; i > 0; i--) {
                    c->stack[--c->sp] = child->value.s[i - 1] - '0';
                }
                c->stack[--c->sp] = _len;
                c->stack[--c->sp] = V_STRING;
                break;
            }
            default:
                break;
            }
        }
        break;
    }
    case K_DICT: {
        c->r[R0A] = V_DICT;
        i64 len = function->symbol->children_count;
        c->r[R1A] = len;
        for (unsigned long i = len; i > 0; i--) {
            Symbol* child = function->symbol->children[i - 1];
            switch (child->value_type) {
            case V_BOOL:
                c->stack[--c->sp] = child->value.b ? 1 : 0;
                c->stack[--c->sp] = V_BOOL;
                break;
            case V_INT:
                c->stack[--c->sp] = child->value.i;
                c->stack[--c->sp] = V_INT;
                break;
            case V_FLOAT: {
                i64 i;
                f64 f = child->value.f;
                memcpy(&i, &f, sizeof f);
                c->stack[--c->sp] = i;
                c->stack[--c->sp] = V_FLOAT;
                break;
            }
            case V_STRING: {
                i64 _len = strlen(child->value.s);
                for (size_t i = _len; i > 0; i--) {
                    c->stack[--c->sp] = child->value.s[i - 1] - '0';
                }
                c->stack[--c->sp] = _len;
                c->stack[--c->sp] = V_STRING;
                break;
            }
            default:
                break;
            }

            i64 key_len = strlen(child->key);
            for (size_t i = key_len; i > 0; i--) {
                c->stack[--c->sp] = child->key[i - 1] - '0';
            }
            c->stack[--c->sp] = key_len;
            c->stack[--c->sp] = V_STRING;
        }
        break;
    }
    default:
        break;
    }
}

dynamic_library getFunctionFromDynamicLibrary(char* dynamic_library_path, char* function_name) {
    dynamic_library dylib;

    dylib.handle = OPENLIB(dynamic_library_path);

    if (dylib.handle == NULL) {
#if !defined(_WIN32) && !defined(_WIN64) && !defined(__CYGWIN__)
        fprintf(stderr, "Unable to open lib: %s\n", dlerror());
#endif
    }
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
    dylib.func = LIBFUNC(dylib.handle, function_name);
#pragma GCC diagnostic pop

    if (dylib.func == NULL) {
        fprintf(stderr, "Unable to get symbol\n");
    }

    return dylib;
}

void returnVariable(Symbol* symbol) {
    scope_override = function_call_stack.arr[function_call_stack.size - 1]->parent_scope;
    function_call_stack.arr[function_call_stack.size - 1]->function->symbol = symbol;
    scope_override = NULL;
    // removeSymbol(symbol);
}
