#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "loop.h"
#include "../utilities/injector.h"

void startForeach(char *list, char *element_name) {
    if (loop_mode != NULL) {
        loop_mode->nested_counter++;
        free(list);
        free(element_name);
        return;
    }

    loop_mode = (Loop*)calloc(1, sizeof(Loop));
    loop_mode->body = "";
    loop_mode->type = FOREACH;
    loop_mode->nested_counter = 0;

    loop_mode->list = malloc(1 + strlen(list));
    loop_mode->element.name = malloc(1 + strlen(element_name));
    strcpy(loop_mode->list, list);
    strcpy(loop_mode->element.name, element_name);
    free(list);
    free(element_name);

    recordToken(strdup("\n"), 1);
}

void startForeachDict(char *list, char *element_key, char *element_value) {
    if (loop_mode != NULL) {
        loop_mode->nested_counter++;
        free(list);
        free(element_key);
        free(element_value);
        return;
    }

    loop_mode = (Loop*)calloc(1, sizeof(Loop));
    loop_mode->body = "";
    loop_mode->type = FOREACH_DICT;
    loop_mode->nested_counter = 0;

    loop_mode->list = malloc(1 + strlen(list));
    loop_mode->element.key = malloc(1 + strlen(element_key));
    loop_mode->element.value = malloc(1 + strlen(element_value));
    strcpy(loop_mode->list, list);
    strcpy(loop_mode->element.key, element_key);
    strcpy(loop_mode->element.value, element_value);
    free(list);
    free(element_key);
    free(element_value);

    recordToken(strdup("\n"), 1);
}
