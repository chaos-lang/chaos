#ifndef FUNCTION_H
#define FUNCTION_H

#include <stdio.h>
#include <string.h>
#include <setjmp.h>

typedef struct _Function _Function;

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
    int line_no;
    struct ASTNode* node;
    struct ASTNode* decision_node;
    struct Symbol** parameters;
    unsigned short parameter_count;
    unsigned short optional_parameter_count;
    enum Type type;
    enum Type secondary_type;
    struct Symbol* symbol;
    struct _Function* previous;
    struct _Function* next;
    struct _Function* parent_scope;
    string_array decision_expressions;
    string_array decision_functions;
    char *decision_default;
    char *context;
    char *module_context;
    char *module;
} _Function;

_Function* function_cursor;
_Function* start_function;
_Function* end_function;

_Function* function_mode;

_Function* function_parameters_mode;

typedef struct function_array {
    _Function** arr;
    unsigned capacity, size;
} function_array;

function_array function_call_stack;

_Function* main_function;
_Function* scopeless;

_Function* scope_override;

_Function* decision_mode;
_Function* decision_expression_mode;
_Function* decision_function_mode;
bool decision_execution_mode;
Symbol* decision_symbol_chain;
char *decision_buffer;

string_array function_names_buffer;

unsigned short recursion_depth;

int reset_line_no_to;

jmp_buf InteractiveShellFunctionErrorAbsorber;
bool interactive_shell_function_error_absorbed;

jmp_buf LoopBreakDecision;

void startFunction(char *name, enum Type type, enum Type secondary_type);
void endFunction();
void freeFunctionParametersMode();
void resetFunctionParametersMode();
_Function* getFunction(char *name, char *module);
void removeFunctionIfDefined(char *name);
void printFunctionTable();
void callFunction(char *name, char *module);
void startFunctionParameters();
void addFunctionParameter(char *secondary_name, enum Type type, enum Type secondary_type);
void addFunctionOptionalParameterBool(char *secondary_name, bool b);
void addFunctionOptionalParameterInt(char *secondary_name, long long i);
void addFunctionOptionalParameterFloat(char *secondary_name, long double f);
void addFunctionOptionalParameterString(char *secondary_name, char *s);
void addFunctionOptionalParameterComplex(char *secondary_name, enum Type type);
void addSymbolToFunctionParameters(Symbol* symbol, bool is_optional);
void addFunctionCallParameterBool(bool b);
void addFunctionCallParameterInt(long long i);
void addFunctionCallParameterFloat(long double f);
void addFunctionCallParameterString(char *s);
void addFunctionCallParameterSymbol(char *name);
void addFunctionCallParameterList(enum Type type);
void returnSymbol(char *name);
void printFunctionReturn(char *name, char *module, char *end, bool pretty, bool escaped);
void createCloneFromFunctionReturn(char *clone_name, enum Type type, char *name, char *module, enum Type extra_type);
void updateSymbolByClonningFunctionReturn(char *clone_name, char *name, char*module);
void updateComplexSymbolByClonningFunctionReturn(char *name, char*module);
void initMainFunction();
void initScopeless();
void removeFunction(_Function* function);
void freeFunction(_Function* function);
void freeAllFunctions();
bool block(enum BlockType type);
void addBooleanDecision();
void addDefaultDecision();
void executeDecision(_Function* function);
void addFunctionNameToFunctionNamesBuffer(char *name);
void freeFunctionNamesBuffer();
bool isInFunctionNamesBuffer(char *name);
bool isFunctionType(char *name, char *module, enum Type type);
void setScopeless(Symbol* symbol);
void pushExecutedFunctionStack(_Function* executed_function);
void popExecutedFunctionStack();
void freeFunctionReturn(char *name, char *module);
void decisionBreakLoop();

#endif
