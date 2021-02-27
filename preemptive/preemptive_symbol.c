/*
 * Description: Preemptive Symbol module of the Chaos Programming Language's source
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

#include "preemptive_symbol.h"

unsigned long long preemptive_nested_complex_counter = 0;

Symbol* preemptive_addSymbol(char *name, enum Type type, enum ValueType value_type) {
    if (preemptive_nested_complex_counter != 0)
        return NULL;

    symbol_cursor = preemptive_start_symbol;

    Symbol* symbol;
    symbol = (struct Symbol*)calloc(1, sizeof(Symbol));

    if (name != NULL) {
        symbol->name = malloc(1 + strlen(name));
        strcpy(symbol->name, name);
    }
    symbol->type = type;
    symbol->value_type = value_type;

    if (preemptive_start_symbol == NULL) {
        preemptive_start_symbol = symbol;
        preemptive_end_symbol = symbol;
    } else {
        preemptive_end_symbol->next = symbol;
        symbol->previous = preemptive_end_symbol;
        preemptive_end_symbol = symbol;
        preemptive_end_symbol->next = NULL;
    }

    return symbol;
}

Symbol* preemptive_findSymbol(char *name, _Function* function) {
    symbol_cursor = preemptive_start_symbol;
    while (symbol_cursor != NULL) {
        if (
            symbol_cursor->name != NULL &&
            strcmp(symbol_cursor->name, name) == 0
        ) {
            Symbol* symbol = symbol_cursor;
            return symbol;
        }
        symbol_cursor = symbol_cursor->next;
    }

    FunctionCall* scope = getCurrentScope();
    symbol_cursor = scope->start_symbol;
    while (symbol_cursor != NULL) {
        if (
            symbol_cursor->secondary_name != NULL &&
            strcmp(symbol_cursor->secondary_name, name) == 0 &&
            symbol_cursor->param_of == function
        ) {
            Symbol* symbol = symbol_cursor;
            return symbol;
        }
        symbol_cursor = symbol_cursor->next;
    }

    return NULL;
}

Symbol* preemptive_getSymbol(char *name, _Function* function) {
    Symbol* symbol = preemptive_findSymbol(name, function);
    if (symbol != NULL)
        return symbol;
    add_preemptive_error(E_UNDEFINED_VARIABLE, function, name);
    return NULL;
}

void preemptive_freeAllSymbols() {
    if (preemptive_start_symbol != NULL)
        preemptive_resetFunctionParametersMode();
    symbol_cursor = preemptive_start_symbol;
    while (symbol_cursor != NULL) {
        Symbol* symbol = symbol_cursor;
        symbol_cursor = symbol_cursor->next;
        preemptive_removeSymbol(symbol);
    }
    preemptive_start_symbol = NULL;
    preemptive_end_symbol = NULL;
}

void preemptive_removeSymbol(Symbol* symbol) {
    removeChildrenOfComplex(symbol);

    Symbol* previous_symbol = symbol->previous;
    Symbol* next_symbol = symbol->next;

    if (previous_symbol == NULL && next_symbol == NULL) {
        preemptive_start_symbol = NULL;
        preemptive_end_symbol = NULL;
        freeSymbol(symbol);
        return;
    } else if (previous_symbol == NULL) {
        preemptive_start_symbol = next_symbol;
        preemptive_start_symbol->previous = NULL;
    } else if (next_symbol == NULL) {
        preemptive_end_symbol = previous_symbol;
        preemptive_end_symbol->next = NULL;
    } else {
        previous_symbol->next = next_symbol;
        next_symbol->previous = previous_symbol;
    }

    freeSymbol(symbol);
}
