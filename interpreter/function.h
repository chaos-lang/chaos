/*
 * Description: Function module of the Chaos Programming Language's source
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

#ifndef KAOS_FUNCTION_H
#define KAOS_FUNCTION_H

#include <stdio.h>
#include <string.h>
#include <setjmp.h>

#if !defined(_WIN32) && !defined(_WIN64) && !defined(__CYGWIN__)
#include <sys/resource.h>
#include <limits.h>
#endif

typedef struct _Function _Function;
typedef struct FunctionCall FunctionCall;

#include "symbol.h"
#include "errors.h"
#if !defined(_WIN32) && !defined(_WIN64) && !defined(__CYGWIN__)
#include "../utilities/shell.h"
#endif
#include "../utilities/language.h"
#include "../utilities/helpers.h"
#include "module.h"

extern enum Phase phase;
enum BlockType { B_EXPRESSION, B_FUNCTION };

typedef struct _Function {
    char *name;
    struct Symbol** parameters;
    unsigned short parameter_count;
    unsigned short optional_parameter_count;
    enum Type type;
    enum Type secondary_type;
    enum ValueType value_type;
    struct Symbol* symbol;
    struct _Function* previous;
    struct _Function* next;
    string_array decision_expressions;
    string_array decision_functions;
    char *decision_default;
    char *context;
    char *module_context;
    char *module;
    long long optional_parameters_addr;
    long long body_addr;
    long long addr;
    _Function* ref;
    bool is_dynamic;
    bool is_compiled;
    int *call_patches;
    int call_patches_size;
} _Function;

_Function* function_cursor;
_Function* start_function;
_Function* end_function;

_Function* function_mode;

_Function* function_parameters_mode;

typedef struct FunctionCall {
    struct _Function* function;
    struct FunctionCall* parent_scope;
    struct ASTNode* trigger;
    int lineno;
    Symbol* start_symbol;
    Symbol* end_symbol;
#ifndef CHAOS_COMPILER
    bool dont_pop_module_stack;
#endif
} FunctionCall;

typedef struct function_call_array {
    FunctionCall** arr;
    unsigned capacity, size;
} function_call_array;

function_call_array function_call_stack;

unsigned long current_file_index;

FunctionCall* function_call_start;
FunctionCall* scopeless;
FunctionCall* scope_override;
FunctionCall* dummy_scope;

_Function* decision_mode;
_Function* decision_expression_mode;
_Function* decision_function_mode;
Symbol* decision_symbol_chain;
char *decision_buffer;

string_array function_names_buffer;

jmp_buf LoopBreakDecision;
jmp_buf LoopContinueDecision;

#ifdef CHAOS_COMPILER
void startFunction(char *name, enum Type type, enum Type secondary_type, char* context, char* module_context, char* module, bool is_dynamic);
#else
void startFunction(char *name, enum Type type, enum Type secondary_type);
#endif

_Function* declareFunction(char *name, char *module, char *module_path, char *context, enum Type type, enum Type secondary_type);
void startFunctionScope(_Function* function);
_Function* startFunctionNew(char *name);
void addFunctionParameterNew(_Function* function, Symbol* parameter);

void endFunction();
void freeFunctionParametersMode();
void resetFunctionParametersMode();
_Function* getFunction(char *name, char *module);
_Function* getFunctionByModuleContext(char *name, char *module_context);
_Function* checkDuplicateFunction(char *name, char *module_path);
void removeFunctionIfDefined(char *name);
void printFunctionTable();

void startFunctionParameters();
void addFunctionParameter(char *secondary_name, enum Type type, enum Type secondary_type);
void addFunctionOptionalParameterBool(char *secondary_name, bool b);
void addFunctionOptionalParameterInt(char *secondary_name, long long i);
void addFunctionOptionalParameterFloat(char *secondary_name, double f);
void addFunctionOptionalParameterString(char *secondary_name, char *s);
void addFunctionOptionalParameterComplex(char *secondary_name, enum Type type);
void addSymbolToFunctionParameters(Symbol* symbol, bool is_optional);
void returnSymbol(char *name);
void initMainFunction();
void initScopeless();

void removeFunction(_Function* function);
void freeFunction(_Function* function);
void freeAllFunctions();
bool block(enum BlockType type);
void addBooleanDecision();
void addDefaultDecision();
void addFunctionNameToFunctionNamesBuffer(char *name);
void freeFunctionNamesBuffer();
bool isInFunctionNamesBuffer(char *name);
bool isFunctionType(char *name, char *module, enum Type type);
void setScopeless(Symbol* symbol);
void pushExecutedFunctionStack(FunctionCall* function_call);
void popExecutedFunctionStack();
void decisionBreakLoop();
void decisionContinueLoop();
void updateDecisionSymbolChainScope();

#endif
