#ifndef LOOP_H
#define LOOP_H

#include <stdio.h>
#include <string.h>
#include <stdbool.h>

enum LoopType { TIMESDO, FOREACH, FOREACH_DICT };

typedef struct {
    char *name;
    char *key;
    char *value;
} LoopElement;

typedef struct {
    enum LoopType type;
    int iter;
    int nested_counter;
    char *array;
    LoopElement element;
    char body[1000];
} Loop;

Loop* loop_mode;
bool loop_execution_mode;

void endLoop();

#include "times_do.h"
#include "foreach.h"

#endif
