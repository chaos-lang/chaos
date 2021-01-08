/*
 * Description: Preemptive Symbol module of the Chaos Programming Language's source
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

#include "preemptive_symbol.h"

Symbol* preemptive_addSymbol(char *name, enum Type type, enum ValueType value_type) {
    symbol_cursor = preemptive_start_symbol;

    Symbol* symbol;
    symbol = (struct Symbol*)calloc(1, sizeof(Symbol));

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

Symbol* preemptive_findSymbol(char *name) {
    symbol_cursor = start_symbol;
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
    return NULL;
}

Symbol* preemptive_getSymbol(char *name) {
    Symbol* symbol = preemptive_findSymbol(name);
    if (symbol != NULL)
        return symbol;
    throw_preemptive_error(E_UNDEFINED_VARIABLE, name);
    return NULL;
}
