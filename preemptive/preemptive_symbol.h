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

#ifndef KAOS_PREEMPTIVE_SYMBOL_H
#define KAOS_PREEMPTIVE_SYMBOL_H

#include "../interpreter/symbol.h"
#include "preemptive_errors.h"

Symbol* preemptive_start_symbol;
Symbol* preemptive_end_symbol;

unsigned long long preemptive_nested_complex_counter;

Symbol* preemptive_addSymbol(char *name, enum Type type, enum ValueType value_type);
Symbol* preemptive_findSymbol(char *name, _Function* function);
Symbol* preemptive_getSymbol(char *name, _Function* function);
void preemptive_freeAllSymbols();
void preemptive_removeSymbol(Symbol* symbol);

#endif
