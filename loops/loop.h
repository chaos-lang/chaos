#ifndef LOOP_H
#define LOOP_H

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <setjmp.h>

enum LoopType { TIMESDO, FOREACH, FOREACH_DICT };

typedef struct {
    char *name;
    char *key;
    char *value;
} LoopElement;

typedef struct {
    enum LoopType type;
    unsigned long long iter;
    bool is_infinite;
    unsigned nested_counter;
    char *array;
    LoopElement element;
    char *body;
} Loop;

Loop* loop_mode;
bool loop_execution_mode;

jmp_buf InteractiveShellLoopErrorAbsorber;
bool interactive_shell_loop_error_absorbed;

void endLoop();

#include "times_do.h"
#include "foreach.h"

#endif
