#include <stdio.h>
#include <string.h>

#include "loop.h"

void startTimesDo(int iter) {
    if (loop_mode != NULL) {
        loop_mode->nested_counter++;
        return;
    }

    loop_mode = (struct Loop*)malloc(sizeof(Loop));
    loop_mode->type = TIMESDO;
    loop_mode->iter = iter;
    loop_mode->nested_counter = 0;
}
