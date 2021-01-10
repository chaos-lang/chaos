/*
 * Description: Preemptive Function module of the Chaos Programming Language's source
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

#include "preemptive_function.h"

void preemptive_callFunction(char *name, char *module) {
    _Function* function = preemptive_getFunction(name, module);

    if (function_parameters_mode != NULL &&
        function_parameters_mode->parameter_count < (function->parameter_count - function->optional_parameter_count)) {
            throw_preemptive_error(E_INCORRECT_FUNCTION_ARGUMENT_COUNT, name);
    }

    if (function->parameter_count > 0 && function_parameters_mode == NULL) {
        throw_preemptive_error(E_INCORRECT_FUNCTION_ARGUMENT_COUNT, name);
    }

    if (function_parameters_mode != NULL && function_parameters_mode->parameter_count > function->parameter_count) {
        freeFunctionParametersMode();
        throw_preemptive_error(E_INCORRECT_FUNCTION_ARGUMENT_COUNT, name);
    }

    for (unsigned short i = 0; i < function->parameter_count; i++) {
        Symbol* parameter = function->parameters[i];

        if ((i + 1) <= function_parameters_mode->parameter_count) {
            Symbol* parameter_call = function_parameters_mode->parameters[i];

            if (parameter->type != K_ANY && parameter->type != parameter_call->type) {
                freeFunctionParametersMode();
                throw_preemptive_error(E_ILLEGAL_VARIABLE_TYPE_FOR_FUNCTION_PARAMETER, parameter->secondary_name, function->name);
            }

            if ((parameter->type == K_LIST || parameter->type == K_DICT) && parameter->secondary_type != K_ANY) {
                for (unsigned long i = 0; i < parameter_call->children_count; i++) {
                    Symbol* child = parameter_call->children[i];
                    if (child->type != parameter->secondary_type) {
                        freeFunctionParametersMode();
                        throw_preemptive_error(E_ILLEGAL_VARIABLE_TYPE_FOR_FUNCTION_PARAMETER, parameter->secondary_name, function->name);
                    }
                }
            }
        }
    }

    if (function->decision_node != NULL) {
        check_break_continue(function->decision_node, function);
    }

    freeFunctionParametersMode();
}

_Function* preemptive_getFunction(char *name, char *module) {
    preemptive_function_cursor = start_function;
    while (preemptive_function_cursor != NULL) {
        if (module == NULL && strcmp(preemptive_function_cursor->module, "") != 0) {
            preemptive_function_cursor = preemptive_function_cursor->next;
            continue;
        }
        bool criteria = preemptive_function_cursor->name != NULL && strcmp(preemptive_function_cursor->name, name) == 0;
        if (module != NULL) criteria = criteria && strcmp(preemptive_function_cursor->module, module) == 0;
        if (criteria) {
            _Function* function = preemptive_function_cursor;
            return function;
        }
        preemptive_function_cursor = preemptive_function_cursor->next;
    }
    throw_preemptive_error(E_UNDEFINED_FUNCTION, name);
    return NULL;
}

void preemptive_addSymbolToFunctionParameters(Symbol* symbol, bool is_optional) {
    if (phase == PREPARSE) {
        symbol->role = PARAM;
    } else if (phase == PROGRAM) {
        symbol->role = CALL_PARAM;
    }
    setScopeless(symbol);

    if (function_parameters_mode == NULL) {
        startFunctionParameters();
    }

    function_parameters_mode->parameters = realloc(
        function_parameters_mode->parameters,
        sizeof(Symbol) * ++function_parameters_mode->parameter_count
    );

    if (is_optional)
        ++function_parameters_mode->optional_parameter_count;

    if (function_parameters_mode->parameters == NULL) {
        throw_preemptive_error(E_MEMORY_ALLOCATION_FOR_FUNCTION_FAILED, NULL);
    }

    for (unsigned short i = function_parameters_mode->parameter_count - 1; i > 0; i--) {
        function_parameters_mode->parameters[i] = function_parameters_mode->parameters[i - 1];
    }

    function_parameters_mode->parameters[0] = symbol;
}

void preemptive_addFunctionCallParameterBool() {
    Symbol* symbol = preemptive_addSymbol(NULL, K_BOOL, V_BOOL);
    preemptive_addSymbolToFunctionParameters(symbol, false);
}

void preemptive_addFunctionCallParameterInt() {
    Symbol* symbol = preemptive_addSymbol(NULL, K_NUMBER, V_INT);
    preemptive_addSymbolToFunctionParameters(symbol, false);
}

void preemptive_addFunctionCallParameterFloat() {
    Symbol* symbol = preemptive_addSymbol(NULL, K_NUMBER, V_FLOAT);
    preemptive_addSymbolToFunctionParameters(symbol, false);
}

void preemptive_addFunctionCallParameterString() {
    Symbol* symbol = preemptive_addSymbol(NULL, K_STRING, V_STRING);
    preemptive_addSymbolToFunctionParameters(symbol, false);
}

void preemptive_addFunctionCallParameterSymbol(char *name, _Function* function) {
    preemptive_addSymbolToFunctionParameters(preemptive_getSymbol(name, function), false);
}

void preemptive_addFunctionCallParameterList(Symbol* symbol) {
    preemptive_addSymbolToFunctionParameters(symbol, false);
}

void preemptive_returnSymbol(char *name, _Function* function) {
    Symbol* symbol = preemptive_getSymbol(name, function);
    if (symbol->type != K_ANY &&
        function->type != K_ANY &&
        symbol->type != function->type
    ) {
        throw_preemptive_error(E_ILLEGAL_VARIABLE_TYPE_FOR_FUNCTION, getTypeName(symbol->type), function->name);
    }
    if (symbol->secondary_type != K_ANY &&
        function->secondary_type != K_ANY &&
        symbol->secondary_type != function->secondary_type
    ) {
        throw_preemptive_error(E_ILLEGAL_VARIABLE_TYPE_FOR_FUNCTION, getTypeName(symbol->secondary_type), function->name);
    }
    function->symbol = symbol;
}

void preemptive_resetFunctionParametersMode() {
    if (function_parameters_mode == NULL) return;

    for (unsigned short i = 0; i < function_parameters_mode->parameter_count; i++) {
        preemptive_removeSymbol(function_parameters_mode->parameters[i]);
    }
    freeFunctionParametersMode();
}
