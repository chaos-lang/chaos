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
    preemptive_getFunction(name, module);
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
