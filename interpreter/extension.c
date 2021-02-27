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

void callFunctionFromDynamicLibrary(_Function* function) {
    char* function_name = "";
    function_name = strcat_ext(function_name, __KAOS_EXTENSION_FUNCTION_PREFIX__);
    function_name = strcat_ext(function_name, function->name);
    dynamic_library dylib = getFunctionFromDynamicLibrary(function->module_context, function_name);
    dylib.func();
    free(function_name);
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
    function_call_stack.arr[function_call_stack.size - 1]->function->symbol = createCloneFromSymbol(
        NULL,
        symbol->type,
        symbol,
        symbol->secondary_type
    );

    decision_symbol_chain = createCloneFromSymbol(
        NULL,
        symbol->type,
        symbol,
        symbol->secondary_type
    );
    scope_override = NULL;
    removeSymbol(symbol);
}
