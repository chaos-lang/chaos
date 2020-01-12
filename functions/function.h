#ifndef FUNCTION_H
#define FUNCTION_H

#include <stdio.h>
#include <string.h>

typedef struct {
    char *name;
    struct Function* previous;
    struct Function* next;
    char body[1000];
} Function;

Function* function_cursor;
Function* start_function;
Function* end_function;

Function* function_mode;

void startFunction();
void endFunction();
Function* getFunction(char *name);
void callFunction(char *name);

#endif
