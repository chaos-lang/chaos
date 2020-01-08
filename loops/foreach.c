#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "loop.h"

void startForeach(char *array, char *element_name) {
    if (loop_mode != NULL) {
        loop_mode->nested_counter++;
        return;
    }

    loop_mode = (struct Loop*)malloc(sizeof(Loop));
    loop_mode->type = FOREACH;
    loop_mode->nested_counter = 0;

    loop_mode->array = array;
    loop_mode->element.name = element_name;
}

void startForeachDict(char *array, char *element_key, char *element_value) {
    if (loop_mode != NULL) {
        loop_mode->nested_counter++;
        return;
    }

    loop_mode = (struct Loop*)malloc(sizeof(Loop));
    loop_mode->type = FOREACH_DICT;
    loop_mode->nested_counter = 0;

    loop_mode->array = array;
    loop_mode->element.key = element_key;
    loop_mode->element.value = element_value;
}
