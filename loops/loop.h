#ifndef LOOP_H
#define LOOP_H

enum LoopType { TIMESDO, FOREACH };

typedef struct {
    enum LoopType type;
    int iter;
    int nested_counter;
    char body[];
} Loop;

Loop* loop_mode;

#endif
