/*
 * Description: Preemptive Function module of the Chaos Programming Language's
 * source
 *
 * Copyright (c) 2019-2020 Chaos Language Development Authority
 * <info@chaos-lang.org>
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

#ifndef KAOS_PREEMPTIVE_FUNCTION_H
#define KAOS_PREEMPTIVE_FUNCTION_H

#include "../interpreter/symbol.h"
#include "preemptive_errors.h"

_Function* preemptive_function_cursor;

void preemptive_callFunction(_Function* _function, char* name, char* module);
_Function* preemptive_getFunction(_Function* _function, char* name,
                                  char* module);
void preemptive_addSymbolToFunctionParameters(Symbol* symbol, bool is_optional);
void preemptive_addFunctionCallParameterBool();
void preemptive_addFunctionCallParameterInt();
void preemptive_addFunctionCallParameterFloat();
void preemptive_addFunctionCallParameterString();
void preemptive_addFunctionCallParameterSymbol(char* name, _Function* function);
void preemptive_addFunctionCallParameterList(Symbol* symbol);
void preemptive_returnSymbol(char* name, _Function* function);
void preemptive_resetFunctionParametersMode();

#endif
