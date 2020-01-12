#ifndef FUNCTION_H
#define FUNCTION_H

#include <stdio.h>
#include <string.h>
#include "../symbol.h"

typedef struct {
    char *name;
    struct Symbol** parameters;
    int parameter_count;
    struct Function* previous;
    struct Function* next;
    char body[1000];
} Function;

Function* function_cursor;
Function* start_function;
Function* end_function;

Function* function_mode;

Function* function_parameters_mode;

void startFunction();
void endFunction();
void freeFunctionMode();
Function* getFunction(char *name);
void callFunction(char *name);
void startFunctionParameters();
void addFunctionParameter(char *secondary_name, enum Type type);
void addSymbolToFunctionParameters(Symbol* symbol);
void addFunctionCallParameterBool(bool b);
void addFunctionCallParameterInt(int i);
void addFunctionCallParameterFloat(float f);
void addFunctionCallParameterString(char *s);

#endif
