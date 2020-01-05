#ifndef LOOP_H
#define LOOP_H

#include <stdio.h>
#include <string.h>

enum LoopType { TIMESDO, FOREACH };

typedef struct {
    enum LoopType type;
    int iter;
    int nested_counter;
    char *array;
    char *element;
    char body[];
} Loop;

Loop* loop_mode;

void endLoop();

#include "times_do.h"
#include "foreach.h"

#endif
