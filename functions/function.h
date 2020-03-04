#ifndef FUNCTION_H
#define FUNCTION_H

#include <stdio.h>
#include <string.h>

typedef struct _Function _Function;

#include "../symbol.h"
#include "../errors.h"
#if !defined(_WIN32) && !defined(_WIN64) && !defined(__CYGWIN__)
#include "../utilities/shell.h"
#endif
#include "../utilities/language.h"
#include "../utilities/phase.h"

extern enum Phase phase;
enum BlockType { B_EXPRESSION, B_FUNCTION };

typedef struct _Function {
    char *name;
    struct Symbol** parameters;
    int parameter_count;
    enum Type type;
    struct Symbol* symbol;
    struct _Function* previous;
    struct _Function* next;
    struct _Function* parent_scope;
    char *decision_expressions[1000];
    char *decision_functions[1000];
    char *decision_default;
    int decision_length;
    char *modules[1000];
    char body[1000];
} _Function;

_Function* function_cursor;
_Function* start_function;
_Function* end_function;

_Function* function_mode;

_Function* function_parameters_mode;

_Function* executed_function;

_Function* main_function;
_Function* scopeless;

_Function* scope_override;

_Function* decision_mode;
_Function* decision_expression_mode;
_Function* decision_function_mode;
Symbol* decision_symbol_chain;
char decision_buffer[1000];

char *modules_buffer[1000];
int modules_buffer_length;

int recursion_depth;

extern char *program_file_path;
extern char *program_file_dir;

void startFunction(char *name, enum Type type);
void endFunction();
void freeFunctionMode();
_Function* getFunction(char *name);
void callFunction(char *name);
void startFunctionParameters();
void addFunctionParameter(char *secondary_name, enum Type type);
void addSymbolToFunctionParameters(Symbol* symbol);
void addFunctionCallParameterBool(bool b);
void addFunctionCallParameterInt(int i);
void addFunctionCallParameterFloat(float f);
void addFunctionCallParameterString(char *s);
void addFunctionCallParameterSymbol(char *name);
void returnSymbol(char *name);
void printFunctionReturn(char *name);
void initMainFunction();
void initScopeless();
void freeFunction(_Function* function);
void freeAllFunctions();
bool block(enum BlockType type);
void addBooleanDecision();
void addDefaultDecision();
void executeDecision(_Function* function);
void addModuleToModuleBuffer(char *module);
void handleModuleImport();
void freeModulesBuffer();

#endif
