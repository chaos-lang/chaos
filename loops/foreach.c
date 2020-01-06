#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "loop.h"

void startForeach(char *array, char *element) {
    if (loop_mode != NULL) {
        loop_mode->nested_counter++;
        return;
    }

    loop_mode = (struct Loop*)malloc(sizeof(Loop));
    loop_mode->type = FOREACH;
    loop_mode->nested_counter = 0;

    loop_mode->array = array;
    loop_mode->element = element;
}
