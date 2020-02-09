#ifndef FUNCTION_H
#define FUNCTION_H

#include <stdio.h>
#include <string.h>

typedef struct _Function _Function;

#include "../symbol.h"
#include "../errors.h"
#include "../utilities/shell.h"
#include "../utilities/phase.h"

extern enum Phase phase;

typedef struct _Function {
    char *name;
    struct Symbol** parameters;
    int parameter_count;
    enum Type type;
    struct Symbol* symbol;
    struct _Function* previous;
    struct _Function* next;
    struct _Function* parent_scope;
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

int recursion_depth;

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

#endif
